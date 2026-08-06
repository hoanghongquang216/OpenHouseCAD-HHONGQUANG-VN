#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/document/EntityId.hpp>
#include <openhouse/document/Transform.hpp>
#include <openhouse/geometry/Point2.hpp>
#include <openhouse/geometry/Vector2.hpp>

#include <optional>

namespace openhouse::document {

// A single undoable/redoable operation on a Document. Spiral 5's
// foundation for Undo/Redo.
//
// Design (locked after review, see docs/ARCHITECTURE_DECISION_RECORDS
// and this Spiral's own design-lock discussion):
//   - Undo/Redo use the MEMENTO pattern (a full before/after snapshot
//     of the affected shape), NOT "store parameters, recompute the
//     inverse transform". This was settled after demonstrating, with
//     real code, that repeated Scale-then-inverse-Scale drifts by
//     ~1.4e-14 per cycle for an off-origin pivot -- a CAD document
//     Undo/Redo'd many times would slowly lose precision. A snapshot
//     restores the EXACT prior value, unconditionally, with zero drift
//     regardless of how many Undo/Redo cycles occur.
//   - `Shape` itself (Line2/Circle2/Arc2 variant) IS the snapshot --
//     no separate "vertex list" or snapshot type is introduced; Shape
//     is already a small, cheap-to-copy value type that fully captures
//     an entity's geometry.
class ICommand {
public:
    virtual ~ICommand() = default;

    // Performs the operation. Returns true if it actually changed
    // something (false for e.g. a locked layer, an invalid scale
    // factor, or an entity that no longer exists) -- callers should not
    // push a failed Execute() onto undo history, since there is nothing
    // meaningful to undo.
    [[nodiscard]] virtual bool Execute(Document& doc) = 0;

    // Restores the state from immediately before the most recent
    // Execute(). Only meaningful to call after a successful Execute().
    virtual void Undo(Document& doc) = 0;

    // Re-applies the change undone by the most recent Undo(). Restores
    // the state from immediately after Execute() -- does NOT re-run
    // Execute()'s computation, for the same drift-avoidance reason
    // Undo() doesn't recompute an inverse.
    virtual void Redo(Document& doc) = 0;
};

// Shared base for any command that performs a single-entity geometric
// transform (Translate/Rotate/Scale) via one of the existing
// TranslateEntity/RotateEntity/ScaleEntity functions (Transform.hpp).
//
// Deliberately stores only an EntityId, never a raw Entity* or Shape*
// cached across calls -- Document's entities_ is a vector, and
// FindEntityMutable()'s returned pointer is only valid until the next
// structural change to that vector (e.g. another Document::Add()
// reallocating it). A Command can easily outlive the moment it was
// constructed (sitting in undo history while the user keeps working),
// so any pointer captured at construction time would risk becoming a
// dangling pointer by the time Undo() eventually runs -- a real bug
// caught and fixed during this Spiral's design review, not a
// theoretical concern. Every access below re-resolves the entity via
// FindEntity/FindEntityMutable(id_) at the moment it's actually needed.
class EntityShapeCommandBase : public ICommand {
protected:
    explicit EntityShapeCommandBase(EntityId id) noexcept : id_(id) {}

    // Subclasses implement this to call the ONE matching existing
    // document-layer function (TranslateEntity/RotateEntity/
    // ScaleEntity) -- not to reimplement any of its validation logic
    // (Locked/Visible/factor>0 checks). Duplicating that logic here
    // would risk it silently drifting out of sync with Transform.hpp's
    // own rules.
    [[nodiscard]] virtual bool DoOperation(Document& doc, EntityId id) = 0;

public:
    [[nodiscard]] bool Execute(Document& doc) override {
        const Entity* entity = doc.FindEntity(id_);
        if (entity == nullptr) {
            return false;
        }
        shapeBefore_ = entity->shape; // snapshot immediately before the change
        if (!DoOperation(doc, id_)) {
            return false; // rejected (locked/hidden/invalid factor/etc.) --
                           // nothing changed, nothing to snapshot as "after"
        }
        shapeAfter_ = doc.FindEntity(id_)->shape; // snapshot immediately after
        return true;
    }

    void Undo(Document& doc) override {
        if (Entity* entity = doc.FindEntityMutable(id_)) {
            entity->shape = shapeBefore_;
        }
        // A missing entity here (e.g. deleted by some other operation
        // since this command executed) is silently a no-op -- there is
        // nothing to restore it TO if it no longer exists. Document has
        // no entity-deletion capability yet (see Document.hpp's own
        // TODO(Spiral5) on FindEntity's indexing), so this path isn't
        // reachable through normal use yet; the check exists so this
        // class doesn't need revisiting the day deletion is added.
    }

    void Redo(Document& doc) override {
        if (Entity* entity = doc.FindEntityMutable(id_)) {
            entity->shape = shapeAfter_;
        }
    }

    [[nodiscard]] EntityId Id() const noexcept { return id_; }

private:
    EntityId id_;
    Shape shapeBefore_{};
    Shape shapeAfter_{};
};

class TranslateCommand final : public EntityShapeCommandBase {
public:
    TranslateCommand(EntityId id, geometry::Vector2d delta) noexcept
        : EntityShapeCommandBase(id), delta_(delta) {}

protected:
    [[nodiscard]] bool DoOperation(Document& doc, EntityId id) override {
        return TranslateEntity(doc, id, delta_);
    }

private:
    geometry::Vector2d delta_;
};

class RotateCommand final : public EntityShapeCommandBase {
public:
    RotateCommand(EntityId id, double radians, geometry::Point2d pivot) noexcept
        : EntityShapeCommandBase(id), radians_(radians), pivot_(pivot) {}

protected:
    [[nodiscard]] bool DoOperation(Document& doc, EntityId id) override {
        return RotateEntity(doc, id, radians_, pivot_);
    }

private:
    double radians_;
    geometry::Point2d pivot_;
};

class ScaleCommand final : public EntityShapeCommandBase {
public:
    ScaleCommand(EntityId id, double factor, geometry::Point2d pivot) noexcept
        : EntityShapeCommandBase(id), factor_(factor), pivot_(pivot) {}

protected:
    [[nodiscard]] bool DoOperation(Document& doc, EntityId id) override {
        // factor > 0 is still enforced inside ScaleEntity itself (see
        // Transform.hpp) -- this command does not duplicate that check;
        // it just forwards to the one function that owns the rule.
        return ScaleEntity(doc, id, factor_, pivot_);
    }

private:
    double factor_;
    geometry::Point2d pivot_;
};

// Shared base for any command whose job is to add exactly ONE new
// entity to the Document (Execute creates it, Undo removes it, Redo
// re-creates it at the same id). Added for COPY-001 (Sprint 4) -- see
// docs/design/COPY-001-Design.md Section 4 for the full rationale,
// including why DELETE-001 deliberately does NOT share this base (it
// runs the opposite direction: Execute removes, Undo restores).
//
// Deliberately the mirror image of EntityShapeCommandBase's discipline:
// never caches a pointer across calls (the same vector-reallocation
// hazard applies to a newly-created entity's pointer as to an existing
// one's), and stores only the minimal id/shape/layer snapshot needed to
// reverse its own single Add.
//
// Responsibility boundary (kept deliberately narrow, per Design §4):
// this class knows how to create/remove/restore ONE entity. It knows
// NOTHING about where the new shape/layer comes from (cloning a source
// entity, deserializing a clipboard payload, or anything else) -- that
// is entirely BuildEntity()'s job, decided by the concrete subclass.
// This base must never grow a Clone/Translate/CanTransform call of its
// own; a subclass needing that logic implements it in BuildEntity()
// instead, using whatever free functions already exist for it (e.g.
// Transform.hpp's TranslateEntity, CanTransform) rather than this base
// duplicating or wrapping them.
class EntityCreationCommandBase : public ICommand {
protected:
    // Subclasses compute what the new entity should look like. Returns
    // std::nullopt to reject (nothing created, no undo-relevant state
    // recorded) -- same "reject via nullopt/false" convention already
    // used by EntityShapeCommandBase::DoOperation and the *Entity
    // transform functions in Transform.hpp. The returned Entity's `id`
    // field is ignored -- Document::Add() assigns the real one.
    [[nodiscard]] virtual std::optional<Entity> BuildEntity(Document& doc) = 0;

public:
    // Strong exception/failure safety: if BuildEntity() returns
    // nullopt, this function returns false having made NO change to
    // `doc` and NO change to this command's own stored state (id_
    // stays kInvalidEntityId, shape_/layer_ stay default-constructed).
    // There is no intermediate state where an entity was created but
    // not recorded, or recorded but not created -- Document::Add() is
    // the only mutating call in this function, and it only runs after
    // BuildEntity() has already succeeded.
    [[nodiscard]] bool Execute(Document& doc) override {
        std::optional<Entity> built = BuildEntity(doc);
        if (!built.has_value()) {
            return false;
        }
        id_ = doc.Add(built->shape, built->layer);
        shape_ = built->shape;
        layer_ = built->layer;
        return true;
    }

    // Safe to call even if Execute() never succeeded: id_ is
    // kInvalidEntityId until a successful Execute() sets it, and
    // Document::RemoveEntity(kInvalidEntityId) is a well-defined no-op
    // (kInvalidEntityId can never be a live entity's id) -- so calling
    // Undo() out of sequence cannot corrupt `doc` or crash, it simply
    // does nothing.
    void Undo(Document& doc) override { doc.RemoveEntity(id_); }

    // Symmetrically safe to call before any successful Execute(): with
    // id_ still kInvalidEntityId, Document::Restore() rejects
    // immediately (its own precondition check) and leaves `doc`
    // untouched.
    void Redo(Document& doc) override { doc.Restore(id_, shape_, layer_); }

    [[nodiscard]] EntityId Id() const noexcept { return id_; }

private:
    EntityId id_ = kInvalidEntityId;
    Shape shape_{};
    foundation::string layer_;
};

// Copies a single entity by `delta`, leaving the source entity
// untouched. Per docs/design/COPY-001.md (Domain Research), Copy is
// Translate applied to a duplicate rather than a distinct geometric
// operation -- BuildEntity() below reuses the same geometry::Translate
// dispatch TranslateEntity (Transform.hpp) uses, and reuses
// CanTransform() as the same permission gate every other *Entity
// function already applies to its source/target.
//
// Deliberately contains NO lifecycle logic of its own (create/remove/
// restore, id bookkeeping) -- all of that is EntityCreationCommandBase's
// job. This class's only responsibility is answering "what should the
// new entity look like," per Design §4's stated boundary.
class CopyCommand final : public EntityCreationCommandBase {
public:
    CopyCommand(EntityId sourceId, geometry::Vector2d delta) noexcept
        : sourceId_(sourceId), delta_(delta) {}

protected:
    [[nodiscard]] std::optional<Entity> BuildEntity(Document& doc) override {
        const Entity* src = doc.FindEntity(sourceId_);
        if (src == nullptr || !CanTransform(doc, sourceId_)) {
            return std::nullopt;
        }
        Shape clone = std::visit(
            [this](const auto& s) -> Shape { return geometry::Translate(s, delta_); },
            src->shape);
        return Entity{kInvalidEntityId, foundation::move(clone), src->layer};
    }

private:
    EntityId sourceId_;
    geometry::Vector2d delta_;
};

// Deletes a single entity. Per docs/design/DELETE-001-Design.md Section 3,
// deliberately does NOT subclass EntityCreationCommandBase -- Delete runs
// the opposite direction (Execute removes, Undo restores) from what that
// base's hooks mean, so it implements ICommand directly instead. Reuses
// the exact same Document::RemoveEntity/Restore pair EntityCreationCommandBase
// already uses, just in the mirrored order -- no new Document API needed
// (per DELETE-001-Architecture-Audit.md's GO finding).
//
// `id_` is fixed at construction and never reassigned -- unlike
// EntityCreationCommandBase's id_ (which starts invalid until a
// successful Execute assigns a freshly-created entity's id),
// DeleteCommand's target is always known up front.
class DeleteCommand final : public ICommand {
public:
    explicit DeleteCommand(EntityId id) noexcept : id_(id) {}

    [[nodiscard]] bool Execute(Document& doc) override {
        const Entity* entity = doc.FindEntity(id_);
        if (entity == nullptr || !CanTransform(doc, id_)) {
            return false;
        }
        shape_ = entity->shape; // snapshot before removal, for Undo
        layer_ = entity->layer;
        return doc.RemoveEntity(id_);
    }

    void Undo(Document& doc) override { doc.Restore(id_, shape_, layer_); }

    // Does not re-resolve or re-snapshot -- the entity is guaranteed to
    // exist at id_ (Undo just restored it there) with the same
    // shape_/layer_ already saved from Execute(). Mirrors the "don't
    // recompute, reuse the saved snapshot" Memento discipline used
    // throughout this file.
    void Redo(Document& doc) override { doc.RemoveEntity(id_); }

    [[nodiscard]] EntityId Id() const noexcept { return id_; }

private:
    EntityId id_;
    Shape shape_{};
    foundation::string layer_;
};

}
