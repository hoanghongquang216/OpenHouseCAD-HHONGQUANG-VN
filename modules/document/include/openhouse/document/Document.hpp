#pragma once

#include <openhouse/document/EntityId.hpp>
#include <openhouse/document/Layer.hpp>
#include <openhouse/foundation/Containers.hpp>
#include <openhouse/foundation/String.hpp>
#include <openhouse/foundation/Utility.hpp>
#include <openhouse/foundation/Variant.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Bounds.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>

#include <optional>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace openhouse::document {

// A Shape is any one of the drawable 2D primitives this project currently
// supports. Deliberately a closed set (std::variant), not a polymorphic
// base class with virtual dispatch -- there is no known need yet for
// user-extensible shape types, and a closed variant keeps every shape's
// storage inline (no heap allocation per shape, no vtable), which matters
// once a document holds many shapes. If/when a real need for open
// extensibility appears (e.g. a plugin system, per the roadmap's later
// Spirals), this is the type to revisit -- not something to speculatively
// solve now.
using Shape = foundation::variant<geometry::Line2d, geometry::Circle2d, geometry::Arc2d>;

// A shape plus the layer it belongs to and a stable EntityId (see
// EntityId.hpp -- assigned once by Document::Add(), never reused,
// independent of position in Document::Entities()). `layer` is a layer
// NAME (matches Layer's own identity model -- see Layer.hpp), not a
// pointer/index into Document's layer list. Document::Add() guarantees
// the named layer exists (auto-creating it if needed -- see Add()'s own
// comment), so in practice every Entity produced by Add() has a
// resolvable layer. Consumers (e.g. RenderToSvg) still look it up by
// name rather than caching a pointer/reference, since FindLayer is cheap
// at this scale and this avoids any risk of a dangling reference if
// Document's layer list is ever mutated after an Entity is created (e.g.
// layers_ vector reallocating).
//
// TODO(Spiral4):
// This name-based layer reference is the other half of Layer.hpp's
// LayerId TODO -- if a stable LayerId is introduced, this field changes
// with it, and any layer-rename implementation must update every Entity
// here.
struct Entity {
    EntityId id;
    Shape shape;
    foundation::string layer;
};

// A minimal CAD document: a list of Layers plus an ordered list of
// Entities (shape + layer assignment + stable ID). Every Document starts
// with one layer, named "0" (matching the DXF/AutoCAD convention for the
// default layer -- the literal name "0", not "Layer0" or similar), so
// `Add()` with no explicit layer always has somewhere valid to go.
//
// Entity removal (RemoveEntity) and id-preserving restoration (Restore)
// were added in COPY-001 (Sprint 4) -- see
// docs/design/COPY-001-Design.md Section 3 for the full contract these
// two methods are required to satisfy. Both exist to serve
// EntityCreationCommandBase's Undo/Redo (Command.hpp), not as a
// general-purpose "delete entity" user action -- DELETE-001 is expected
// to build on RemoveEntity directly, but does not exist yet.
class Document {
public:
    static constexpr const char* kDefaultLayerName = "0";

    Document() { CreateLayer(kDefaultLayerName); }

    // --- Layer management ----------------------------------------------

    // Creates a layer with the given name if one doesn't already exist,
    // and returns a reference to it either way (idempotent "get or
    // create", not an error to call with an existing name). This keeps
    // simple, common patterns -- e.g. a demo unconditionally ensuring
    // "Walls" exists before adding entities to it -- from needing
    // explicit existence checks, without inventing a full error-handling
    // policy (exceptions vs. std::expected vs. assert) prematurely for a
    // case that isn't really an error.
    Layer& CreateLayer(foundation::string name) {
        if (Layer* existing = FindLayer(name)) {
            return *existing;
        }
        layers_.emplace_back(foundation::move(name));
        return layers_.back();
    }

    [[nodiscard]] const Layer* FindLayer(foundation::string_view name) const noexcept {
        for (const auto& layer : layers_) {
            if (layer.Name() == name) {
                return &layer;
            }
        }
        return nullptr;
    }

    [[nodiscard]] Layer* FindLayer(foundation::string_view name) noexcept {
        for (auto& layer : layers_) {
            if (layer.Name() == name) {
                return &layer;
            }
        }
        return nullptr;
    }

    [[nodiscard]] const foundation::vector<Layer>& Layers() const noexcept { return layers_; }

    // --- Entity management -----------------------------------------------

    // Auto-creates `layerName` (via CreateLayer, so this is idempotent --
    // adding many entities to "Walls" only creates that layer once) if it
    // doesn't already exist, then records the entity against it and
    // assigns it a new, never-before-used EntityId (monotonically
    // increasing, starting at 1 -- see EntityId.hpp). Returns that ID so
    // callers that need to reference the entity later (Selection, hit-
    // testing) can do so without depending on its position in
    // Entities(). Existing callers that ignore the return value (every
    // call site before this Spiral) are unaffected -- this is an
    // additive change to Add()'s signature, not a behavioral one.
    //
    // Contract note (COPY-001 / Design §3): this is the ONLY way to
    // obtain a brand-new EntityId. Callers never get to pick the id --
    // see Restore() below for the distinct, narrowly-scoped method that
    // exists for the one legitimate case (Undo/Redo machinery) where an
    // id must be reused rather than freshly allocated.
    EntityId Add(Shape shape, foundation::string layerName = kDefaultLayerName) {
        CreateLayer(layerName); // no-op if it already exists
        const EntityId id = nextId_++;
        entities_.push_back(Entity{id, foundation::move(shape), foundation::move(layerName)});
        index_[id] = entities_.size() - 1;
        return id;
    }

    // Removes the entity identified by `id`, if it exists. Reindexes
    // every entity that came after the removed one in Entities() so that
    // FindEntity/FindEntityMutable stay correct for all of them --
    // resolving the tradeoff TODO(Spiral5) (see the original comment on
    // FindEntity, now folded into this method's implementation) flagged
    // as needing a deliberate decision once deletion was added. Chosen
    // approach: `entities_` stays a simple, insertion-order-preserving
    // vector (RenderToSvg's stacking order still depends on this, see
    // RenderDocumentTests.cpp's TestOrderIsPreserved), and `index_` is
    // repaired for the affected suffix on every removal -- O(n) in the
    // worst case (removing the first entity of a large document), judged
    // acceptable at CAD-drawing scale per COPY-001-Architecture-Audit.md's
    // risk assessment. Revisit only if profiling ever shows this is
    // actually hot.
    //
    // Does NOT touch nextId_ -- a removed id is never reissued to a new
    // logical entity by Add(). This is the same "IDs never reused"
    // invariant Clear() already documents, extended to per-entity
    // removal.
    //
    // Returns true if an entity was actually removed, false if `id`
    // didn't resolve to an existing entity (a no-op, not an error --
    // same convention as SelectionSet::Deselect and the *Entity
    // transform functions in Transform.hpp).
    bool RemoveEntity(EntityId id) {
        const auto it = index_.find(id);
        if (it == index_.end()) {
            return false;
        }
        const std::size_t pos = it->second;
        entities_.erase(entities_.begin() + static_cast<std::ptrdiff_t>(pos));
        index_.erase(it);
        // Every entity that was after `pos` shifted left by one position
        // when erase() closed the gap -- repair index_ for exactly that
        // suffix (entities before `pos` are untouched, no need to
        // revisit them).
        for (std::size_t i = pos; i < entities_.size(); ++i) {
            index_[entities_[i].id] = i;
        }
        return true;
    }

    // Re-adds an entity at a SPECIFIC, caller-supplied id, rather than
    // allocating a fresh one from nextId_. This is deliberately a
    // separate method from Add() -- not an overload -- so that Add()'s
    // contract ("you don't get to pick the id") stays honest for
    // ordinary callers; only Undo/Redo machinery (EntityCreationCommandBase's
    // Redo hook, Command.hpp) is expected to call this. See
    // docs/design/COPY-001-Design.md Section 3 for the full rationale.
    //
    // Precondition: `id` must have been previously issued by this same
    // Document's Add() (i.e. `id != kInvalidEntityId && id < nextId_`)
    // and must not currently resolve to a live entity. Both conditions
    // should be unreachable through the public Command surface -- they
    // are checked defensively here (returning false) rather than
    // OH_ASSERTed, so a violation is a safe no-op instead of a crash,
    // consistent with every other mutating Document method's "bool =
    // did this actually change anything" convention rather than being
    // the one exception that aborts.
    //
    // Does NOT touch nextId_. A restored id can never collide with a
    // subsequently Add()-ed fresh id, because nextId_ only ever
    // increases and a restored id is always strictly less than the
    // current nextId_ (it was issued before nextId_ advanced past it).
    bool Restore(EntityId id, Shape shape, foundation::string layerName) {
        if (id == kInvalidEntityId || id >= nextId_) {
            return false; // never a real id this Document issued
        }
        if (index_.find(id) != index_.end()) {
            return false; // already occupied by a live entity
        }
        CreateLayer(layerName); // no-op if it already exists
        entities_.push_back(Entity{id, foundation::move(shape), foundation::move(layerName)});
        index_[id] = entities_.size() - 1;
        return true;
    }

    [[nodiscard]] const foundation::vector<Entity>& Entities() const noexcept { return entities_; }

    // O(1) lookup by EntityId, via the index_ map maintained alongside
    // entities_. Kept correct across RemoveEntity() by that method's own
    // suffix-reindexing (see its comment above).
    [[nodiscard]] const Entity* FindEntity(EntityId id) const noexcept {
        const auto it = index_.find(id);
        if (it == index_.end()) {
            return nullptr;
        }
        return &entities_[it->second];
    }

    // Mutable counterpart to FindEntity(), added for Spiral 4 (Transform)
    // -- Document had no way to modify an existing entity's shape in
    // place until now (Entities()/FindEntity() are both read-only).
    // Deliberately still just "find and get a pointer", not a dedicated
    // "TransformEntity" method on Document itself -- the actual
    // transform logic (and its business rules: rejecting a locked or
    // hidden layer, validating a scale factor) lives in
    // document::TranslateEntity/RotateEntity/ScaleEntity (see
    // Transform.hpp), which use this accessor rather than duplicating
    // Document's internals. Keeping Document itself free of transform-
    // specific logic matches the same reasoning HitTest.hpp was kept
    // out of Document as a free function rather than a method.
    [[nodiscard]] Entity* FindEntityMutable(EntityId id) noexcept {
        const auto it = index_.find(id);
        if (it == index_.end()) {
            return nullptr;
        }
        return &entities_[it->second];
    }

    [[nodiscard]] std::size_t Count() const noexcept { return entities_.size(); }

    [[nodiscard]] bool Empty() const noexcept { return entities_.empty(); }

    // Clears entities only -- layer definitions are retained. Matches
    // the principle that Clear() empties the drawing, not the document's
    // structural setup (layers are closer to document configuration than
    // to drawing content).
    //
    // Deliberately does NOT reset the EntityId counter (nextId_): IDs
    // are never reused for the lifetime of a Document. If they were
    // reset and reused after Clear(), any EntityId held elsewhere from
    // before the Clear() (e.g. in a SelectionSet that wasn't also
    // cleared) could silently resolve to a completely different,
    // unrelated entity added afterward -- a correctness hazard worse
    // than FindEntity() simply returning nullptr for a stale ID, which
    // is what happens with monotonic, never-reused IDs.
    void Clear() noexcept {
        entities_.clear();
        index_.clear();
    }

    // Aggregate bounding box of every VISIBLE entity (the classic "Zoom
    // Extents" use case -- zooming to fit content the user can't see
    // would be a poor default). An entity on a hidden or unresolvable
    // layer is excluded, same as in RenderToSvg. Returns std::nullopt if
    // there is nothing visible to bound (either no entities at all, or
    // every entity is on a hidden/unknown layer) -- see Document.hpp's
    // original Bounds() rationale for why this returns an optional
    // rather than a default-constructed box.
    [[nodiscard]] std::optional<geometry::BoundingBox2d> Bounds() const {
        std::optional<geometry::BoundingBox2d> box;
        for (const auto& entity : entities_) {
            const Layer* layer = FindLayer(entity.layer);
            if (layer == nullptr || !layer->Visible()) {
                continue;
            }
            const geometry::BoundingBox2d entityBox =
                std::visit([](const auto& s) { return geometry::Bounds(s); }, entity.shape);
            box = box.has_value() ? geometry::Union(*box, entityBox) : entityBox;
        }
        return box;
    }

private:
    foundation::vector<Layer> layers_;
    foundation::vector<Entity> entities_;
    std::unordered_map<EntityId, std::size_t> index_;
    EntityId nextId_ = 1; // 0 is kInvalidEntityId; real IDs start at 1.
};

}
