// The first complete vertical slice: A1 (Geometry) -> A2 (Document) ->
// A4 (Renderer). Builds a Document containing several shapes, then
// renders the WHOLE document to a single SVG file in one call --
// proving Document and SvgDocument compose correctly, which is the
// entire point of this slice (see conversation history).
#include <openhouse/render/RenderDocument.hpp>
#include <cstdio>

int main(int argc, char** argv) {
    const char* outPath = (argc > 1) ? argv[1] : "document_demo.svg";
    using namespace openhouse::document;
    using namespace openhouse::geometry;
    using namespace openhouse::render;

    Document doc;
    doc.Add(Circle2d{Point2d{100.0, 100.0}, 60.0});
    doc.Add(Line2d{Point2d{40.0, 100.0}, Point2d{160.0, 100.0}});
    doc.Add(Line2d{Point2d{100.0, 40.0}, Point2d{100.0, 160.0}});
    doc.Add(Arc2d{Point2d{100.0, 100.0}, 80.0, 0.0, 3.14159265358979323846 / 2.0});

    std::printf("Document has %zu shapes.\n", doc.Count());

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg);

    if (!svg.WriteToFile(outPath)) {
        std::fprintf(stderr, "Failed to write %s\n", outPath);
        return 1;
    }
    std::printf("Wrote %s\n", outPath);
    return 0;
}
