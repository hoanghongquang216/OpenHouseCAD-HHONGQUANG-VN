#include <openhouse/document/Transform.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>
#include <variant>

using namespace openhouse::document;
using namespace openhouse::geometry;

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

    std::puts("DocumentTransformTests: all tests passed.");
    return 0;
}
