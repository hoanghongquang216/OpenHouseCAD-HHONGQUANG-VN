#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/document/EntityId.hpp>
#include <openhouse/geometry/BoundingBox2.hpp>
#include <openhouse/geometry/HitTest.hpp>

#include <optional>
#include <type_traits>
#include <variant>

namespace openhouse::document {

// The result of a successful HitTest: which entity, and how far the
// queried point was from it. `distance` is kept (not just a bool "did
// it hit") because later Spirals -- Snap, Nearest-point queries,
// Dimension -- all need "how close" as well as "which one", and
// computing it a second time there would be wasted, duplicated work.
struct HitResult {
    EntityId id;
    double distance;
};

// Finds the entity nearest to `point`, among entities within
// `tolerance` of it, on a visible layer. Returns std::nullopt if
// nothing qualifies (empty document, everything too far away, or
// everything on a hidden/unresolvable layer).
//
// Scans every entity (this function does not short-circuit on the
// first match) so the result is always the CLOSEST qualifying entity,
// not merely the first one encountered -- with several overlapping
// entities near the query point, "first" and "closest" are not
// generally the same entity.
//
// Locked layers are intentionally NOT excluded here (unlike hidden
// layers) -- Locked means "cannot be edited," not "cannot be selected
// or inspected." Many CAD tools allow selecting a locked entity to view
// its properties or use it as a reference (e.g. for Snap, once that
// exists) while still preventing modification. The actual editing block
// belongs wherever editing happens -- Transform (Spiral 4) -- not here.
//
// Performance: before the exact (and comparatively expensive)
// geometry::DistanceToShape() call, each entity's bounding box (already
// cheap to compute -- geometry::Bounds()) is dilated by `tolerance` and
// checked against `point` with a simple AABB test
// (geometry::Contains()). This is not a premature optimization tacked
// on afterward; it's treated as part of the hit-testing algorithm
// itself, on the reasoning that most entities in a non-trivial document
// are nowhere near the query point, and rejecting those cheaply matters
// once a document holds thousands of entities. The overall complexity
// remains O(n) either way -- this only reduces the constant factor by
// replacing most exact-distance calls with a cheap bounds check.
[[nodiscard]] inline std::optional<HitResult> HitTest(const Document& doc,
                                                        const geometry::Point2d& point,
                                                        double tolerance) {
    std::optional<HitResult> best;

    for (const auto& entity : doc.Entities()) {
        const Layer* layer = doc.FindLayer(entity.layer);
        if (layer == nullptr || !layer->Visible()) {
            continue;
        }

        const geometry::BoundingBox2d bounds =
            std::visit([](const auto& s) { return geometry::Bounds(s); }, entity.shape);
        const geometry::BoundingBox2d dilated = geometry::Dilate(bounds, tolerance);
        if (!geometry::Contains(dilated, point)) {
            continue; // fast-reject: definitely farther than `tolerance`
        }

        const double distance = std::visit(
            [&point](const auto& s) { return geometry::DistanceToShape(s, point); }, entity.shape);

        if (distance <= tolerance && (!best.has_value() || distance < best->distance)) {
            best = HitResult{entity.id, distance};
        }
    }

    return best;
}

}
