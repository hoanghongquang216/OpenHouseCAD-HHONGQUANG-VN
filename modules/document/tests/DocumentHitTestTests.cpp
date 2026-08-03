#include <openhouse/document/HitTest.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>

using namespace openhouse::document;
using namespace openhouse::geometry;

static void TestHitTestFindsSingleEntity() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0});

    const auto result = HitTest(doc, Point2d{5.0, 0.0}, 0.1);
    OH_CHECK(result.has_value());
    OH_CHECK(result->id == id);
    OH_CHECK(result->distance < 0.1);
}

static void TestHitTestReturnsNulloptWhenNothingWithinTolerance() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0});

    const auto result = HitTest(doc, Point2d{100.0, 100.0}, 1.0);
    OH_CHECK(!result.has_value());
}

static void TestHitTestReturnsClosestOfOverlappingEntities() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 10.0}); // far edge from query point
    const EntityId closer = doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0}); // near edge

    // Query point sits between the two circle outlines, closer to the
    // radius=5 one (distance 1) than the radius=10 one (distance 4).
    const auto result = HitTest(doc, Point2d{6.0, 0.0}, 10.0);
    OH_CHECK(result.has_value());
    OH_CHECK(result->id == closer);
}

static void TestHitTestSkipsHiddenLayer() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0}, "Hidden");
    doc.FindLayer("Hidden")->SetVisible(false);

    const auto result = HitTest(doc, Point2d{5.0, 0.0}, 0.1);
    OH_CHECK(!result.has_value());
}

static void TestHitTestDoesNOTSkipLockedLayer() {
    // The key behavioral decision from SEL-002's design review: Locked
    // means "cannot be edited," not "cannot be selected." An entity on
    // a locked-but-visible layer must still be hit-testable.
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0}, "Locked");
    doc.FindLayer("Locked")->SetLocked(true);
    // Deliberately NOT calling SetVisible(false) -- locked, still visible.

    const auto result = HitTest(doc, Point2d{5.0, 0.0}, 0.1);
    OH_CHECK(result.has_value());
    OH_CHECK(result->id == id);
}

static void TestHitTestOnEmptyDocumentReturnsNullopt() {
    const Document doc;
    const auto result = HitTest(doc, Point2d{0.0, 0.0}, 1.0);
    OH_CHECK(!result.has_value());
}

static void TestHitTestWithZeroToleranceRequiresExactHit() {
    Document doc;
    const EntityId id = doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}});

    // Point exactly on the line -> must match even with tolerance 0.
    const auto exact = HitTest(doc, Point2d{5.0, 0.0}, 0.0);
    OH_CHECK(exact.has_value());
    OH_CHECK(exact->id == id);

    // Point a tiny amount off the line -> must NOT match with tolerance 0.
    const auto near = HitTest(doc, Point2d{5.0, 0.0001}, 0.0);
    OH_CHECK(!near.has_value());
}

int main() {
    TestHitTestFindsSingleEntity();
    TestHitTestReturnsNulloptWhenNothingWithinTolerance();
    TestHitTestReturnsClosestOfOverlappingEntities();
    TestHitTestSkipsHiddenLayer();
    TestHitTestDoesNOTSkipLockedLayer();
    TestHitTestOnEmptyDocumentReturnsNullopt();
    TestHitTestWithZeroToleranceRequiresExactHit();

    std::puts("DocumentHitTestTests: all tests passed.");
    return 0;
}
