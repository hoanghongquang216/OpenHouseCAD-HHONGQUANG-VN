#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/document/EntityId.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>
#include <openhouse/geometry/Point2.hpp>

#include <optional>
#include <variant>
#include <vector>

namespace openhouse::document {

// Which geometric feature a SnapResult's point came from. Distinguishing
// this isn't speculative abstraction ahead of a need (contrast
// DxfErrorCode, deferred in DXF-ROBUST-003b for lack of a consumer) --
// telling Endpoint/Midpoint/Center apart IS the point of Snap to a
// user (a future UI shows a different marker per kind), so it belongs
// in the type from the start. SNAP-INTERSECTION-001 (see
// docs/SNAP_BACKLOG.md) will add `Intersection` when that Sprint
// happens -- not added speculatively here.
enum class SnapType {
    Endpoint,
    Midpoint,
    Center,
};

// A single snap match: which entity, what kind of point, where, and
// how far the query point was from it. `distance` is kept for the same
// reason document::HitResult keeps it (see HitTest.hpp) -- a future UI
// likely wants it (e.g. to break ties, or show it), and recomputing
// would be wasted work.
struct SnapResult {
    EntityId id;
    SnapType type;
    geometry::Point2d point;
    double distance;
};

namespace detail {

struct SnapCandidate {
    SnapType type;
    geometry::Point2d point;
};

// Every candidate snap point this Sprint recognizes for one shape.
// Only the point KINDS that are geometrically meaningful for a given
// shape are produced -- matching how other CAD tools treat these
// primitives, not an arbitrary choice:
//   - Circle2 has no Endpoint or Midpoint (a full circle has neither).
//   - Line2 has no Center (Midpoint already fills that role for a
//     straight segment).
[[nodiscard]] inline std::vector<SnapCandidate> SnapCandidatesFor(const geometry::Line2d& line) {
    return {
        {SnapType::Endpoint, line.start},
        {SnapType::Endpoint, line.end},
        {SnapType::Midpoint, geometry::Midpoint(line)},
    };
}

[[nodiscard]] inline std::vector<SnapCandidate> SnapCandidatesFor(const geometry::Circle2d& circle) {
    return {{SnapType::Center, circle.center}};
}

[[nodiscard]] inline std::vector<SnapCandidate> SnapCandidatesFor(const geometry::Arc2d& arc) {
    return {
        {SnapType::Endpoint, geometry::StartPoint(arc)},
        {SnapType::Endpoint, geometry::EndPoint(arc)},
        {SnapType::Midpoint, geometry::Midpoint(arc)},
        {SnapType::Center, arc.center},
    };
}

}

// Finds the nearest snap candidate point (Endpoint, Midpoint, or
// Center) to `point`, among entities within `tolerance`, on a visible
// layer. Mirrors document::HitTest()'s own shape exactly (Document +
// Point2 + tolerance -> the single nearest qualifying result) -- the
// same "pure query, no UI dependency" pattern already established and
// shipped for click-to-select (and, before that, for Transform/
// Command -- see docs/ROADMAP_EXECUTION.md), applied here to cursor-
// snap candidates instead of whole-entity hit testing. This is
// deliberately a single best match, not a candidate list: matches
// HitTest's own precedent, and nothing yet needs more than "the one
// point a cursor would snap to."
//
// Returns std::nullopt if nothing qualifies (empty document, nothing
// within tolerance, or everything on a hidden/unresolvable layer).
//
// Locked layers are intentionally NOT excluded here -- same reasoning
// as HitTest's own comment on this: Locked means "cannot be edited,"
// and a snap query, like a hit test, is a read-only lookup, not an
// edit. The actual editing block belongs wherever editing happens
// (Transform), not here.
//
// Deliberately does NOT use the AABB-dilate fast-reject
// document::HitTest() uses (dilating each shape's own Bounds() and
// checking the query point against that): that optimization would be
// WRONG here, not just unnecessary. A shape's Bounds() covers its
// drawn curve, not every snap candidate derived from it -- an Arc2's
// Center, in particular, can sit well outside the arc's own bounding
// box (e.g. a short arc segment far from its center), so filtering by
// the curve's bounds could silently discard a legitimately in-
// tolerance Center candidate. Snap's per-entity work is also already
// cheap (a fixed handful of point-to-point distance checks, not
// HitTest's more expensive DistanceToShape() projection), so the
// optimization has less to offer here even before that correctness
// problem -- see PERF-001 in docs/DXF_BACKLOG.md for this project's
// general stance on not optimizing ahead of evidence.
[[nodiscard]] inline std::optional<SnapResult> FindSnapPoint(const Document& doc,
                                                               const geometry::Point2d& point,
                                                               double tolerance) {
    std::optional<SnapResult> best;

    for (const auto& entity : doc.Entities()) {
        const Layer* layer = doc.FindLayer(entity.layer);
        if (layer == nullptr || !layer->Visible()) {
            continue;
        }

        const std::vector<detail::SnapCandidate> candidates = std::visit(
            [](const auto& shape) { return detail::SnapCandidatesFor(shape); }, entity.shape);

        for (const auto& candidate : candidates) {
            const double distance = geometry::Distance(point, candidate.point);
            if (distance <= tolerance && (!best.has_value() || distance < best->distance)) {
                best = SnapResult{entity.id, candidate.type, candidate.point, distance};
            }
        }
    }

    return best;
}

}
