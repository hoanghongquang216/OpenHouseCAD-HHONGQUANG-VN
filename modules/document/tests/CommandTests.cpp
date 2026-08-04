#include <openhouse/document/Command.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>
#include <variant>

using namespace openhouse::document;
using namespace openhouse::geometry;

namespace {
constexpr double kPi = 3.14159265358979323846;
} // namespace

// --- TranslateCommand ---------------------------------------------------

static void TestTranslateCommandExecuteMovesShape() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{10.0, 10.0}, 5.0});
    TranslateCommand cmd(id, Vector2d{100.0, 100.0});

    OH_CHECK(cmd.Execute(doc));

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->center.x == 110.0 && circle->center.y == 110.0);
}

static void TestTranslateCommandUndoRestoresExactPriorShape() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{10.0, 10.0}, 5.0});
    TranslateCommand cmd(id, Vector2d{100.0, 100.0});

    OH_CHECK(cmd.Execute(doc));
    cmd.Undo(doc);

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->center.x == 10.0 && circle->center.y == 10.0);
}

static void TestTranslateCommandRedoReappliesExactPostShape() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{10.0, 10.0}, 5.0});
    TranslateCommand cmd(id, Vector2d{100.0, 100.0});

    OH_CHECK(cmd.Execute(doc));
    cmd.Undo(doc);
    cmd.Redo(doc);

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->center.x == 110.0 && circle->center.y == 110.0);
}

static void TestTranslateCommandOnLockedLayerFailsAndChangesNothing() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{5.0, 5.0}, 3.0}, "Locked");
    doc.FindLayer("Locked")->SetLocked(true);
    TranslateCommand cmd(id, Vector2d{100.0, 100.0});

    OH_CHECK(!cmd.Execute(doc));

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->center.x == 5.0 && circle->center.y == 5.0);
}

static void TestTranslateCommandOnUnknownEntityFails() {
    Document doc;
    TranslateCommand cmd(99999, Vector2d{1.0, 1.0});
    OH_CHECK(!cmd.Execute(doc));
}

// --- RotateCommand --------------------------------------------------------

static void TestRotateCommandExecuteAndUndo() {
    Document doc;
    const EntityId id = doc.Add(Line2d{Point2d{10.0, 0.0}, Point2d{20.0, 0.0}});
    RotateCommand cmd(id, kPi / 2.0, Point2d{0.0, 0.0});

    OH_CHECK(cmd.Execute(doc));
    const auto* rotated = std::get_if<Line2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(std::abs(rotated->start.x - 0.0) < 1e-9);
    OH_CHECK(std::abs(rotated->start.y - 10.0) < 1e-9);

    cmd.Undo(doc);
    const auto* restored = std::get_if<Line2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(restored->start.x == 10.0 && restored->start.y == 0.0);
}

static void TestRotateCommandOnHiddenLayerFails() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Hidden");
    doc.FindLayer("Hidden")->SetVisible(false);
    RotateCommand cmd(id, kPi / 2.0, Point2d{0.0, 0.0});
    OH_CHECK(!cmd.Execute(doc));
}

// --- ScaleCommand -----------------------------------------------------

static void TestScaleCommandExecuteAndUndo() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{10.0, 10.0}, 5.0});
    ScaleCommand cmd(id, 2.0, Point2d{0.0, 0.0});

    OH_CHECK(cmd.Execute(doc));
    const auto* scaled = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(scaled->radius == 10.0);

    cmd.Undo(doc);
    const auto* restored = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(restored->radius == 5.0);
}

static void TestScaleCommandWithZeroFactorFailsAndChangesNothing() {
    // ScaleCommand does not duplicate the factor>0 check -- it just
    // forwards to ScaleEntity, which already enforces it. Confirms that
    // enforcement is still in effect when reached through a Command.
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{5.0, 5.0}, 3.0});
    ScaleCommand cmd(id, 0.0, Point2d{0.0, 0.0});

    OH_CHECK(!cmd.Execute(doc));

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->radius == 3.0);
}

static void TestScaleCommandWithNegativeFactorFails() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{5.0, 5.0}, 3.0});
    ScaleCommand cmd(id, -2.0, Point2d{0.0, 0.0});
    OH_CHECK(!cmd.Execute(doc));
}

// The single most important test in this file, per Spiral 5's design
// review: repeated Execute+Undo cycles must show ZERO accumulated
// floating-point drift, because Undo restores an exact snapshot rather
// than recomputing an inverse transform. This directly regression-tests
// the ~1.4e-14 drift that was demonstrated (with real code) BEFORE this
// design was locked -- see this file's own module-level comment on
// ICommand. Uses an off-origin pivot deliberately, since drift was only
// observable that way (an origin pivot has fewer arithmetic operations,
// hiding the issue).
static void TestScaleCommandOneHundredExecuteUndoCyclesShowZeroDrift() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{123.456, 789.012}, 17.3});
    const Circle2d original = *std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    const Point2d offPivot{55.5, 22.2};

    ScaleCommand cmd(id, 0.7, offPivot);
    for (int i = 0; i < 100; ++i) {
        OH_CHECK(cmd.Execute(doc));
        cmd.Undo(doc);
    }

    const auto* finalShape = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    // Bit-exact equality, not "nearly equal" -- that's the entire point
    // of the Memento/snapshot design.
    OH_CHECK(finalShape->radius == original.radius);
    OH_CHECK(finalShape->center.x == original.center.x);
    OH_CHECK(finalShape->center.y == original.center.y);
}

static void TestRotateCommandOneHundredExecuteUndoCyclesShowZeroDrift() {
    // Same regression, for Rotate -- sin/cos-based, a different (if
    // generally smaller) source of potential drift than Scale's
    // multiply/divide.
    Document doc;
    const EntityId id = doc.Add(Arc2d{Point2d{12.0, 34.0}, 5.6, 0.3, 1.9});
    const Arc2d original = *std::get_if<Arc2d>(&doc.FindEntity(id)->shape);
    const Point2d offPivot{-7.0, 3.0};

    RotateCommand cmd(id, kPi / 7.0, offPivot); // an "awkward" non-round angle
    for (int i = 0; i < 100; ++i) {
        OH_CHECK(cmd.Execute(doc));
        cmd.Undo(doc);
    }

    const auto* finalShape = std::get_if<Arc2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(finalShape->radius == original.radius);
    OH_CHECK(finalShape->center.x == original.center.x);
    OH_CHECK(finalShape->center.y == original.center.y);
    OH_CHECK(finalShape->startAngle == original.startAngle);
    OH_CHECK(finalShape->endAngle == original.endAngle);
}

int main() {
    TestTranslateCommandExecuteMovesShape();
    TestTranslateCommandUndoRestoresExactPriorShape();
    TestTranslateCommandRedoReappliesExactPostShape();
    TestTranslateCommandOnLockedLayerFailsAndChangesNothing();
    TestTranslateCommandOnUnknownEntityFails();

    TestRotateCommandExecuteAndUndo();
    TestRotateCommandOnHiddenLayerFails();

    TestScaleCommandExecuteAndUndo();
    TestScaleCommandWithZeroFactorFailsAndChangesNothing();
    TestScaleCommandWithNegativeFactorFails();
    TestScaleCommandOneHundredExecuteUndoCyclesShowZeroDrift();
    TestRotateCommandOneHundredExecuteUndoCyclesShowZeroDrift();

    std::puts("CommandTests: all tests passed.");
    return 0;
}
