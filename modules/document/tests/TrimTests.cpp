#include <openhouse/document/Trim.hpp>
#include <openhouse/document/TrimCommand.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>
#include <cmath>

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

static void TestTrimNormalIntersectionKeepsFarSide() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {10.0, 0.0}});
    const EntityId cutterId = doc.Add(Line2d{{5.0, -5.0}, {5.0, 5.0}});
    const auto trimmed = ComputeTrim(doc, targetId, cutterId, Point2d{1.0, 0.0});
    OH_CHECK(trimmed.has_value());
    OH_CHECK(PointNearlyEqual(trimmed->start, Point2d{5.0, 0.0}));
    OH_CHECK(PointNearlyEqual(trimmed->end, Point2d{10.0, 0.0}));
}

static void TestTrimClickNearEndBKeepsNearSideA() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {10.0, 0.0}});
    const EntityId cutterId = doc.Add(Line2d{{5.0, -5.0}, {5.0, 5.0}});
    const auto trimmed = ComputeTrim(doc, targetId, cutterId, Point2d{9.0, 0.0});
    OH_CHECK(trimmed.has_value());
    OH_CHECK(PointNearlyEqual(trimmed->start, Point2d{0.0, 0.0}));
    OH_CHECK(PointNearlyEqual(trimmed->end, Point2d{5.0, 0.0}));
}

static void TestTrimClickAtEndAKeepsFarSide() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {10.0, 0.0}});
    const EntityId cutterId = doc.Add(Line2d{{5.0, -5.0}, {5.0, 5.0}});
    const auto trimmed = ComputeTrim(doc, targetId, cutterId, Point2d{0.0, 0.0});
    OH_CHECK(trimmed.has_value());
    OH_CHECK(PointNearlyEqual(trimmed->start, Point2d{5.0, 0.0}));
    OH_CHECK(PointNearlyEqual(trimmed->end, Point2d{10.0, 0.0}));
}

static void TestTrimClickAtEndBKeepsNearSideA() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {10.0, 0.0}});
    const EntityId cutterId = doc.Add(Line2d{{5.0, -5.0}, {5.0, 5.0}});
    const auto trimmed = ComputeTrim(doc, targetId, cutterId, Point2d{10.0, 0.0});
    OH_CHECK(trimmed.has_value());
    OH_CHECK(PointNearlyEqual(trimmed->start, Point2d{0.0, 0.0}));
    OH_CHECK(PointNearlyEqual(trimmed->end, Point2d{5.0, 0.0}));
}

static void TestTrimNoIntersectionReturnsNullopt() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {10.0, 0.0}});
    const EntityId cutterId = doc.Add(Line2d{{100.0, -5.0}, {100.0, 5.0}});
    const auto trimmed = ComputeTrim(doc, targetId, cutterId, Point2d{1.0, 0.0});
    OH_CHECK(!trimmed.has_value());
}

static void TestTrimIntersectionOutsideCutterSegmentReturnsNullopt() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {10.0, 0.0}});
    const EntityId cutterId = doc.Add(Line2d{{5.0, 1.0}, {5.0, 5.0}});
    const auto trimmed = ComputeTrim(doc, targetId, cutterId, Point2d{1.0, 0.0});
    OH_CHECK(!trimmed.has_value());
}

static void TestTrimParallelLinesReturnsNullopt() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {10.0, 0.0}});
    const EntityId cutterId = doc.Add(Line2d{{0.0, 5.0}, {10.0, 5.0}});
    const auto trimmed = ComputeTrim(doc, targetId, cutterId, Point2d{1.0, 0.0});
    OH_CHECK(!trimmed.has_value());
}

static void TestTrimCommandExecuteShortensTarget() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {10.0, 0.0}});
    const EntityId cutterId = doc.Add(Line2d{{5.0, -5.0}, {5.0, 5.0}});
    TrimCommand cmd(targetId, cutterId, Point2d{1.0, 0.0});
    OH_CHECK(cmd.Execute(doc));
    const auto* line = std::get_if<Line2d>(&doc.FindEntity(targetId)->shape);
    OH_CHECK(line != nullptr);
    OH_CHECK(PointNearlyEqual(line->start, Point2d{5.0, 0.0}));
    OH_CHECK(PointNearlyEqual(line->end, Point2d{10.0, 0.0}));
}

static void TestTrimCommandExecuteFailsWhenNoIntersection() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {10.0, 0.0}});
    const EntityId cutterId = doc.Add(Line2d{{100.0, -5.0}, {100.0, 5.0}});
    TrimCommand cmd(targetId, cutterId, Point2d{1.0, 0.0});
    OH_CHECK(!cmd.Execute(doc));
    const auto* line = std::get_if<Line2d>(&doc.FindEntity(targetId)->shape);
    OH_CHECK(PointNearlyEqual(line->start, Point2d{0.0, 0.0}));
    OH_CHECK(PointNearlyEqual(line->end, Point2d{10.0, 0.0}));
}

static void TestTrimCommandUndoRestoresExactPriorShape() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {10.0, 0.0}});
    const EntityId cutterId = doc.Add(Line2d{{5.0, -5.0}, {5.0, 5.0}});
    TrimCommand cmd(targetId, cutterId, Point2d{1.0, 0.0});
    OH_CHECK(cmd.Execute(doc));
    cmd.Undo(doc);
    const auto* line = std::get_if<Line2d>(&doc.FindEntity(targetId)->shape);
    OH_CHECK(PointNearlyEqual(line->start, Point2d{0.0, 0.0}));
    OH_CHECK(PointNearlyEqual(line->end, Point2d{10.0, 0.0}));
}

static void TestTrimCommandRedoReappliesExactPostShape() {
    Document doc;
    const EntityId targetId = doc.Add(Line2d{{0.0, 0.0}, {10.0, 0.0}});
    const EntityId cutterId = doc.Add(Line2d{{5.0, -5.0}, {5.0, 5.0}});
    TrimCommand cmd(targetId, cutterId, Point2d{1.0, 0.0});
    OH_CHECK(cmd.Execute(doc));
    cmd.Undo(doc);
    cmd.Redo(doc);
    const auto* line = std::get_if<Line2d>(&doc.FindEntity(targetId)->shape);
    OH_CHECK(PointNearlyEqual(line->start, Point2d{5.0, 0.0}));
    OH_CHECK(PointNearlyEqual(line->end, Point2d{10.0, 0.0}));
}

int main() {
    TestTrimNormalIntersectionKeepsFarSide();
    TestTrimClickNearEndBKeepsNearSideA();
    TestTrimClickAtEndAKeepsFarSide();
    TestTrimClickAtEndBKeepsNearSideA();
    TestTrimNoIntersectionReturnsNullopt();
    TestTrimIntersectionOutsideCutterSegmentReturnsNullopt();
    TestTrimParallelLinesReturnsNullopt();
    TestTrimCommandExecuteShortensTarget();
    TestTrimCommandExecuteFailsWhenNoIntersection();
    TestTrimCommandUndoRestoresExactPriorShape();
    TestTrimCommandRedoReappliesExactPostShape();
    std::puts("TrimTests: all tests passed.");
    return 0;
}
