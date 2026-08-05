#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/document/EntityId.hpp>
#include <openhouse/geometry/Line2.hpp>
#include <openhouse/geometry/Point2.hpp>
#include <openhouse/geometry/Projection.hpp>

#include <optional>
#include <variant>

namespace openhouse::document {

// EXTEND-001: lengthens a Line2 entity so its nearer end reaches
// `boundary`. Scope (per EXTEND-001 audit): Line<->Line only, "No
// extend" mode (AutoCAD EDGEMODE=0) -- `boundary` stays a bounded
// segment; only `target` is treated as extending infinitely. See
// geometry::FindExtendIntersection's own comment for the full
// EDGEMODE rationale and the EXTEND-002 backlog item for the deferred
// "implied edge" (EDGEMODE=1) mode.
//
// `targetId`   -- the entity being extended (lengthened).
// `boundaryId` -- the entity providing the edge to extend to.
// `clickPoint` -- where the user clicked on `targetId`; used only to
//                 decide WHICH end of the target moves (the end
//                 nearer clickPoint is the one extended).
//
// Returns the new (lengthened) Line2 on success, std::nullopt if:
//   - either id fails to resolve to an entity,
//   - either entity's shape is not a Line2,
//   - the target's layer disallows editing (same Locked/Visible
//     semantics as CanTransform, Transform.hpp),
//   - target's infinite line does not meet boundary's own segment
//     bounds (geometry::FindExtendIntersection already enforces this).
//
// Does NOT mutate `doc` -- mirrors ComputeTrim's own split (Trim.hpp):
// this computes the result; ExtendCommand (ExtendCommand.hpp) applies
// it and participates in Undo/Redo.
[[nodiscard]] inline std::optional<geometry::Line2d> ComputeExtend(const Document& doc,
                                                                     EntityId targetId,
                                                                     EntityId boundaryId,
                                                                     geometry::Point2d clickPoint) {
    const Entity* target = doc.FindEntity(targetId);
    const Entity* boundary = doc.FindEntity(boundaryId);
    if (target == nullptr || boundary == nullptr) {
        return std::nullopt;
    }

    const Layer* layer = doc.FindLayer(target->layer);
    if (layer == nullptr || !layer->Visible() || layer->Locked()) {
        return std::nullopt;
    }

    const auto* targetLine = std::get_if<geometry::Line2d>(&target->shape);
    const auto* boundaryLine = std::get_if<geometry::Line2d>(&boundary->shape);
    if (targetLine == nullptr || boundaryLine == nullptr) {
        return std::nullopt; // Arc/Circle out of EXTEND-001 scope.
    }

    const auto intersection = geometry::FindExtendIntersection(*targetLine, *boundaryLine);
    if (!intersection.has_value()) {
        return std::nullopt;
    }

    // Which end is nearer clickPoint decides which end moves -- same
    // "click decides the side" convention as ComputeTrim (Trim.hpp),
    // just extending the nearer end instead of cutting at it.
    const double distToStart = geometry::Distance(clickPoint, targetLine->start);
    const double distToEnd = geometry::Distance(clickPoint, targetLine->end);

    geometry::Line2d extended = *targetLine;
    if (distToStart < distToEnd) {
        extended.start = *intersection;
    } else {
        extended.end = *intersection;
    }
    return extended;
}

}
