#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/foundation/Numbers.hpp>
#include <openhouse/foundation/String.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>

#include <fstream>
#include <iomanip>
#include <optional>
#include <ostream>
#include <sstream>
#include <type_traits>
#include <variant>

// DXF Export (DXF-EXPORT-001). See docs/design/DXF-EXPORT-001*.md for the
// full design trail -- Domain Research, Architecture Audit, Architecture
// Design (this file implements that document's API contract exactly),
// Test Design.
//
// Deliberately two free functions, not a builder class -- Document -> DXF
// is a one-shot, complete transformation with no caller need to interleave
// other content mid-stream (see Design.md Section 2's own reasoning for why
// this does NOT mirror SvgDocument's stateful-builder shape, despite reusing
// RenderToSvg's std::visit dispatch PRINCIPLE in WriteEntities below).
namespace openhouse::dxf {

namespace detail {

// Exact inverse of DxfReader.hpp's detail::AciToSvgColor. Per DG-003
// (docs/design/DXF-EXPORT-001-Architecture-Audit.md), a color with no
// entry here produces NO group code 62 output at all -- never a guessed
// "nearest" ACI. This is intentionally NOT a general-purpose color
// mapping; it exists only to invert the one this project's own Import
// already produces.
[[nodiscard]] inline std::optional<int> SvgColorToAci(const foundation::string& color) {
    if (color == "red") return 1;
    if (color == "yellow") return 2;
    if (color == "#00ff00") return 3;
    if (color == "cyan") return 4;
    if (color == "blue") return 5;
    if (color == "magenta") return 6;
    if (color == "black") return 7;
    if (color == "#414141") return 8;
    if (color == "#808080") return 9;
    return std::nullopt;
}

// Exact inverse of DxfReader.hpp's detail::LineTypeNameToEnum. Unlike
// color, this is a closed, unambiguous bijection -- LineType is this
// project's own enum, so every value has exactly one canonical DXF name.
[[nodiscard]] inline foundation::string LineTypeEnumToName(document::LineType type) {
    switch (type) {
        case document::LineType::Continuous:
            return "CONTINUOUS";
        case document::LineType::Dashed:
            return "DASHED";
        case document::LineType::Dotted:
            return "DOTTED";
        case document::LineType::DashDot:
            return "DASHDOT";
    }
    return "CONTINUOUS"; // unreachable for a valid enum value; safe fallback over UB.
}

[[nodiscard]] inline double RadiansToDegrees(double radians) noexcept {
    return radians * 180.0 / foundation::pi_v<double>;
}

// Minimal HEADER -- only $ACADVER, targeting DXF R12 (matches Import's own
// scope). Per Design.md's Non-Goals: no other header variables are written;
// this Sprint does not attempt full header fidelity.
inline void WriteHeader(std::ostringstream& out) {
    out << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1009\n0\nENDSEC\n";
}

// TABLES/LAYER: one entry per doc.Layers(), in insertion order (per
// Design.md's Determinism guarantee -- Layers() is a vector, not a
// hash-based container). Realizes DG-001 (full TABLES section) and DG-003
// (color: exact match or omit, never a guess).
inline void WriteLayerTable(const document::Document& doc, std::ostringstream& out) {
    out << "0\nSECTION\n2\nTABLES\n0\nTABLE\n2\nLAYER\n";
    for (const auto& layer : doc.Layers()) {
        out << "0\nLAYER\n2\n" << layer.Name() << "\n70\n0\n";
        if (const std::optional<int> aci = SvgColorToAci(layer.Color()); aci.has_value()) {
            out << "62\n" << *aci << "\n";
        }
        // No `else` branch: per DG-003, an unmapped color means NO group
        // code 62 is written for this layer at all -- not a fallback
        // value. The DXF-standard default (omitted 62 -> ACI 7) applies
        // on the reading side, per Phase 2's confirmed research.
        out << "6\n" << LineTypeEnumToName(layer.GetLineType()) << "\n";
    }
    out << "0\nENDTAB\n0\nENDSEC\n";
}

inline void WriteLine(const geometry::Line2d& line, const foundation::string& layerName,
                       std::ostringstream& out) {
    out << "0\nLINE\n8\n"
        << layerName << "\n10\n"
        << line.start.x << "\n20\n"
        << line.start.y << "\n11\n"
        << line.end.x << "\n21\n"
        << line.end.y << "\n";
}

inline void WriteCircle(const geometry::Circle2d& circle, const foundation::string& layerName,
                         std::ostringstream& out) {
    out << "0\nCIRCLE\n8\n"
        << layerName << "\n10\n"
        << circle.center.x << "\n20\n"
        << circle.center.y << "\n40\n"
        << circle.radius << "\n";
}

inline void WriteArc(const geometry::Arc2d& arc, const foundation::string& layerName,
                      std::ostringstream& out) {
    out << "0\nARC\n8\n"
        << layerName << "\n10\n"
        << arc.center.x << "\n20\n"
        << arc.center.y << "\n40\n"
        << arc.radius << "\n50\n"
        << RadiansToDegrees(arc.startAngle) << "\n51\n"
        << RadiansToDegrees(arc.endAngle) << "\n";
}

// ENTITIES: one record per doc.Entities(), in document order (Determinism
// guarantee, same reasoning as WriteLayerTable above). Dispatches via
// std::visit/if constexpr over entity.shape -- reuses RenderToSvg's
// dispatch PRINCIPLE (see RenderDocument.hpp), not its structure: these
// Write* functions have no relationship to SvgDocument::Add* beyond
// matching the same three Shape alternatives.
//
// Realizes DG-002: every entity is written regardless of its layer's
// Visible() state -- no filtering, unlike RenderToSvg's visible/selected
// two-pass split. A single pass over doc.Entities() is sufficient.
inline void WriteEntities(const document::Document& doc, std::ostringstream& out) {
    out << "0\nSECTION\n2\nENTITIES\n";
    for (const auto& entity : doc.Entities()) {
        std::visit(
            [&out, &entity](const auto& shape) {
                using T = std::decay_t<decltype(shape)>;
                if constexpr (std::is_same_v<T, geometry::Line2d>) {
                    WriteLine(shape, entity.layer, out);
                } else if constexpr (std::is_same_v<T, geometry::Circle2d>) {
                    WriteCircle(shape, entity.layer, out);
                } else if constexpr (std::is_same_v<T, geometry::Arc2d>) {
                    WriteArc(shape, entity.layer, out);
                }
            },
            entity.shape);
    }
    out << "0\nENDSEC\n";
}

} // namespace detail

// Writes `doc` as a DXF R12 stream: HEADER, TABLES/LAYER, ENTITIES, EOF.
// Returns true on success. The only failure mode is a stream write error
// -- there is no "malformed Document" case, since Document/Shape/Layer
// are already well-typed C++ data, unlike DxfReader's job of validating
// untrusted external text.
//
// No partial/corrupt output on failure: the entire document is built as
// a single in-memory string first (every Write* call above is string
// concatenation, which cannot fail), then written to `out` in one `<<`
// call, with `out`'s failure state checked exactly once at the end.
// There is no midpoint where HEADER/TABLES succeeded but ENTITIES is
// missing.
[[nodiscard]] inline bool WriteDxfStream(const document::Document& doc, std::ostream& out) {
    std::ostringstream buffer;
    // max_digits10 (17 for double) guarantees a lossless round-trip
    // through decimal text -- default stream precision (6 significant
    // digits) would silently truncate coordinates, breaking Phase 4's
    // R-001 round-trip fidelity requirement.
    buffer << std::setprecision(17);

    detail::WriteHeader(buffer);
    detail::WriteLayerTable(doc, buffer);
    detail::WriteEntities(doc, buffer);
    buffer << "0\nEOF\n";

    out << buffer.str();
    return static_cast<bool>(out);
}

// Convenience wrapper matching SvgDocument::WriteToFile's existing
// convention: opens the file, delegates to WriteDxfStream, returns false
// without throwing on I/O failure (including the file failing to open).
[[nodiscard]] inline bool WriteDxfFile(const document::Document& doc, const foundation::string& path) {
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out) {
        return false;
    }
    return WriteDxfStream(doc, out);
}

}
