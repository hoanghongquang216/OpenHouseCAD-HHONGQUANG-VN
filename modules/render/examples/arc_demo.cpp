#include <openhouse/render/SvgDocument.hpp>
#include <cstdio>

int main(int argc, char** argv) {
    const char* outPath = (argc > 1) ? argv[1] : "arc_demo.svg";
    using namespace openhouse::geometry;
    using namespace openhouse::render;
    constexpr double kPi = 3.14159265358979323846;

    SvgDocument doc(200.0, 200.0);

    // Quarter arc, small, counter-clockwise: top-right quadrant.
    doc.AddArc(Arc2d{Point2d{100.0, 100.0}, 70.0, 0.0, kPi / 2.0}, 3.0, "blue");
    // Large arc (270 degrees): most of the circle, leaving one quadrant open.
    doc.AddArc(Arc2d{Point2d{100.0, 100.0}, 40.0, kPi / 2.0, 2.0 * kPi}, 2.0, "green");

    doc.AddPoint(Point2d{100.0, 100.0}, 3.0, "red");

    if (!doc.WriteToFile(outPath)) {
        std::fprintf(stderr, "Failed to write %s\n", outPath);
        return 1;
    }
    std::printf("Wrote %s\n", outPath);
    return 0;
}
