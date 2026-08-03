#pragma once

#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Format.hpp>
#include <openhouse/foundation/Numbers.hpp>
#include <openhouse/foundation/String.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>
#include <openhouse/geometry/Point2.hpp>

#include <fstream>

namespace openhouse::render {

// Minimal SVG document builder. Scope grows one Spiral at a time (see
// docs/ROADMAP_EXECUTION.md spiral plan): Spiral 1 added points, Spiral 2
// added line segments, later revisions added circles, arcs, and an
// optional dash pattern (for Document::Layer's LineType). Further
// shapes/styling beyond that are deliberately deferred to later,
// narrowly-scoped tasks rather than speculatively added here.
//
// Coordinate system: SVG's native coordinate system is used as-is (origin
// top-left, Y increases downward). This does NOT match the conventional
// CAD/math convention (Y increases upward) used implicitly elsewhere in
// this codebase. No flip is applied in this first version -- this is a
// deliberate, documented simplification for Spiral 1, not an oversight.
// It must be revisited (with an explicit coordinate-mapping decision,
// likely worth its own ADR) once a Spiral introduces shapes where the
// visual orientation actually matters to a user looking at the output.
//
// `color` parameters are assumed to be developer-supplied, trusted SVG
// attribute values (e.g. "black", "#ff0000") -- they are NOT XML-escaped.
// If color (or any other string) ever originates from untrusted input
// (a loaded file, user text entry), escaping must be added before this
// class is safe to use with that input.
class SvgDocument {
public:
    explicit SvgDocument(double width = 200.0, double height = 200.0) noexcept
        : width_(width), height_(height) {}

    void AddPoint(const geometry::Point2d& p, double radius = 3.0,
                  foundation::string_view color = "black") {
        body_ += foundation::format(
            R"(<circle cx="{}" cy="{}" r="{}" fill="{}"/>)"
            "\n",
            p.x, p.y, radius, color);
    }

    // `dashArray`: raw SVG stroke-dasharray value (e.g. "6,4"), or empty
    // (the default) for a solid line -- when empty, no stroke-dasharray
    // attribute is emitted at all, so existing callers that don't pass
    // this parameter get byte-identical output to before this parameter
    // existed. This class intentionally does not know about
    // Document::LineType (that would make render depend on document for
    // a concept render doesn't need to understand) -- RenderToSvg is
    // where LineType gets translated into a concrete dash-array string.
    void AddLine(const geometry::Line2d& line, double strokeWidth = 1.0,
                 foundation::string_view color = "black",
                 foundation::string_view dashArray = "") {
        body_ += foundation::format(
            R"(<line x1="{}" y1="{}" x2="{}" y2="{}" stroke="{}" stroke-width="{}"{}/>)"
            "\n",
            line.start.x, line.start.y, line.end.x, line.end.y, color, strokeWidth,
            DashAttribute(dashArray));
    }

    // Unlike AddPoint (a filled dot marker), AddCircle draws an OUTLINE
    // (fill="none") -- a geometric Circle2 and a point marker are
    // visually distinct concepts and should not render identically just
    // because both happen to use SVG's <circle> element under the hood.
    void AddCircle(const geometry::Circle2d& circle, double strokeWidth = 1.0,
                   foundation::string_view color = "black",
                   foundation::string_view dashArray = "") {
        body_ += foundation::format(
            R"(<circle cx="{}" cy="{}" r="{}" fill="none" stroke="{}" stroke-width="{}"{}/>)"
            "\n",
            circle.center.x, circle.center.y, circle.radius, color, strokeWidth,
            DashAttribute(dashArray));
    }

    // Renders as an SVG <path> with an elliptical-arc command (rx=ry=
    // radius, so effectively circular). See the design note above
    // AddArc's implementation comment inline: startAngle/endAngle are
    // used as-is (no transformation), consistent with Arc2's own
    // PointAt() formula and with this class's documented decision not to
    // flip the Y axis.
    void AddArc(const geometry::Arc2d& arc, double strokeWidth = 1.0,
                foundation::string_view color = "black",
                foundation::string_view dashArray = "") {
        const geometry::Point2d start = geometry::StartPoint(arc);
        const geometry::Point2d end = geometry::EndPoint(arc);
        const double sweep = geometry::Sweep(arc);

        const int largeArcFlag = (foundation::abs(sweep) > foundation::pi_v<double>) ? 1 : 0;
        const int sweepFlag = (sweep > 0.0) ? 1 : 0;

        body_ += foundation::format(
            R"(<path d="M {} {} A {} {} 0 {} {} {} {}" fill="none" stroke="{}" stroke-width="{}"{}/>)"
            "\n",
            start.x, start.y, arc.radius, arc.radius, largeArcFlag, sweepFlag, end.x, end.y,
            color, strokeWidth, DashAttribute(dashArray));
    }

    [[nodiscard]] foundation::string ToString() const {
        return foundation::format(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            R"(<svg xmlns="http://www.w3.org/2000/svg" width="{}" height="{}" )"
            R"(viewBox="0 0 {} {}">)"
            "\n{}</svg>\n",
            width_, height_, width_, height_, body_);
    }

    // Returns true on success. Does not throw on I/O failure -- callers
    // that need to distinguish failure reasons should inspect the
    // filesystem/errno themselves; this keeps the render module's error
    // handling simple for Spiral 1's scope.
    [[nodiscard]] bool WriteToFile(const foundation::string& path) const {
        std::ofstream out(path, std::ios::out | std::ios::trunc);
        if (!out) {
            return false;
        }
        out << ToString();
        return static_cast<bool>(out);
    }

private:
    static foundation::string DashAttribute(foundation::string_view dashArray) {
        if (dashArray.empty()) {
            return "";
        }
        return foundation::format(R"( stroke-dasharray="{}")", dashArray);
    }

    double width_;
    double height_;
    foundation::string body_;
};

}
