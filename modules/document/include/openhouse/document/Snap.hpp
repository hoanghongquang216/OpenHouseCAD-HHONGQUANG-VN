#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/document/EntityId.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Intersection.hpp>
#include <openhouse/geometry/Line2.hpp>
#include <openhouse/geometry/Point2.hpp>

#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

namespace openhouse::document {

// Which geometric feature a SnapResult's point came from. Distinguishing
// this isn't speculative abstraction ahead of a need (contrast
// DxfErrorCode, deferred in DXF-ROBUST-003b for lack of a consumer) --
// telling Endpoint/Midpoint/Center/Intersection apart IS the point of
// Snap to a user (a future UI shows a different marker per kind), so
// it belongs in the type from the start.
enum class SnapType {
    Endpoint,
    Midpoint,
    Center,
    Intersection,
};

// A single snap match: which entity, what kind of point, where, and
// how far the query point was from it. `distance` is kept for the same
// reason document::HitResult keeps it (see HitTest.hpp) -- a future UI
// likely wants it (e.g. to break ties, or show it), and recomputing
// would be wasted work.
//
// `entityId`/`relatedEntityId`, not `id`/`secondId`: for
// Endpoint/Midpoint/Center, the point belongs to exactly one entity,
// which `entityId` names -- there is no ordering between "first" and
// "second" entity for those, so a name implying one would mislead. For
// Intersection, the point is symmetric between the two entities that
// produced it (SNAP-INTERSECTION-001, see docs/SNAP_BACKLOG.md);
// `relatedEntityId` names the second one without implying either is
// primary. `relatedEntityId` is `std::nullopt` for every SnapType
// except Intersection.
//
// Deliberately not std::vector<EntityId> or std::array<EntityId, 2>:
// every SnapType this project has today involves at most two entities
// (one, or the pair forming an Intersection). Generalizing to an
// arbitrary-size entity list ahead of any actual need beyond that
// would be premature abstraction.
struct SnapResult {
    EntityId entityId;
    std::optional<EntityId> relatedEntityId;
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

// Intersection candidates for one PAIR of shapes -- SNAP-INTERSECTION-001's
// own scope (see docs/SNAP_BACKLOG.md). Deliberately thin: every
// geometric formula (Line-Line, Line-Circle, Circle-Circle, and the
// Arc filtering via AngleOnArc) already lives in geometry::Intersect(...)
// per GEOM-INTERSECTION-001 -- this just calls it and repackages the
// result. Per ADR-0006 (docs/ARCHITECTURE_DECISION_RECORDS), this file
// is Document Services, and must not grow a geometric formula of its
// own; if this function ever needs one, it belongs in `geometry`
// instead, not here.
//
// A generic lambda over both shape variants, rather than a switch over
// every concrete-type pair, because geometry::Intersect(...) already
// has an overload for each of the 9 (3x3) Line2/Circle2/Arc2
// combinations (per GEOM-INTERSECTION-001) -- overload resolution
// picks the right one, so there is nothing pair-specific to write here.
template<typename ShapeA, typename ShapeB>
[[nodiscard]] std::vector<geometry::Point2d> IntersectionCandidatesFor(const ShapeA& a, const ShapeB& b) {
    const auto result = geometry::Intersect(a, b);
    std::vector<geometry::Point2d> points;
    points.reserve(result.count);
    for (std::size_t i = 0; i < result.count; ++i) {
        points.push_back(result.points[i]);
    }
    return points;
}

}

// Finds the nearest snap candidate point (Endpoint, Midpoint, Center,
// or Intersection) to `point`, among entities within `tolerance`, on a
// visible layer. Mirrors document::HitTest()'s own shape exactly
// (Document + Point2 + tolerance -> the single nearest qualifying
// result) -- the same "pure query, no UI dependency" pattern already
// established and shipped for click-to-select (and, before that, for
// Transform/Command -- see docs/ROADMAP_EXECUTION.md), applied here to
// cursor-snap candidates instead of whole-entity hit testing. This is
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

    const auto& entities = doc.Entities();

    for (const auto& entity : entities) {
        const Layer* layer = doc.FindLayer(entity.layer);
        if (layer == nullptr || !layer->Visible()) {
            continue;
        }

        const std::vector<detail::SnapCandidate> candidates = std::visit(
            [](const auto& shape) { return detail::SnapCandidatesFor(shape); }, entity.shape);

        for (const auto& candidate : candidates) {
            const double distance = geometry::Distance(point, candidate.point);
            if (distance <= tolerance && (!best.has_value() || distance < best->distance)) {
                best = SnapResult{entity.id, std::nullopt, candidate.type, candidate.point, distance};
            }
        }
    }

    // Second pass, over PAIRS of entities, for Intersection candidates
    // (SNAP-INTERSECTION-001). Separate from the loop above rather than
    // folded into it: the loop above is inherently per-entity (one
    // shape produces its own candidates independent of any other
    // entity), while this is inherently per-pair -- forcing both into
    // one loop shape would make neither read clearly. i < j (not all
    // ordered pairs) since Intersection is symmetric -- geometry::Intersect
    // has both (A,B) and (B,A) overloads for exactly this reason (see
    // GEOM-INTERSECTION-001), so either order would produce the same
    // points; checking each unordered pair once avoids the redundant
    // second call.
    for (std::size_t i = 0; i < entities.size(); ++i) {
        const Layer* layerA = doc.FindLayer(entities[i].layer);
        if (layerA == nullptr || !layerA->Visible()) {
            continue;
        }

        for (std::size_t j = i + 1; j < entities.size(); ++j) {
            const Layer* layerB = doc.FindLayer(entities[j].layer);
            if (layerB == nullptr || !layerB->Visible()) {
                continue;
            }

            const std::vector<geometry::Point2d> points = std::visit(
                [](const auto& shapeA, const auto& shapeB) {
                    return detail::IntersectionCandidatesFor(shapeA, shapeB);
                },
                entities[i].shape, entities[j].shape);

            for (const auto& candidatePoint : points) {
                const double distance = geometry::Distance(point, candidatePoint);
                if (distance <= tolerance && (!best.has_value() || distance < best->distance)) {
                    best = SnapResult{
                        entities[i].id, entities[j].id, SnapType::Intersection, candidatePoint, distance};
                }
            }
        }
    }

    return best;
}

}
