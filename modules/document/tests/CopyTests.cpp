// Tests for CopyCommand, added for COPY-001 (Sprint 4). Maps to
// docs/design/COPY-001-Test-Design.md Sections 1 (Functional), 2
// (Undo/Redo), 4 (Error/Edge -- E-001..E-003 only; E-004..E-007 are
// either covered elsewhere or explicitly deferred, see that doc).

#include <openhouse/document/Command.hpp>
#include <openhouse/document/Document.hpp>

#include <openhouse/testing/Check.hpp>
#include <cstdio>
#include <variant>

using namespace openhouse::document;
using namespace openhouse::geometry;

static Line2d MakeLine(double x1, double y1, double x2, double y2) {
    return Line2d{Point2d{x1, y1}, Point2d{x2, y2}};
}

// --- F: Functional Tests ---------------------------------------------

// F-001: Copy Line.
static void TestF001_CopyLine_CreatesTranslatedCopySourceUnchanged() {
    Document doc;
    const EntityId sourceId = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    CopyCommand cmd(sourceId, Vector2d{3.0, 4.0});

    OH_CHECK(cmd.Execute(doc));

    const Entity* copy = doc.FindEntity(cmd.Id());
    OH_CHECK(copy != nullptr);
    const auto* copyLine = std::get_if<Line2d>(&copy->shape);
    OH_CHECK(copyLine != nullptr);
    OH_CHECK(copyLine->start.x == 3.0 && copyLine->start.y == 4.0);
    OH_CHECK(copyLine->end.x == 4.0 && copyLine->end.y == 5.0);

    const Entity* source = doc.FindEntity(sourceId);
    OH_CHECK(source != nullptr);
    const auto* sourceLine = std::get_if<Line2d>(&source->shape);
    OH_CHECK(sourceLine->start.x == 0.0 && sourceLine->start.y == 0.0);
    OH_CHECK(sourceLine->end.x == 1.0 && sourceLine->end.y == 1.0);
}

// F-002: Copy Circle -- radius preserved exactly.
static void TestF002_CopyCircle_RadiusPreservedCenterShifted() {
    Document doc;
    const EntityId sourceId = doc.Add(Circle2d{Point2d{5.0, 5.0}, 2.5});
    CopyCommand cmd(sourceId, Vector2d{1.0, 1.0});

    OH_CHECK(cmd.Execute(doc));

    const Entity* copy = doc.FindEntity(cmd.Id());
    OH_CHECK(copy != nullptr);
    const auto* circle = std::get_if<Circle2d>(&copy->shape);
    OH_CHECK(circle != nullptr);
    OH_CHECK(circle->radius == 2.5);
    OH_CHECK(circle->center.x == 6.0 && circle->center.y == 6.0);
}

// F-003: Copy Arc -- start/end angles and radius preserved exactly.
static void TestF003_CopyArc_AnglesAndRadiusPreservedCenterShifted() {
    Document doc;
    const EntityId sourceId = doc.Add(Arc2d{Point2d{0.0, 0.0}, 3.0, 0.25, 1.5});
    CopyCommand cmd(sourceId, Vector2d{2.0, -2.0});

    OH_CHECK(cmd.Execute(doc));

    const Entity* copy = doc.FindEntity(cmd.Id());
    OH_CHECK(copy != nullptr);
    const auto* arc = std::get_if<Arc2d>(&copy->shape);
    OH_CHECK(arc != nullptr);
    OH_CHECK(arc->radius == 3.0);
    OH_CHECK(arc->startAngle == 0.25);
    OH_CHECK(arc->endAngle == 1.5);
    OH_CHECK(arc->center.x == 2.0 && arc->center.y == -2.0);
}

// F-004: Copy multiple entities (loop of CopyCommands, per Design §1 --
// CopySelection itself is out of scope for this sprint).
static void TestF004_CopyMultipleEntities_AllCreatedIndependently() {
    Document doc;
    const EntityId a = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    const EntityId b = doc.Add(Circle2d{Point2d{5.0, 5.0}, 1.0});
    const EntityId c = doc.Add(Arc2d{Point2d{0.0, 0.0}, 1.0, 0.0, 1.0});

    CopyCommand copyA(a, Vector2d{1.0, 0.0});
    CopyCommand copyB(b, Vector2d{1.0, 0.0});
    CopyCommand copyC(c, Vector2d{1.0, 0.0});
    OH_CHECK(copyA.Execute(doc));
    OH_CHECK(copyB.Execute(doc));
    OH_CHECK(copyC.Execute(doc));

    OH_CHECK(doc.Count() == 6);
    OH_CHECK(doc.FindEntity(copyA.Id()) != nullptr);
    OH_CHECK(doc.FindEntity(copyB.Id()) != nullptr);
    OH_CHECK(doc.FindEntity(copyC.Id()) != nullptr);
}

// F-005: delta = (0, 0) is a valid, non-rejected duplicate-in-place.
static void TestF005_ZeroDelta_CreatesExactDuplicateAtSamePosition() {
    Document doc;
    const EntityId sourceId = doc.Add(MakeLine(2.0, 2.0, 5.0, 5.0));
    CopyCommand cmd(sourceId, Vector2d{0.0, 0.0});

    OH_CHECK(cmd.Execute(doc));

    const Entity* copy = doc.FindEntity(cmd.Id());
    OH_CHECK(copy != nullptr);
    const auto* line = std::get_if<Line2d>(&copy->shape);
    OH_CHECK(line->start.x == 2.0 && line->start.y == 2.0);
    OH_CHECK(line->end.x == 5.0 && line->end.y == 5.0);
}

// F-006: two sequential copies from the same source don't interfere.
static void TestF006_SequentialCopies_IndependentSourceUnaffected() {
    Document doc;
    const EntityId sourceId = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));

    CopyCommand first(sourceId, Vector2d{1.0, 0.0});
    CopyCommand second(sourceId, Vector2d{2.0, 0.0});
    OH_CHECK(first.Execute(doc));
    OH_CHECK(second.Execute(doc));

    OH_CHECK(first.Id() != second.Id());
    const auto* firstLine = std::get_if<Line2d>(&doc.FindEntity(first.Id())->shape);
    const auto* secondLine = std::get_if<Line2d>(&doc.FindEntity(second.Id())->shape);
    OH_CHECK(firstLine->start.x == 1.0);
    OH_CHECK(secondLine->start.x == 2.0);
    const auto* sourceLine = std::get_if<Line2d>(&doc.FindEntity(sourceId)->shape);
    OH_CHECK(sourceLine->start.x == 0.0 && sourceLine->start.y == 0.0);
}

// F-007: copy preserves the source's layer (not the default layer).
static void TestF007_CopyPreservesSourceLayer() {
    Document doc;
    const EntityId sourceId = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0), "Walls");
    CopyCommand cmd(sourceId, Vector2d{1.0, 1.0});

    OH_CHECK(cmd.Execute(doc));

    const Entity* copy = doc.FindEntity(cmd.Id());
    OH_CHECK(copy != nullptr);
    OH_CHECK(copy->layer == "Walls");
}

// --- U: Undo/Redo Tests ------------------------------------------------

// U-001: Execute -> Undo.
static void TestU001_ExecuteThenUndo_RemovesCopyRestoresCount() {
    Document doc;
    const EntityId sourceId = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    CopyCommand cmd(sourceId, Vector2d{1.0, 1.0});
    OH_CHECK(cmd.Execute(doc));
    OH_CHECK(doc.Count() == 2);

    cmd.Undo(doc);

    OH_CHECK(doc.Count() == 1);
    OH_CHECK(doc.FindEntity(cmd.Id()) == nullptr);
    OH_CHECK(doc.FindEntity(sourceId) != nullptr);
}

// U-002: Execute -> Undo -> Redo -- same id, same shape, same layer.
static void TestU002_ExecuteUndoRedo_SameIdShapeLayer() {
    Document doc;
    const EntityId sourceId = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0), "Walls");
    CopyCommand cmd(sourceId, Vector2d{1.0, 1.0});
    OH_CHECK(cmd.Execute(doc));
    const EntityId copyId = cmd.Id();
    cmd.Undo(doc);

    cmd.Redo(doc);

    const Entity* copy = doc.FindEntity(copyId);
    OH_CHECK(copy != nullptr);
    OH_CHECK(copy->id == copyId);
    OH_CHECK(copy->layer == "Walls");
    const auto* line = std::get_if<Line2d>(&copy->shape);
    OH_CHECK(line->start.x == 1.0 && line->start.y == 1.0);
}

// U-003: repeated Execute/Undo/Redo cycles -- no drift, id stable.
static void TestU003_RepeatedUndoRedoCycles_NoDrift() {
    Document doc;
    const EntityId sourceId = doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0});
    CopyCommand cmd(sourceId, Vector2d{1.0, 1.0});
    OH_CHECK(cmd.Execute(doc));
    const EntityId copyId = cmd.Id();

    for (int cycle = 0; cycle < 4; ++cycle) {
        cmd.Undo(doc);
        OH_CHECK(doc.FindEntity(copyId) == nullptr);
        cmd.Redo(doc);
        const Entity* copy = doc.FindEntity(copyId);
        OH_CHECK(copy != nullptr);
        OH_CHECK(copy->id == copyId);
        const auto* circle = std::get_if<Circle2d>(&copy->shape);
        OH_CHECK(circle != nullptr);
        OH_CHECK(circle->radius == 5.0); // exact, no accumulated drift
        OH_CHECK(circle->center.x == 1.0 && circle->center.y == 1.0);
    }
}

// U-004: two independent copies, Undo/Redo in LIFO order, no cross-contamination.
static void TestU004_TwoIndependentCopies_UndoRedoLifoNoCrossContamination() {
    Document doc;
    const EntityId sourceA = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    const EntityId sourceB = doc.Add(MakeLine(10.0, 10.0, 11.0, 11.0));
    CopyCommand copyA(sourceA, Vector2d{1.0, 0.0});
    CopyCommand copyB(sourceB, Vector2d{1.0, 0.0});
    OH_CHECK(copyA.Execute(doc));
    OH_CHECK(copyB.Execute(doc));
    OH_CHECK(doc.Count() == 4);

    copyB.Undo(doc);
    copyA.Undo(doc);
    OH_CHECK(doc.Count() == 2); // only the two sources remain
    OH_CHECK(doc.FindEntity(sourceA) != nullptr);
    OH_CHECK(doc.FindEntity(sourceB) != nullptr);

    copyA.Redo(doc);
    copyB.Redo(doc);
    OH_CHECK(doc.Count() == 4);
    OH_CHECK(doc.FindEntity(copyA.Id()) != nullptr);
    OH_CHECK(doc.FindEntity(copyB.Id()) != nullptr);
}

// U-005: Execute -> Undo -> Execute again -- second Execute must get a
// FRESH id, not accidentally reuse the freed one via Restore-like reuse.
static void TestU005_ExecuteAgainAfterUndo_GetsDifferentId() {
    Document doc;
    const EntityId sourceId = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    CopyCommand cmd(sourceId, Vector2d{1.0, 1.0});
    OH_CHECK(cmd.Execute(doc));
    const EntityId firstId = cmd.Id();
    cmd.Undo(doc);

    OH_CHECK(cmd.Execute(doc));
    const EntityId secondId = cmd.Id();

    OH_CHECK(secondId != firstId);
    OH_CHECK(doc.FindEntity(secondId) != nullptr);
}

// --- E: Error & Edge Cases ----------------------------------------------

// E-001: source entity does not exist.
static void TestE001_SourceDoesNotExist_RejectedNoEntityCreated() {
    Document doc;
    const EntityId bogus = 99999;
    CopyCommand cmd(bogus, Vector2d{1.0, 1.0});

    const bool result = cmd.Execute(doc);

    OH_CHECK(!result);
    OH_CHECK(doc.Empty());
    OH_CHECK(cmd.Id() == kInvalidEntityId);
}

// E-002: source entity on a locked layer.
static void TestE002_SourceOnLockedLayer_Rejected() {
    Document doc;
    const EntityId sourceId = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0), "Locked");
    doc.FindLayer("Locked")->SetLocked(true);
    CopyCommand cmd(sourceId, Vector2d{1.0, 1.0});

    const bool result = cmd.Execute(doc);

    OH_CHECK(!result);
    OH_CHECK(doc.Count() == 1); // only the source, no copy created
}

// E-003: source entity on a hidden layer.
static void TestE003_SourceOnHiddenLayer_Rejected() {
    Document doc;
    const EntityId sourceId = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0), "Hidden");
    doc.FindLayer("Hidden")->SetVisible(false);
    CopyCommand cmd(sourceId, Vector2d{1.0, 1.0});

    const bool result = cmd.Execute(doc);

    OH_CHECK(!result);
    OH_CHECK(doc.Count() == 1);
}

int main() {
    TestF001_CopyLine_CreatesTranslatedCopySourceUnchanged();
    TestF002_CopyCircle_RadiusPreservedCenterShifted();
    TestF003_CopyArc_AnglesAndRadiusPreservedCenterShifted();
    TestF004_CopyMultipleEntities_AllCreatedIndependently();
    TestF005_ZeroDelta_CreatesExactDuplicateAtSamePosition();
    TestF006_SequentialCopies_IndependentSourceUnaffected();
    TestF007_CopyPreservesSourceLayer();

    TestU001_ExecuteThenUndo_RemovesCopyRestoresCount();
    TestU002_ExecuteUndoRedo_SameIdShapeLayer();
    TestU003_RepeatedUndoRedoCycles_NoDrift();
    TestU004_TwoIndependentCopies_UndoRedoLifoNoCrossContamination();
    TestU005_ExecuteAgainAfterUndo_GetsDifferentId();

    TestE001_SourceDoesNotExist_RejectedNoEntityCreated();
    TestE002_SourceOnLockedLayer_Rejected();
    TestE003_SourceOnHiddenLayer_Rejected();

    std::puts("CopyTests: all tests passed.");
    return 0;
}
