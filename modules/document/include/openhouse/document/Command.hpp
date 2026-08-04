#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/document/EntityId.hpp>
#include <openhouse/document/Transform.hpp>
#include <openhouse/geometry/Point2.hpp>
#include <openhouse/geometry/Vector2.hpp>

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
class TransformEntityCommandBase : public ICommand {
protected:
    explicit TransformEntityCommandBase(EntityId id) noexcept : id_(id) {}

    // Subclasses implement this to call the ONE matching existing
    // document-layer function (TranslateEntity/RotateEntity/
    // ScaleEntity) -- not to reimplement any of its validation logic
    // (Locked/Visible/factor>0 checks). Duplicating that logic here
    // would risk it silently drifting out of sync with Transform.hpp's
    // own rules.
    [[nodiscard]] virtual bool DoTransform(Document& doc, EntityId id) = 0;

public:
    [[nodiscard]] bool Execute(Document& doc) override {
        const Entity* entity = doc.FindEntity(id_);
        if (entity == nullptr) {
            return false;
        }
        shapeBefore_ = entity->shape; // snapshot immediately before the change
        if (!DoTransform(doc, id_)) {
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

class TranslateCommand final : public TransformEntityCommandBase {
public:
    TranslateCommand(EntityId id, geometry::Vector2d delta) noexcept
        : TransformEntityCommandBase(id), delta_(delta) {}

protected:
    [[nodiscard]] bool DoTransform(Document& doc, EntityId id) override {
        return TranslateEntity(doc, id, delta_);
    }

private:
    geometry::Vector2d delta_;
};

class RotateCommand final : public TransformEntityCommandBase {
public:
    RotateCommand(EntityId id, double radians, geometry::Point2d pivot) noexcept
        : TransformEntityCommandBase(id), radians_(radians), pivot_(pivot) {}

protected:
    [[nodiscard]] bool DoTransform(Document& doc, EntityId id) override {
        return RotateEntity(doc, id, radians_, pivot_);
    }

private:
    double radians_;
    geometry::Point2d pivot_;
};

class ScaleCommand final : public TransformEntityCommandBase {
public:
    ScaleCommand(EntityId id, double factor, geometry::Point2d pivot) noexcept
        : TransformEntityCommandBase(id), factor_(factor), pivot_(pivot) {}

protected:
    [[nodiscard]] bool DoTransform(Document& doc, EntityId id) override {
        // factor > 0 is still enforced inside ScaleEntity itself (see
        // Transform.hpp) -- this command does not duplicate that check;
        // it just forwards to the one function that owns the rule.
        return ScaleEntity(doc, id, factor_, pivot_);
    }

private:
    double factor_;
    geometry::Point2d pivot_;
};

}
