// Demonstrates all three SVG primitives (point, line, circle) together --
// the complete output-side vocabulary as of RENDER-003.
#include <openhouse/render/SvgDocument.hpp>
#include <cstdio>

int main(int argc, char** argv) {
    const char* outPath = (argc > 1) ? argv[1] : "primitives_demo.svg";

    using namespace openhouse::geometry;
    using namespace openhouse::render;

    SvgDocument doc(200.0, 200.0);

    doc.AddCircle(Circle2d{Point2d{100.0, 100.0}, 60.0}, 2.0, "blue");
    doc.AddLine(Line2d{Point2d{40.0, 100.0}, Point2d{160.0, 100.0}}, 1.5, "gray");
    doc.AddPoint(Point2d{100.0, 100.0}, 4.0, "red");

    if (!doc.WriteToFile(outPath)) {
        std::fprintf(stderr, "Failed to write %s\n", outPath);
        return 1;
    }
    std::printf("Wrote %s\n", outPath);
    return 0;
}
