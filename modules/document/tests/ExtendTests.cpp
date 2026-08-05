#include <openhouse/document/Extend.hpp>
#include <openhouse/document/ExtendCommand.hpp>
#include <openhouse/testing/Check.hpp>

#include <cmath>
#include <cstdio>

using namespace openhouse::document;
using namespace openhouse::geometry;

namespace {

bool NearlyEqual(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
}

bool PointNearlyEqual(const Point2d& a, const Point2d& b, double eps = 1e-6) {
    return NearlyEqual(a.x, b.x, eps) && NearlyEqual(a.y, b.y, eps);
}

} // namespace

// Target: (0,0)-(3,0), short. Boundary: (5,-5)-(5,5), crosses target's
// infinite extension at (5,0). Click near the END (3,0) -> extend the
// end forward to (5,0).
static void TestExtendClickNearEndExtendsEnd() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {3.0, 0.0}});
    const EntityId boundaryId = doc.Add(Line2d{{5.0, -5.0}, {5.0, 5.0}});
    const auto extended = ComputeExtend(doc, targetId, boundaryId, Point2d{3.0, 0.0});
    OH_CHECK(extended.has_value());
    OH_CHECK(PointNearlyEqual(extended->start, Point2d{0.0, 0.0}));
    OH_CHECK(PointNearlyEqual(extended->end, Point2d{5.0, 0.0}));
}

// Same target, click near START (0,0) -> extend the start backward to
// wherever the boundary at x=-5 sits.
static void TestExtendClickNearStartExtendsStart() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {3.0, 0.0}});
    const EntityId boundaryId = doc.Add(Line2d{{-5.0, -5.0}, {-5.0, 5.0}});
    const auto extended = ComputeExtend(doc, targetId, boundaryId, Point2d{0.0, 0.0});
    OH_CHECK(extended.has_value());
    OH_CHECK(PointNearlyEqual(extended->start, Point2d{-5.0, 0.0}));
    OH_CHECK(PointNearlyEqual(extended->end, Point2d{3.0, 0.0}));
}

// Boundary segment too short to actually reach the crossing point of
// the infinite lines -- EDGEMODE=0 (boundary stays bounded), so this
// must fail.
static void TestExtendFailsWhenBoundaryTooShort() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {3.0, 0.0}});
    const EntityId boundaryId = doc.Add(Line2d{{5.0, 1.0}, {5.0, 5.0}});
    const auto extended = ComputeExtend(doc, targetId, boundaryId, Point2d{3.0, 0.0});
    OH_CHECK(!extended.has_value());
}

// Parallel lines never intersect.
static void TestExtendParallelLinesReturnsNullopt() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {3.0, 0.0}});
    const EntityId boundaryId = doc.Add(Line2d{{0.0, 5.0}, {10.0, 5.0}});
    const auto extended = ComputeExtend(doc, targetId, boundaryId, Point2d{3.0, 0.0});
    OH_CHECK(!extended.has_value());
}

static void TestExtendCommandExecuteLengthensTarget() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {3.0, 0.0}});
    const EntityId boundaryId = doc.Add(Line2d{{5.0, -5.0}, {5.0, 5.0}});
    ExtendCommand cmd(targetId, boundaryId, Point2d{3.0, 0.0});
    OH_CHECK(cmd.Execute(doc));
    const auto* line = std::get_if<Line2d>(&doc.FindEntity(targetId)->shape);
    OH_CHECK(line != nullptr);
    OH_CHECK(PointNearlyEqual(line->start, Point2d{0.0, 0.0}));
    OH_CHECK(PointNearlyEqual(line->end, Point2d{5.0, 0.0}));
}

static void TestExtendCommandExecuteFailsWhenBoundaryTooShort() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {3.0, 0.0}});
    const EntityId boundaryId = doc.Add(Line2d{{5.0, 1.0}, {5.0, 5.0}});
    ExtendCommand cmd(targetId, boundaryId, Point2d{3.0, 0.0});
    OH_CHECK(!cmd.Execute(doc));
    const auto* line = std::get_if<Line2d>(&doc.FindEntity(targetId)->shape);
    OH_CHECK(PointNearlyEqual(line->start, Point2d{0.0, 0.0}));
    OH_CHECK(PointNearlyEqual(line->end, Point2d{3.0, 0.0}));
}

static void TestExtendCommandUndoRestoresExactPriorShape() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {3.0, 0.0}});
    const EntityId boundaryId = doc.Add(Line2d{{5.0, -5.0}, {5.0, 5.0}});
    ExtendCommand cmd(targetId, boundaryId, Point2d{3.0, 0.0});
    OH_CHECK(cmd.Execute(doc));
    cmd.Undo(doc);
    const auto* line = std::get_if<Line2d>(&doc.FindEntity(targetId)->shape);
    OH_CHECK(PointNearlyEqual(line->start, Point2d{0.0, 0.0}));
    OH_CHECK(PointNearlyEqual(line->end, Point2d{3.0, 0.0}));
}

static void TestExtendCommandRedoReappliesExactPostShape() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {3.0, 0.0}});
    const EntityId boundaryId = doc.Add(Line2d{{5.0, -5.0}, {5.0, 5.0}});
    ExtendCommand cmd(targetId, boundaryId, Point2d{3.0, 0.0});
    OH_CHECK(cmd.Execute(doc));
    cmd.Undo(doc);
    cmd.Redo(doc);
    const auto* line = std::get_if<Line2d>(&doc.FindEntity(targetId)->shape);
    OH_CHECK(PointNearlyEqual(line->start, Point2d{0.0, 0.0}));
    OH_CHECK(PointNearlyEqual(line->end, Point2d{5.0, 0.0}));
}

int main() {
    TestExtendClickNearEndExtendsEnd();
    TestExtendClickNearStartExtendsStart();
    TestExtendFailsWhenBoundaryTooShort();
    TestExtendParallelLinesReturnsNullopt();
    TestExtendCommandExecuteLengthensTarget();
    TestExtendCommandExecuteFailsWhenBoundaryTooShort();
    TestExtendCommandUndoRestoresExactPriorShape();
    TestExtendCommandRedoReappliesExactPostShape();
    std::puts("ExtendTests: all tests passed.");
    return 0;
}
