#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/foundation/String.hpp>
#include <openhouse/render/SvgDocument.hpp>

#include <type_traits>
#include <variant>

namespace openhouse::render {

// Maps Document::LineType to an SVG stroke-dasharray value. Continuous
// maps to "" (empty), matching SvgDocument's own convention that an
// empty dashArray omits the attribute entirely (solid line). The dash/
// gap numbers below are reasonable fixed defaults, not configurable --
// if a real need for user-adjustable dash scale appears later (e.g. to
// match a specific CAD linetype's exact pattern from DXF), that's a
// concrete reason to revisit this, not something to parameterize ahead
// of that need.
[[nodiscard]] inline foundation::string_view DashArrayForLineType(document::LineType type) {
    switch (type) {
        case document::LineType::Continuous:
            return "";
        case document::LineType::Dashed:
            return "6,4";
        case document::LineType::Dotted:
            return "1,3";
        case document::LineType::DashDot:
            return "6,3,1,3";
    }
    return ""; // unreachable for a valid enum value; safe fallback over UB.
}

// Renders every entity in a Document into an SvgDocument, honoring each
// entity's Layer: color -> stroke, LineWeight -> stroke-width, LineType
// -> stroke-dasharray, and Visible() -- an entity on a hidden layer (or
// one whose layer name doesn't resolve, which shouldn't normally happen
// since Document::Add() always creates the named layer, but is handled
// defensively regardless) is skipped entirely, not rendered with some
// placeholder style. This is Spiral 2's explicit requirement: no Layer
// attribute is allowed to be "stored but unused" by the renderer --
// Locked is the one exception, since there is no Selection/Editing
// system yet for it to affect (see Layer.hpp's own comment on Locked).
inline void RenderToSvg(const document::Document& doc, SvgDocument& svg) {
    for (const auto& entity : doc.Entities()) {
        const document::Layer* layer = doc.FindLayer(entity.layer);
        if (layer == nullptr || !layer->Visible()) {
            continue;
        }

        const foundation::string_view color = layer->Color();
        const double lineWeight = layer->LineWeight();
        const foundation::string_view dashArray = DashArrayForLineType(layer->GetLineType());

        std::visit(
            [&svg, color, lineWeight, dashArray](const auto& s) {
                using T = std::decay_t<decltype(s)>;
                if constexpr (std::is_same_v<T, geometry::Line2d>) {
                    svg.AddLine(s, lineWeight, color, dashArray);
                } else if constexpr (std::is_same_v<T, geometry::Circle2d>) {
                    svg.AddCircle(s, lineWeight, color, dashArray);
                } else if constexpr (std::is_same_v<T, geometry::Arc2d>) {
                    svg.AddArc(s, lineWeight, color, dashArray);
                }
            },
            entity.shape);
    }
}

}
