// Spiral 3 demo (SEL-003, the final milestone closing out Spiral 3):
// loads a real .dxf file, hit-tests a query point against it to find an
// entity, selects that entity, and renders the result -- proving the
// full pipeline (DXF -> Document -> HitTest -> SelectionSet -> SVG with
// highlight) works end to end. No SelectionSet is built by hand here on
// purpose: per SEL-003's design review, the demo should reflect the
// real interaction flow (click point -> hit-test -> select), not a
// shortcut that skips straight to constructing a selection.
#include <openhouse/dxf/DxfReader.hpp>
#include <openhouse/document/HitTest.hpp>
#include <openhouse/render/RenderDocument.hpp>

#include <cstdio>

// The bundled sample .dxf's path, embedded at compile time (see
// modules/dxf/CMakeLists.txt's target_compile_definitions) rather than
// taken as a required runtime argument. This matches every other demo's
// invocation convention -- ./scripts/demo.sh runs each one as
// `executable <output.svg>` (a single, OUTPUT-path argument; see
// scripts/demo.sh) -- so this demo's own single optional argument is
// consistently the output path too, not an input file, avoiding an
// argument-order mismatch that would silently break under the
// automated demo sweep.
//
// TODO(later Spiral):
// A compile-time absolute source-tree path only works for a from-source
// build; it won't resolve correctly for an installed/packaged binary
// (see docs/ROADMAP_EXECUTION.md's later packaging concerns). The more
// portable fix -- locate sample_house.dxf relative to the running
// executable's own path (e.g. next to it, in an installed "demo/"
// resources directory) -- is deliberately deferred: it's a real
// improvement, not blocking SEL-003, and belongs with whatever Spiral
// first does real packaging/installation work, not guessed at here
// ahead of that need.
#ifndef OPENHOUSE_SAMPLE_HOUSE_DXF
#define OPENHOUSE_SAMPLE_HOUSE_DXF "sample_house.dxf"
#endif

int main(int argc, char** argv) {
    const char* svgPath = (argc > 1) ? argv[1] : "selection_demo_output.svg";
    const char* dxfPath = OPENHOUSE_SAMPLE_HOUSE_DXF;

    // Simulates a mouse click at this point (in the same coordinate
    // space the DXF file's own entities use). Chosen to land exactly on
    // the window/circle fixture in sample_house.dxf (center (50,50),
    // radius 15) -- see modules/dxf/examples/sample_house.dxf.
    const openhouse::geometry::Point2d clickPoint{65.0, 50.0};
    constexpr double kHitTolerance = 2.0;

    auto parsed = openhouse::dxf::ParseDxfFile(dxfPath);
    if (!parsed.has_value()) {
        std::fprintf(stderr, "Failed to parse %s: %s\n", dxfPath, parsed.error().c_str());
        return 1;
    }
    openhouse::document::Document& doc = *parsed;
    std::printf("Parsed %s: %zu entities across %zu layers.\n", dxfPath, doc.Count(),
                doc.Layers().size());

    // --- HitTest: simulate the user clicking at clickPoint -----------------
    const auto hit = openhouse::document::HitTest(doc, clickPoint, kHitTolerance);
    if (!hit.has_value()) {
        std::printf("No entity found within %.1f units of (%.1f, %.1f).\n", kHitTolerance,
                    clickPoint.x, clickPoint.y);
        // Still render (unselected) rather than treating a miss as fatal --
        // a real click that misses everything is a normal, expected
        // outcome, not an error.
    } else {
        std::printf("Hit entity #%llu at distance %.4f.\n",
                    static_cast<unsigned long long>(hit->id), hit->distance);
    }

    // --- Selection: select whatever HitTest found (if anything) ------------
    openhouse::document::SelectionSet selection;
    if (hit.has_value()) {
        (void)selection.Select(hit->id);
    }

    // --- Render: selected entity highlighted in the output SVG -------------
    openhouse::render::RenderOptions options;
    options.selection = &selection;

    openhouse::render::SvgDocument svg(200.0, 200.0);
    openhouse::render::RenderToSvg(doc, svg, options);

    if (!svg.WriteToFile(svgPath)) {
        std::fprintf(stderr, "Failed to write %s\n", svgPath);
        return 1;
    }
    std::printf("Wrote %s (%s)\n", svgPath,
                hit.has_value() ? "with highlighted selection" : "with no selection");
    return 0;
}
