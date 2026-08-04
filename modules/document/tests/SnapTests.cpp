#include <openhouse/document/Snap.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>

using namespace openhouse::document;
using namespace openhouse::geometry;

namespace {
constexpr double kPi = 3.14159265358979323846;

bool NearlyEqual(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}
} // namespace

static void TestSnapFindsLineEndpoint() {
    Document doc;
    const EntityId id = doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}});

    const auto result = FindSnapPoint(doc, Point2d{0.05, 0.0}, 0.5);
    OH_CHECK(result.has_value());
    OH_CHECK(result->id == id);
    OH_CHECK(result->type == SnapType::Endpoint);
    OH_CHECK(NearlyEqual(result->point.x, 0.0));
    OH_CHECK(NearlyEqual(result->point.y, 0.0));
}

static void TestSnapFindsLineMidpoint() {
    Document doc;
    const EntityId id = doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}});

    // Query near the midpoint (5,0), far enough from either endpoint
    // that only the midpoint candidate is within tolerance.
    const auto result = FindSnapPoint(doc, Point2d{5.05, 0.0}, 0.5);
    OH_CHECK(result.has_value());
    OH_CHECK(result->id == id);
    OH_CHECK(result->type == SnapType::Midpoint);
    OH_CHECK(NearlyEqual(result->point.x, 5.0));
}

static void TestSnapFindsCircleCenter() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{3.0, 4.0}, 10.0});

    const auto result = FindSnapPoint(doc, Point2d{3.05, 4.0}, 0.5);
    OH_CHECK(result.has_value());
    OH_CHECK(result->id == id);
    OH_CHECK(result->type == SnapType::Center);
    OH_CHECK(NearlyEqual(result->point.x, 3.0));
    OH_CHECK(NearlyEqual(result->point.y, 4.0));
}

static void TestSnapFindsArcEndpointsMidpointAndCenter() {
    // Quarter arc, center (0,0), radius 10: start at 0 rad (10,0), end
    // at pi/2 (0,10), midpoint at pi/4 (~7.07,~7.07).
    Document doc;
    const EntityId id = doc.Add(Arc2d{Point2d{0.0, 0.0}, 10.0, 0.0, kPi / 2.0});

    const auto startResult = FindSnapPoint(doc, Point2d{10.05, 0.0}, 0.5);
    OH_CHECK(startResult.has_value());
    OH_CHECK(startResult->id == id);
    OH_CHECK(startResult->type == SnapType::Endpoint);

    const auto endResult = FindSnapPoint(doc, Point2d{0.0, 10.05}, 0.5);
    OH_CHECK(endResult.has_value());
    OH_CHECK(endResult->type == SnapType::Endpoint);

    const auto midResult = FindSnapPoint(doc, Point2d{7.071, 7.071}, 0.5);
    OH_CHECK(midResult.has_value());
    OH_CHECK(midResult->type == SnapType::Midpoint);

    const auto centerResult = FindSnapPoint(doc, Point2d{0.05, 0.0}, 0.5);
    OH_CHECK(centerResult.has_value());
    OH_CHECK(centerResult->type == SnapType::Center);
}

static void TestSnapReturnsClosestCandidateAmongMultipleOnSameEntity() {
    // A single Line2d has 3 candidates (2 endpoints + midpoint); query
    // point closer to the (10,0) endpoint than to the midpoint (5,0)
    // or the (0,0) endpoint must return that endpoint specifically.
    Document doc;
    const EntityId id = doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}});

    const auto result = FindSnapPoint(doc, Point2d{9.9, 0.0}, 5.0);
    OH_CHECK(result.has_value());
    OH_CHECK(result->id == id);
    OH_CHECK(result->type == SnapType::Endpoint);
    OH_CHECK(NearlyEqual(result->point.x, 10.0));
}

static void TestSnapReturnsClosestCandidateAcrossDifferentEntities() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}); // center at origin, far edge
    const EntityId closer = doc.Add(Line2d{Point2d{1.0, 1.0}, Point2d{1.0, 3.0}});

    // Query point is closer to the Line's (1,1) endpoint than to the
    // Circle's center at the origin.
    const auto result = FindSnapPoint(doc, Point2d{1.1, 1.1}, 5.0);
    OH_CHECK(result.has_value());
    OH_CHECK(result->id == closer);
    OH_CHECK(result->type == SnapType::Endpoint);
}

static void TestSnapReturnsNulloptWhenNothingWithinTolerance() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0});

    const auto result = FindSnapPoint(doc, Point2d{100.0, 100.0}, 1.0);
    OH_CHECK(!result.has_value());
}

static void TestSnapOnEmptyDocumentReturnsNullopt() {
    const Document doc;
    const auto result = FindSnapPoint(doc, Point2d{0.0, 0.0}, 1.0);
    OH_CHECK(!result.has_value());
}

static void TestSnapSkipsHiddenLayer() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0}, "Hidden");
    doc.FindLayer("Hidden")->SetVisible(false);

    const auto result = FindSnapPoint(doc, Point2d{0.05, 0.0}, 0.5);
    OH_CHECK(!result.has_value());
}

static void TestSnapDoesNOTSkipLockedLayer() {
    // Same reasoning as document::HitTest's own equivalent test: Locked
    // means "cannot be edited," not "cannot be queried."
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0}, "Locked");
    doc.FindLayer("Locked")->SetLocked(true);

    const auto result = FindSnapPoint(doc, Point2d{0.05, 0.0}, 0.5);
    OH_CHECK(result.has_value());
    OH_CHECK(result->id == id);
}

static void TestSnapFindsArcCenterFarOutsideArcsOwnBoundingBox() {
    // Regression guard for a specific design decision (see Snap.hpp's
    // own comment on why no AABB-dilate fast-reject is used): a short
    // arc segment far around its circle from its own center means the
    // arc's Bounds() (the curve's extent) does NOT contain the center
    // at all. If a naive AABB-based fast-reject (like HitTest's) were
    // ever added here, this exact case would silently break -- a query
    // point sitting exactly on the center would be wrongly rejected
    // before the Center candidate was even considered, because the
    // center lies nowhere near the swept curve's own bounding box.
    Document doc;
    // Center at the origin, radius 1000, but only a 0.01-radian sliver
    // of the circle is swept -- the curve itself sits far out near
    // (1000, ~0), nowhere near the origin.
    const EntityId id = doc.Add(Arc2d{Point2d{0.0, 0.0}, 1000.0, 0.0, 0.01});

    const auto result = FindSnapPoint(doc, Point2d{0.0, 0.0}, 1.0);
    OH_CHECK(result.has_value());
    OH_CHECK(result->id == id);
    OH_CHECK(result->type == SnapType::Center);
    OH_CHECK(NearlyEqual(result->distance, 0.0));
}

static void TestSnapWithZeroToleranceRequiresExactMatch() {
    Document doc;
    const EntityId id = doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}});

    const auto exact = FindSnapPoint(doc, Point2d{0.0, 0.0}, 0.0);
    OH_CHECK(exact.has_value());
    OH_CHECK(exact->id == id);

    const auto near = FindSnapPoint(doc, Point2d{0.0001, 0.0}, 0.0);
    OH_CHECK(!near.has_value());
}

int main() {
    TestSnapFindsLineEndpoint();
    TestSnapFindsLineMidpoint();
    TestSnapFindsCircleCenter();
    TestSnapFindsArcEndpointsMidpointAndCenter();
    TestSnapReturnsClosestCandidateAmongMultipleOnSameEntity();
    TestSnapReturnsClosestCandidateAcrossDifferentEntities();
    TestSnapReturnsNulloptWhenNothingWithinTolerance();
    TestSnapOnEmptyDocumentReturnsNullopt();
    TestSnapSkipsHiddenLayer();
    TestSnapDoesNOTSkipLockedLayer();
    TestSnapFindsArcCenterFarOutsideArcsOwnBoundingBox();
    TestSnapWithZeroToleranceRequiresExactMatch();

    std::puts("SnapTests: all tests passed.");
    return 0;
}
