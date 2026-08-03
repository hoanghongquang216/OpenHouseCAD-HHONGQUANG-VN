// Spiral 2 demo: draws two points connected by a line segment.
// Represents the "Draw Line" output capability of Milestone 2. The
// "Click" (interactive) half of Spiral 2 is deferred until a windowing
// stack decision is made (see conversation history / a future ADR);
// this demo proves the Geometry -> Render -> Output path for Line2.
#include <openhouse/render/SvgDocument.hpp>
#include <cstdio>

int main(int argc, char** argv) {
    const char* outPath = (argc > 1) ? argv[1] : "spiral2_output.svg";

    using namespace openhouse::geometry;
    using namespace openhouse::render;

    SvgDocument doc(200.0, 200.0);

    const Point2d a{30.0, 30.0};
    const Point2d b{170.0, 170.0};

    doc.AddLine(Line2d{a, b}, 2.0, "black");
    doc.AddPoint(a, 4.0, "red");
    doc.AddPoint(b, 4.0, "red");

    if (!doc.WriteToFile(outPath)) {
        std::fprintf(stderr, "Failed to write %s\n", outPath);
        return 1;
    }
    std::printf("Wrote %s\n", outPath);
    return 0;
}
