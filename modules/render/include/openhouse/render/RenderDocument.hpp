#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/document/Selection.hpp>
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

// Optional overlays a render pass can draw on top of a Document's own
// layer-driven appearance. A container for render-time concerns in
// general (Spiral 3 only uses `selection`; `drawOrigin`,
// `drawBoundingBoxes`, and `drawDebugInfo` are placeholders for later
// Spirals -- Snap points, debug grips, etc. -- so that adding the next
// overlay doesn't mean adding another raw pointer parameter to
// RenderToSvg's signature every time). Fields beyond `selection` are
// currently unused by RenderToSvg itself; they exist as a stable place
// for future overlays to land, not as speculative behavior implemented
// ahead of need.
struct RenderOptions {
    const document::SelectionSet* selection = nullptr;
    bool drawOrigin = false;
    bool drawBoundingBoxes = false;
    bool drawDebugInfo = false;
};

namespace detail {

// Selection highlight style: fixed constants, not derived from the
// entity's own layer and not configurable (per SEL-003's design
// review) -- a simple, always-legible convention now, revisited only if
// a real need for customization appears later.
inline constexpr foundation::string_view kSelectionColor = "#ff0000";
inline constexpr double kSelectionStrokeWidthBonus = 1.5;

} // namespace detail

// Renders every entity in a Document into an SvgDocument, honoring each
// entity's Layer: color -> stroke, LineWeight -> stroke-width, LineType
// -> stroke-dasharray, and Visible() -- an entity on a hidden layer (or
// one whose layer name doesn't resolve, which shouldn't normally happen
// since Document::Add() always creates the named layer, but is handled
// defensively regardless) is skipped entirely, not rendered with some
// placeholder style.
//
// `options.selection`, if given, overlays the fixed selection style
// (see detail::kSelectionColor/kSelectionStrokeWidthBonus above) on top
// of an entity's own color and line weight -- NOT its dash pattern,
// which is left as-is, so a selected dashed centerline still reads as
// "dashed" while highlighted. This is layered ON TOP of the entity's
// normal appearance at render time only; nothing about the Document,
// Entity, or Layer data is touched by selection state (Selection is
// session/UI state, not drawing data -- see Selection.hpp's own
// rationale).
//
// Selected entities are rendered in a SECOND pass, after every
// unselected entity, specifically so the highlight is never visually
// covered by an unselected entity drawn later in Z-order -- selection
// always ends up on top, regardless of each entity's original position
// in Document::Entities().
//
// `options` defaults to `{}` (no selection, all other overlays off), so
// existing two-argument calls (`RenderToSvg(doc, svg)`) are unaffected
// and produce byte-identical output to before RenderOptions existed --
// verified explicitly in RenderDocumentTests.cpp
// (TestEmptySelectionProducesIdenticalOutputToNoSelectionArgument).
// Locked is deliberately NOT consulted here -- see Layer.hpp's own
// comment: Locked gates editing (Transform, Spiral 4), not rendering or
// selection.
inline void RenderToSvg(const document::Document& doc, SvgDocument& svg,
                         const RenderOptions& options = {}) {
    const auto isSelected = [&options](document::EntityId id) noexcept {
        return options.selection != nullptr && options.selection->IsSelected(id);
    };

    const auto renderOne = [&svg](const document::Entity& entity, const document::Layer& layer,
                                   bool highlighted) {
        foundation::string_view color = layer.Color();
        double lineWeight = layer.LineWeight();
        const foundation::string_view dashArray = DashArrayForLineType(layer.GetLineType());

        if (highlighted) {
            color = detail::kSelectionColor;
            lineWeight = layer.LineWeight() + detail::kSelectionStrokeWidthBonus;
        }

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
    };

    // Pass 1: everything NOT selected, in original document order.
    for (const auto& entity : doc.Entities()) {
        if (isSelected(entity.id)) {
            continue;
        }
        const document::Layer* layer = doc.FindLayer(entity.layer);
        if (layer == nullptr || !layer->Visible()) {
            continue;
        }
        renderOne(entity, *layer, false);
    }

    // Pass 2: selected entities, drawn last (on top of everything above).
    // A selection containing an EntityId that doesn't resolve to any
    // entity in `doc` (e.g. a stale ID left over after Document::Clear())
    // simply never matches isSelected() for any real entity here -- no
    // special handling needed, no crash risk, nothing to skip.
    for (const auto& entity : doc.Entities()) {
        if (!isSelected(entity.id)) {
            continue;
        }
        const document::Layer* layer = doc.FindLayer(entity.layer);
        if (layer == nullptr || !layer->Visible()) {
            continue;
        }
        renderOne(entity, *layer, true);
    }
}

}
