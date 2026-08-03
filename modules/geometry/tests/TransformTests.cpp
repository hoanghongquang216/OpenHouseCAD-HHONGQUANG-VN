#include <openhouse/geometry/Transform.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>

using namespace openhouse::geometry;

static void TestTranslateLine() {
    const Line2d line{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}};
    const Line2d moved = Translate(line, Vector2d{5.0, 3.0});
    OH_CHECK(moved.start.x == 5.0 && moved.start.y == 3.0);
    OH_CHECK(moved.end.x == 15.0 && moved.end.y == 3.0);
}

static void TestTranslateLineByZeroIsNoOp() {
    const Line2d line{Point2d{1.0, 2.0}, Point2d{3.0, 4.0}};
    const Line2d moved = Translate(line, Vector2d{0.0, 0.0});
    OH_CHECK(moved == line);
}

static void TestTranslateCircleMovesCenterKeepsRadius() {
    const Circle2d circle{Point2d{10.0, 10.0}, 5.0};
    const Circle2d moved = Translate(circle, Vector2d{-2.0, -2.0});
    OH_CHECK(moved.center.x == 8.0 && moved.center.y == 8.0);
    OH_CHECK(moved.radius == 5.0); // radius must be unaffected by translation
}

static void TestTranslateArcMovesCenterKeepsRadiusAndAngles() {
    const Arc2d arc{Point2d{0.0, 0.0}, 10.0, 0.5, 1.5};
    const Arc2d moved = Translate(arc, Vector2d{100.0, 100.0});
    OH_CHECK(moved.center.x == 100.0 && moved.center.y == 100.0);
    OH_CHECK(moved.radius == 10.0);
    // Angles are relative to the center, not world space -- a pure
    // translation must leave them exactly unchanged.
    OH_CHECK(moved.startAngle == 0.5);
    OH_CHECK(moved.endAngle == 1.5);
}

static void TestTranslateNegativeDelta() {
    const Line2d line{Point2d{0.0, 0.0}, Point2d{0.0, 0.0}};
    const Line2d moved = Translate(line, Vector2d{-5.0, -7.0});
    OH_CHECK(moved.start.x == -5.0 && moved.start.y == -7.0);
}

int main() {
    TestTranslateLine();
    TestTranslateLineByZeroIsNoOp();
    TestTranslateCircleMovesCenterKeepsRadius();
    TestTranslateArcMovesCenterKeepsRadiusAndAngles();
    TestTranslateNegativeDelta();

    std::puts("TransformTests: all tests passed.");
    return 0;
}
