// Demonstrates Matrix3 with a REAL consumer -- unlike Matrix4/Angle,
// which the audit found had none. Draws a shape, then draws it again
// transformed (translated + rotated + scaled) via a single composed
// Matrix3, foreshadowing Spiral 3's "Move" capability.
#include <openhouse/math/Matrix3.hpp>
#include <openhouse/render/SvgDocument.hpp>
#include <cstdio>

int main(int argc, char** argv) {
    const char* outPath = (argc > 1) ? argv[1] : "transform_demo.svg";
    using namespace openhouse::geometry;
    using namespace openhouse::math;
    using namespace openhouse::render;

    SvgDocument doc(300.0, 200.0);

    // Original shape: a small triangle-ish mark via a circle + line, at origin-relative coords.
    const Point2d p0{0.0, 0.0};
    const Point2d p1{20.0, 0.0};

    // Draw original in gray, anchored near top-left.
    const Matrix3d place = Matrix3d::Translation(Vector2d{30.0, 30.0});
    doc.AddCircle(Circle2d{place * p0, 5.0}, 1.0, "gray");
    doc.AddLine(Line2d{place * p0, place * p1}, 1.0, "gray");

    // Transformed copy: translate elsewhere, rotate 45 degrees, scale up 2x --
    // a single composed Matrix3 applied to both the point and the line.
    const Matrix3d transform = Matrix3d::Translation(Vector2d{200.0, 120.0}) *
                                Matrix3d::Rotation(Angled::FromDegrees(45.0)) *
                                Matrix3d::UniformScale(2.0);
    doc.AddCircle(Circle2d{transform * p0, 5.0}, 2.0, "blue");
    doc.AddLine(Line2d{transform * p0, transform * p1}, 2.0, "blue");

    if (!doc.WriteToFile(outPath)) {
        std::fprintf(stderr, "Failed to write %s\n", outPath);
        return 1;
    }
    std::printf("Wrote %s\n", outPath);
    return 0;
}
