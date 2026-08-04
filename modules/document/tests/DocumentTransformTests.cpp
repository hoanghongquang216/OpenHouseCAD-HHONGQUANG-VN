#include <openhouse/document/Transform.hpp>
#include <openhouse/testing/Check.hpp>

#include <cmath>
#include <cstdio>
#include <variant>

using namespace openhouse::document;
using namespace openhouse::geometry;

namespace {
constexpr double kPi = 3.14159265358979323846;
} // namespace

static void TestTranslateEntitySucceedsAndMovesShape() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0});

    OH_CHECK(TranslateEntity(doc, id, Vector2d{10.0, 10.0}));

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle != nullptr);
    OH_CHECK(circle->center.x == 10.0 && circle->center.y == 10.0);
    OH_CHECK(circle->radius == 5.0);
}

static void TestTranslateEntityWithUnknownIdReturnsFalse() {
    Document doc;
    OH_CHECK(!TranslateEntity(doc, 99999, Vector2d{1.0, 1.0}));
}

static void TestTranslateEntityOnLockedLayerIsRejected() {
    // The core behavior locked in during Spiral 4's design review:
    // Locked blocks Transform (but not Selection/HitTest -- see
    // SEL-002's tests for those). The shape must be verifiably
    // UNCHANGED, not just "the call returned false".
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{5.0, 5.0}, 3.0}, "Locked");
    doc.FindLayer("Locked")->SetLocked(true);

    OH_CHECK(!TranslateEntity(doc, id, Vector2d{100.0, 100.0}));

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->center.x == 5.0 && circle->center.y == 5.0);
}

static void TestTranslateEntityOnHiddenLayerIsRejected() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Hidden");
    doc.FindLayer("Hidden")->SetVisible(false);

    OH_CHECK(!TranslateEntity(doc, id, Vector2d{1.0, 1.0}));
}

static void TestTranslateEntityOnUnlockedVisibleLayerSucceeds() {
    // Explicit positive control alongside the locked/hidden negative
    // cases -- a layer that is neither locked nor hidden must not be
    // accidentally rejected by an overly broad condition.
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Normal");
    OH_CHECK(TranslateEntity(doc, id, Vector2d{1.0, 1.0}));
}

static void TestTranslateSelectionMovesAllEligibleEntities() {
    Document doc;
    const EntityId a = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Free");
    const EntityId b = doc.Add(Circle2d{Point2d{1.0, 1.0}, 1.0}, "Free");

    SelectionSet sel;
    OH_CHECK(sel.Select(a));
    OH_CHECK(sel.Select(b));

    OH_CHECK(TranslateSelection(doc, sel, Vector2d{10.0, 10.0}) == 2);

    const auto* circleA = std::get_if<Circle2d>(&doc.FindEntity(a)->shape);
    const auto* circleB = std::get_if<Circle2d>(&doc.FindEntity(b)->shape);
    OH_CHECK(circleA->center.x == 10.0 && circleA->center.y == 10.0);
    OH_CHECK(circleB->center.x == 11.0 && circleB->center.y == 11.0);
}

static void TestTranslateSelectionReturnsPartialCountWhenSomeLocked() {
    Document doc;
    const EntityId a = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Free");
    const EntityId b = doc.Add(Circle2d{Point2d{1.0, 1.0}, 1.0}, "Locked");
    const EntityId c = doc.Add(Circle2d{Point2d{2.0, 2.0}, 1.0}, "Free");
    doc.FindLayer("Locked")->SetLocked(true);

    SelectionSet sel;
    OH_CHECK(sel.Select(a));
    OH_CHECK(sel.Select(b));
    OH_CHECK(sel.Select(c));

    OH_CHECK(TranslateSelection(doc, sel, Vector2d{50.0, 50.0}) == 2); // b excluded

    const auto* circleB = std::get_if<Circle2d>(&doc.FindEntity(b)->shape);
    OH_CHECK(circleB->center.x == 1.0 && circleB->center.y == 1.0); // unchanged
}

static void TestTranslateSelectionOnEmptySelectionReturnsZero() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    const SelectionSet empty;
    OH_CHECK(TranslateSelection(doc, empty, Vector2d{1.0, 1.0}) == 0);
}

static void TestTranslateSelectionWithStaleIdDoesNotCrash() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    SelectionSet sel;
    OH_CHECK(sel.Select(99999)); // no entity has this ID
    OH_CHECK(TranslateSelection(doc, sel, Vector2d{1.0, 1.0}) == 0);
}

// --- RotateEntity / RotateSelection -----------------------------------

static void TestRotateEntitySucceedsAndRotatesShape() {
    Document doc;
    const EntityId id = doc.Add(Line2d{Point2d{10.0, 0.0}, Point2d{20.0, 0.0}});

    OH_CHECK(RotateEntity(doc, id, kPi / 2.0, Point2d{0.0, 0.0}));

    const auto* line = std::get_if<Line2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(line != nullptr);
    OH_CHECK(std::abs(line->start.x - 0.0) < 1e-9);
    OH_CHECK(std::abs(line->start.y - 10.0) < 1e-9);
}

static void TestRotateEntityWithUnknownIdReturnsFalse() {
    Document doc;
    OH_CHECK(!RotateEntity(doc, 99999, kPi / 2.0, Point2d{0.0, 0.0}));
}

static void TestRotateEntityOnLockedLayerIsRejected() {
    // Same core behavior as TranslateEntity's locked-layer test: the
    // shape must be verifiably UNCHANGED after a rejected rotate, not
    // just "the call returned false".
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{5.0, 5.0}, 3.0}, "Locked");
    doc.FindLayer("Locked")->SetLocked(true);

    OH_CHECK(!RotateEntity(doc, id, kPi, Point2d{0.0, 0.0}));

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->center.x == 5.0 && circle->center.y == 5.0);
}

static void TestRotateEntityOnHiddenLayerIsRejected() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Hidden");
    doc.FindLayer("Hidden")->SetVisible(false);

    OH_CHECK(!RotateEntity(doc, id, kPi / 2.0, Point2d{0.0, 0.0}));
}

static void TestRotateSelectionRotatesAllEligibleEntities() {
    Document doc;
    const EntityId a = doc.Add(Circle2d{Point2d{10.0, 0.0}, 1.0}, "Free");
    const EntityId b = doc.Add(Circle2d{Point2d{0.0, 10.0}, 1.0}, "Free");

    SelectionSet sel;
    OH_CHECK(sel.Select(a));
    OH_CHECK(sel.Select(b));

    OH_CHECK(RotateSelection(doc, sel, kPi / 2.0, Point2d{0.0, 0.0}) == 2);

    const auto* circleA = std::get_if<Circle2d>(&doc.FindEntity(a)->shape);
    OH_CHECK(std::abs(circleA->center.x - 0.0) < 1e-9);
    OH_CHECK(std::abs(circleA->center.y - 10.0) < 1e-9);
}

// Explicitly requested during TRF-002's design review: a selection of
// 10 entities, 3 on a locked layer, must report exactly 7 successful
// rotations -- confirming std::size_t's partial-success counting
// behavior at a larger, more realistic scale than the 3-entity case
// already covered for Translate.
static void TestRotateSelectionWithTenEntitiesThreeLockedReturnsSeven() {
    Document doc;
    SelectionSet sel;

    for (int i = 0; i < 7; ++i) {
        const EntityId id =
            doc.Add(Circle2d{Point2d{static_cast<double>(i), 0.0}, 1.0}, "Free");
        OH_CHECK(sel.Select(id));
    }
    Layer& locked = doc.CreateLayer("Locked");
    locked.SetLocked(true);
    for (int i = 0; i < 3; ++i) {
        const EntityId id =
            doc.Add(Circle2d{Point2d{100.0 + static_cast<double>(i), 0.0}, 1.0}, "Locked");
        OH_CHECK(sel.Select(id));
    }

    OH_CHECK(sel.Count() == 10);
    OH_CHECK(RotateSelection(doc, sel, kPi / 4.0, Point2d{0.0, 0.0}) == 7);
}

// --- ScaleEntity / ScaleSelection --------------------------------------

static void TestScaleEntitySucceedsAndScalesShape() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{10.0, 10.0}, 5.0});

    OH_CHECK(ScaleEntity(doc, id, 2.0, Point2d{0.0, 0.0}));

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->center.x == 20.0 && circle->center.y == 20.0);
    OH_CHECK(circle->radius == 10.0);
}

static void TestScaleEntityWithUnknownIdReturnsFalse() {
    Document doc;
    OH_CHECK(!ScaleEntity(doc, 99999, 2.0, Point2d{0.0, 0.0}));
}

// The core behavior locked in during TRF-003's design review: factor
// <= 0 is rejected outright, before ever touching the shape -- neither
// zero (collapses geometry to a point, an unrecoverable loss) nor
// negative (a reflection, unsupported until its own future Spiral) is
// allowed. Both must leave the shape verifiably UNCHANGED, matching the
// same "reject clean, don't half-apply" pattern as the locked-layer
// tests.
static void TestScaleEntityWithZeroFactorIsRejected() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{5.0, 5.0}, 3.0});

    OH_CHECK(!ScaleEntity(doc, id, 0.0, Point2d{0.0, 0.0}));

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->radius == 3.0); // unchanged
}

static void TestScaleEntityWithNegativeFactorIsRejected() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{5.0, 5.0}, 3.0});

    OH_CHECK(!ScaleEntity(doc, id, -2.0, Point2d{0.0, 0.0}));

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->radius == 3.0); // unchanged
}

static void TestScaleEntityOnLockedLayerIsRejected() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{5.0, 5.0}, 3.0}, "Locked");
    doc.FindLayer("Locked")->SetLocked(true);

    OH_CHECK(!ScaleEntity(doc, id, 2.0, Point2d{0.0, 0.0}));

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->center.x == 5.0 && circle->center.y == 5.0 && circle->radius == 3.0);
}

static void TestScaleEntityOnHiddenLayerIsRejected() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Hidden");
    doc.FindLayer("Hidden")->SetVisible(false);

    OH_CHECK(!ScaleEntity(doc, id, 2.0, Point2d{0.0, 0.0}));
}

static void TestScaleSelectionScalesAllEligibleEntities() {
    Document doc;
    const EntityId a = doc.Add(Circle2d{Point2d{10.0, 0.0}, 1.0}, "Free");
    const EntityId b = doc.Add(Circle2d{Point2d{0.0, 10.0}, 2.0}, "Free");

    SelectionSet sel;
    OH_CHECK(sel.Select(a));
    OH_CHECK(sel.Select(b));

    OH_CHECK(ScaleSelection(doc, sel, 3.0, Point2d{0.0, 0.0}) == 2);

    const auto* circleA = std::get_if<Circle2d>(&doc.FindEntity(a)->shape);
    const auto* circleB = std::get_if<Circle2d>(&doc.FindEntity(b)->shape);
    OH_CHECK(circleA->radius == 3.0);
    OH_CHECK(circleB->radius == 6.0);
}

static void TestScaleSelectionWithInvalidFactorScalesNothing() {
    // A zero/negative factor rejects EVERY entity in the selection, not
    // just the first one hit -- ScaleSelection must return 0, not
    // silently succeed on some subset.
    Document doc;
    const EntityId a = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    const EntityId b = doc.Add(Circle2d{Point2d{1.0, 1.0}, 2.0});

    SelectionSet sel;
    OH_CHECK(sel.Select(a));
    OH_CHECK(sel.Select(b));

    OH_CHECK(ScaleSelection(doc, sel, 0.0, Point2d{0.0, 0.0}) == 0);
    OH_CHECK(ScaleSelection(doc, sel, -1.0, Point2d{0.0, 0.0}) == 0);

    const auto* circleA = std::get_if<Circle2d>(&doc.FindEntity(a)->shape);
    OH_CHECK(circleA->radius == 1.0); // never touched
}

int main() {
    TestTranslateEntitySucceedsAndMovesShape();
    TestTranslateEntityWithUnknownIdReturnsFalse();
    TestTranslateEntityOnLockedLayerIsRejected();
    TestTranslateEntityOnHiddenLayerIsRejected();
    TestTranslateEntityOnUnlockedVisibleLayerSucceeds();
    TestTranslateSelectionMovesAllEligibleEntities();
    TestTranslateSelectionReturnsPartialCountWhenSomeLocked();
    TestTranslateSelectionOnEmptySelectionReturnsZero();
    TestTranslateSelectionWithStaleIdDoesNotCrash();

    TestRotateEntitySucceedsAndRotatesShape();
    TestRotateEntityWithUnknownIdReturnsFalse();
    TestRotateEntityOnLockedLayerIsRejected();
    TestRotateEntityOnHiddenLayerIsRejected();
    TestRotateSelectionRotatesAllEligibleEntities();
    TestRotateSelectionWithTenEntitiesThreeLockedReturnsSeven();

    TestScaleEntitySucceedsAndScalesShape();
    TestScaleEntityWithUnknownIdReturnsFalse();
    TestScaleEntityWithZeroFactorIsRejected();
    TestScaleEntityWithNegativeFactorIsRejected();
    TestScaleEntityOnLockedLayerIsRejected();
    TestScaleEntityOnHiddenLayerIsRejected();
    TestScaleSelectionScalesAllEligibleEntities();
    TestScaleSelectionWithInvalidFactorScalesNothing();

    std::puts("DocumentTransformTests: all tests passed.");
    return 0;
}
