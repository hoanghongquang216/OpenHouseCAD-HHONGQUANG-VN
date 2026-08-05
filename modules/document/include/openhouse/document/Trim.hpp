#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/document/EntityId.hpp>
#include <openhouse/geometry/Intersection.hpp>
#include <openhouse/geometry/Line2.hpp>
#include <openhouse/geometry/Projection.hpp>

#include <optional>
#include <variant>

namespace openhouse::document {

// TRIM-001: shortens a Line2 entity at the intersection nearest the
// point the user clicked, discarding the side of the intersection that
// contains the click. Scope (per TRIM-001 audit / ADR-0006):
// Line<->Line only -- Arc, Polyline, Circle explicitly deferred.
//
// Direct consumer of geometry::Intersect() and geometry::ParameterOnLine();
// does NOT depend on document::Snap and implements no geometric formula
// of its own, per ADR-0006's Document Services boundary.
[[nodiscard]] inline std::optional<geometry::Line2d> ComputeTrim(const Document& doc,
                                                                   EntityId targetId,
                                                                   EntityId cutterId,
                                                                   geometry::Point2d clickPoint) {
    const Entity* target = doc.FindEntity(targetId);
    const Entity* cutter = doc.FindEntity(cutterId);
    if (target == nullptr || cutter == nullptr) {
        return std::nullopt;
    }

    const Layer* layer = doc.FindLayer(target->layer);
    if (layer == nullptr || !layer->Visible() || layer->Locked()) {
        return std::nullopt;
    }

    const auto* targetLine = std::get_if<geometry::Line2d>(&target->shape);
    const auto* cutterLine = std::get_if<geometry::Line2d>(&cutter->shape);
    if (targetLine == nullptr || cutterLine == nullptr) {
        return std::nullopt;
    }

    const geometry::IntersectionResultd result = geometry::Intersect(*targetLine, *cutterLine);
    if (result.count == 0) {
        return std::nullopt;
    }
    const geometry::Point2d intersection = result.points[0];

    const double tIntersection = geometry::ParameterOnLine(*targetLine, intersection);
    const double tClick = geometry::ParameterOnLine(*targetLine, clickPoint);

    geometry::Line2d trimmed = *targetLine;
    if (tClick < tIntersection) {
        trimmed.start = intersection;
    } else {
        trimmed.end = intersection;
    }
    return trimmed;
}

// Mutating counterpart to ComputeTrim -- same TranslateEntity/
// RotateEntity/ScaleEntity split (Transform.hpp): applies the computed
// result directly to `doc`. Used by TrimCommand (via
// EntityShapeCommandBase, REFACTOR-001) instead of duplicating the
// find-entity-and-assign-shape logic that ComputeTrim's callers would
// otherwise each have to repeat.
[[nodiscard]] inline bool TrimEntity(Document& doc, EntityId targetId, EntityId cutterId,
                                      geometry::Point2d clickPoint) {
    const auto trimmed = ComputeTrim(doc, targetId, cutterId, clickPoint);
    if (!trimmed.has_value()) {
        return false;
    }
    Entity* entity = doc.FindEntityMutable(targetId);
    if (entity == nullptr) {
        return false;
    }
    entity->shape = *trimmed;
    return true;
}

}
