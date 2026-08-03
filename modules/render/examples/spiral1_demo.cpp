// Spiral 1 demo: Foundation -> Geometry -> Render -> Output.
// Produces a single .svg file containing one point. This is the
// vertical-slice proof-of-life for the render module.
#include <openhouse/render/SvgDocument.hpp>
#include <cstdio>

int main(int argc, char** argv) {
    const char* outPath = (argc > 1) ? argv[1] : "spiral1_output.svg";

    openhouse::render::SvgDocument doc(200.0, 200.0);
    doc.AddPoint(openhouse::geometry::Point2d{100.0, 100.0}, 5.0, "black");

    if (!doc.WriteToFile(outPath)) {
        std::fprintf(stderr, "Failed to write %s\n", outPath);
        return 1;
    }
    std::printf("Wrote %s\n", outPath);
    return 0;
}
