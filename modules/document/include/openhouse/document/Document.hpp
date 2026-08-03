#pragma once

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

// A shape plus the layer it belongs to. `layer` is a layer NAME (matches
// Layer's own identity model -- see Layer.hpp), not a pointer/index into
// Document's layer list. Document::Add() guarantees the named layer
// exists (auto-creating it if needed -- see Add()'s own comment), so in
// practice every Entity produced by Add() has a resolvable layer.
// Consumers (e.g. RenderToSvg) still look it up by name rather than
// caching a pointer/reference, since FindLayer is cheap at this scale
// and this avoids any risk of a dangling reference if Document's layer
// list is ever mutated after an Entity is created (e.g. layers_ vector
// reallocating).
//
// TODO(Spiral4):
// This name-based reference is the other half of Layer.hpp's LayerId
// TODO -- if a stable LayerId is introduced, this field changes with it,
// and any layer-rename implementation must update every Entity here.
struct Entity {
    Shape shape;
    foundation::string layer;
};

// A minimal CAD document: a list of Layers plus an ordered list of
// Entities (shape + layer assignment). Every Document starts with one
// layer, named "0" (matching the DXF/AutoCAD convention for the default
// layer -- the literal name "0", not "Layer0" or similar), so `Add()`
// with no explicit layer always has somewhere valid to go.
//
// Still intentionally minimal: no Selection yet (a later step in this
// Spiral), no entity handles/IDs for referencing a specific Entity later
// (needed once deletion/editing of individual entities exists), no
// nested/grouped layers. Each addition here is scoped to what Spiral 2
// actually specified (Layer + per-entity assignment), not built ahead of
// it.
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
    // doesn't already exist, then records the entity against it. This
    // matches Spiral 2's explicit requirement: `Add(shape)` (no layer
    // argument) still works exactly as before (goes to "0", which the
    // Document constructor already created), and `Add(shape, "Walls")`
    // creates "Walls" on first use rather than requiring a separate
    // CreateLayer("Walls") call beforehand. Because a layer is always
    // guaranteed to exist after Add() returns, RenderToSvg and Bounds()
    // can look up an entity's layer by name and only ever fail to find
    // it if a caller mutates layers_ directly in some unusual way (they
    // still handle a missing layer gracefully regardless, as defensive
    // coding, not because it's an expected path).
    void Add(Shape shape, foundation::string layerName = kDefaultLayerName) {
        CreateLayer(layerName); // no-op if it already exists
        entities_.push_back(Entity{foundation::move(shape), foundation::move(layerName)});
    }

    [[nodiscard]] const foundation::vector<Entity>& Entities() const noexcept { return entities_; }

    [[nodiscard]] std::size_t Count() const noexcept { return entities_.size(); }

    [[nodiscard]] bool Empty() const noexcept { return entities_.empty(); }

    // Clears entities only -- layer definitions are retained. Matches
    // the principle that Clear() empties the drawing, not the document's
    // structural setup (layers are closer to document configuration than
    // to drawing content).
    void Clear() noexcept { entities_.clear(); }

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
};

}
