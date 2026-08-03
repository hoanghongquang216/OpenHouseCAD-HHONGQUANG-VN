// Spiral 3 demo (DXF-001 backlog item, kept as a working reference):
// reads a real .dxf file from disk, parses it into a Document, and
// renders that Document to SVG -- proving the full DXF -> Document ->
// SvgDocument pipeline works end to end, not just that the parser
// passes unit tests in isolation.
#include <openhouse/dxf/DxfReader.hpp>
#include <openhouse/render/RenderDocument.hpp>

#include <cstdio>

// See selection_demo.cpp's identical comment: the bundled sample .dxf
// path is embedded at compile time (modules/dxf/CMakeLists.txt) so this
// demo's single optional argument matches every other demo's
// convention (an OUTPUT path, per scripts/demo.sh) instead of requiring
// an input path as argv[1] -- which would silently break under the
// automated demo sweep, since demo.sh always calls
// `executable <output.svg>` with exactly one, output-meaning, argument.
//
// TODO(later Spiral): see selection_demo.cpp's identical TODO -- a
// compile-time absolute path doesn't survive packaging/installation;
// deferred to whatever Spiral first does that work.
#ifndef OPENHOUSE_SAMPLE_HOUSE_DXF
#define OPENHOUSE_SAMPLE_HOUSE_DXF "sample_house.dxf"
#endif

int main(int argc, char** argv) {
    const char* svgPath = (argc > 1) ? argv[1] : "dxf_import_output.svg";
    const char* dxfPath = OPENHOUSE_SAMPLE_HOUSE_DXF;

    auto result = openhouse::dxf::ParseDxfFile(dxfPath);
    if (!result.has_value()) {
        std::fprintf(stderr, "Failed to parse %s: %s\n", dxfPath, result.error().c_str());
        return 1;
    }

    openhouse::document::Document& doc = *result;
    std::printf("Parsed %s: %zu entities across %zu layers.\n", dxfPath, doc.Count(),
                doc.Layers().size());

    // KNOWN LIMITATION, not fixed here: SvgDocument's viewBox is always
    // "0 0 width height" (see SvgDocument.hpp) -- it has no support for
    // an offset origin. A DXF file whose geometry sits far from (0,0)
    // would render outside the visible viewBox with a naive auto-size-
    // from-bounds approach (width/height alone, without translating
    // content to match). Rather than ship that half-working behavior,
    // this demo uses a fixed canvas and expects example DXF content
    // that's already near the origin -- correct for the bundled sample
    // file, but real-world DXF files (which routinely use large
    // absolute coordinates from a site survey, etc.) will need
    // SvgDocument to gain viewBox-offset support before DXF import is
    // useful on arbitrary real files. Worth a follow-up task.
    openhouse::render::SvgDocument svg(200.0, 200.0);
    openhouse::render::RenderToSvg(doc, svg);

    if (!svg.WriteToFile(svgPath)) {
        std::fprintf(stderr, "Failed to write %s\n", svgPath);
        return 1;
    }
    std::printf("Wrote %s\n", svgPath);
    return 0;
}
