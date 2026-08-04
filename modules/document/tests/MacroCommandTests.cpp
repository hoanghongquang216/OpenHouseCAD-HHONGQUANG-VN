#include <openhouse/document/MacroCommand.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>
#include <memory>
#include <variant>

using namespace openhouse::document;
using namespace openhouse::geometry;

static void TestMacroCommandEmptyExecuteReturnsFalse() {
    Document doc;
    MacroCommand macro;
    OH_CHECK(!macro.Execute(doc));
    OH_CHECK(macro.SuccessCount() == 0);
    OH_CHECK(macro.TotalCount() == 0);
}

static void TestMacroCommandAllSucceedExecutesEveryChild() {
    Document doc;
    const EntityId a = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    const EntityId b = doc.Add(Circle2d{Point2d{10.0, 10.0}, 2.0});
    const EntityId c = doc.Add(Circle2d{Point2d{20.0, 20.0}, 3.0});

    MacroCommand macro;
    macro.Add(std::make_unique<ScaleCommand>(a, 2.0, Point2d{0.0, 0.0}));
    macro.Add(std::make_unique<ScaleCommand>(b, 2.0, Point2d{0.0, 0.0}));
    macro.Add(std::make_unique<ScaleCommand>(c, 2.0, Point2d{0.0, 0.0}));

    OH_CHECK(macro.Execute(doc));
    OH_CHECK(macro.SuccessCount() == 3);
    OH_CHECK(macro.TotalCount() == 3);

    const auto* circleA = std::get_if<Circle2d>(&doc.FindEntity(a)->shape);
    const auto* circleB = std::get_if<Circle2d>(&doc.FindEntity(b)->shape);
    const auto* circleC = std::get_if<Circle2d>(&doc.FindEntity(c)->shape);
    OH_CHECK(circleA->radius == 2.0);
    OH_CHECK(circleB->radius == 4.0);
    OH_CHECK(circleC->radius == 6.0);
}

// The primary scenario explicitly requested for this Spiral's testing:
// undoing a MacroCommand reverses ALL of its sub-commands' effects in
// one call, exactly the "one undo step for a multi-entity operation"
// behavior the whole class exists for.
static void TestMacroCommandUndoRestoresAllEntitiesInOneCall() {
    Document doc;
    const EntityId a = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    const EntityId b = doc.Add(Circle2d{Point2d{10.0, 10.0}, 2.0});
    const EntityId c = doc.Add(Circle2d{Point2d{20.0, 20.0}, 3.0});

    MacroCommand macro;
    macro.Add(std::make_unique<ScaleCommand>(a, 2.0, Point2d{0.0, 0.0}));
    macro.Add(std::make_unique<ScaleCommand>(b, 2.0, Point2d{0.0, 0.0}));
    macro.Add(std::make_unique<ScaleCommand>(c, 2.0, Point2d{0.0, 0.0}));

    OH_CHECK(macro.Execute(doc));
    macro.Undo(doc);

    const auto* circleA = std::get_if<Circle2d>(&doc.FindEntity(a)->shape);
    const auto* circleB = std::get_if<Circle2d>(&doc.FindEntity(b)->shape);
    const auto* circleC = std::get_if<Circle2d>(&doc.FindEntity(c)->shape);
    OH_CHECK(circleA->radius == 1.0);
    OH_CHECK(circleB->radius == 2.0);
    OH_CHECK(circleC->radius == 3.0);
}

static void TestMacroCommandRedoReappliesAllEntities() {
    Document doc;
    const EntityId a = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    const EntityId b = doc.Add(Circle2d{Point2d{10.0, 10.0}, 2.0});

    MacroCommand macro;
    macro.Add(std::make_unique<ScaleCommand>(a, 2.0, Point2d{0.0, 0.0}));
    macro.Add(std::make_unique<ScaleCommand>(b, 2.0, Point2d{0.0, 0.0}));

    OH_CHECK(macro.Execute(doc));
    macro.Undo(doc);
    macro.Redo(doc);

    const auto* circleA = std::get_if<Circle2d>(&doc.FindEntity(a)->shape);
    const auto* circleB = std::get_if<Circle2d>(&doc.FindEntity(b)->shape);
    OH_CHECK(circleA->radius == 2.0);
    OH_CHECK(circleB->radius == 4.0);
}

// The other essential scenario: a selection with a MIX of eligible and
// locked entities must partially succeed, reporting exactly how many --
// preserving Spiral 4's std::size_t partial-success information rather
// than collapsing it into a single bool.
static void TestMacroCommandWithOneLockedEntityPartiallySucceeds() {
    Document doc;
    const EntityId x = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Free");
    const EntityId y = doc.Add(Circle2d{Point2d{1.0, 1.0}, 2.0}, "Locked");
    const EntityId z = doc.Add(Circle2d{Point2d{2.0, 2.0}, 3.0}, "Free");
    doc.FindLayer("Locked")->SetLocked(true);

    MacroCommand macro;
    macro.Add(std::make_unique<ScaleCommand>(x, 2.0, Point2d{0.0, 0.0}));
    macro.Add(std::make_unique<ScaleCommand>(y, 2.0, Point2d{0.0, 0.0})); // rejected
    macro.Add(std::make_unique<ScaleCommand>(z, 2.0, Point2d{0.0, 0.0}));

    OH_CHECK(macro.Execute(doc));
    OH_CHECK(macro.SuccessCount() == 2);
    OH_CHECK(macro.TotalCount() == 3);

    const auto* circleY = std::get_if<Circle2d>(&doc.FindEntity(y)->shape);
    OH_CHECK(circleY->radius == 2.0); // untouched -- the locked one

    macro.Undo(doc);
    const auto* circleX = std::get_if<Circle2d>(&doc.FindEntity(x)->shape);
    const auto* circleZ = std::get_if<Circle2d>(&doc.FindEntity(z)->shape);
    OH_CHECK(circleX->radius == 1.0);
    OH_CHECK(circleZ->radius == 3.0);
}

static void TestMacroCommandWhereAllChildrenFailReturnsFalse() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Locked");
    doc.FindLayer("Locked")->SetLocked(true);

    MacroCommand macro;
    macro.Add(std::make_unique<ScaleCommand>(id, 2.0, Point2d{0.0, 0.0}));

    OH_CHECK(!macro.Execute(doc));
    OH_CHECK(macro.SuccessCount() == 0);
}

static void TestMacroCommandCanMixDifferentCommandTypes() {
    // A MacroCommand isn't restricted to one operation type -- a
    // "select 3 entities, then Translate one and Scale another" kind of
    // compound action (not yet a real UI flow, but the class itself
    // should not assume homogeneity).
    Document doc;
    const EntityId a = doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0});
    const EntityId b = doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}});

    MacroCommand macro;
    macro.Add(std::make_unique<ScaleCommand>(a, 2.0, Point2d{0.0, 0.0}));
    macro.Add(std::make_unique<TranslateCommand>(b, Vector2d{5.0, 5.0}));

    OH_CHECK(macro.Execute(doc));
    OH_CHECK(macro.SuccessCount() == 2);

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(a)->shape);
    const auto* line = std::get_if<Line2d>(&doc.FindEntity(b)->shape);
    OH_CHECK(circle->radius == 10.0);
    OH_CHECK(line->start.x == 5.0 && line->start.y == 5.0);
}

int main() {
    TestMacroCommandEmptyExecuteReturnsFalse();
    TestMacroCommandAllSucceedExecutesEveryChild();
    TestMacroCommandUndoRestoresAllEntitiesInOneCall();
    TestMacroCommandRedoReappliesAllEntities();
    TestMacroCommandWithOneLockedEntityPartiallySucceeds();
    TestMacroCommandWhereAllChildrenFailReturnsFalse();
    TestMacroCommandCanMixDifferentCommandTypes();

    std::puts("MacroCommandTests: all tests passed.");
    return 0;
}
