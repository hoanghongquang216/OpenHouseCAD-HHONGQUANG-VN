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
        return std::nullopt;
    }

    const auto intersection = geometry::FindExtendIntersection(*targetLine, *boundaryLine);
    if (!intersection.has_value()) {
        return std::nullopt;
    }

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

// Mutating counterpart to ComputeExtend -- same TranslateEntity/
// RotateEntity/ScaleEntity split (Transform.hpp). Used by
// ExtendCommand (via EntityShapeCommandBase, REFACTOR-001).
[[nodiscard]] inline bool ExtendEntity(Document& doc, EntityId targetId, EntityId boundaryId,
                                        geometry::Point2d clickPoint) {
    const auto extended = ComputeExtend(doc, targetId, boundaryId, clickPoint);
    if (!extended.has_value()) {
        return false;
    }
    Entity* entity = doc.FindEntityMutable(targetId);
    if (entity == nullptr) {
        return false;
    }
    entity->shape = *extended;
    return true;
}

}
