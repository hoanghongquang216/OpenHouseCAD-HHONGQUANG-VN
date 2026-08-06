// Tests for DeleteCommand, added for DELETE-001 (Sprint 5). Maps to
// docs/design/DELETE-001-Test-Design.md Sections 1 (Functional), 2
// (Undo/Redo), 3 (Error/Edge).

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

// F-001: Delete an existing Line2d.
static void TestF001_DeleteLine_RemovesEntity() {
    Document doc;
    const EntityId id = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    DeleteCommand cmd(id);

    const bool result = cmd.Execute(doc);

    OH_CHECK(result);
    OH_CHECK(doc.Count() == 0);
    OH_CHECK(doc.FindEntity(id) == nullptr);
}

// F-002: Delete a Circle2d -- shape-agnostic behavior.
static void TestF002_DeleteCircle_RemovesEntity() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{5.0, 5.0}, 2.0});
    DeleteCommand cmd(id);

    OH_CHECK(cmd.Execute(doc));

    OH_CHECK(doc.Count() == 0);
    OH_CHECK(doc.FindEntity(id) == nullptr);
}

// F-003: Delete an Arc2d.
static void TestF003_DeleteArc_RemovesEntity() {
    Document doc;
    const EntityId id = doc.Add(Arc2d{Point2d{0.0, 0.0}, 3.0, 0.0, 1.0});
    DeleteCommand cmd(id);

    OH_CHECK(cmd.Execute(doc));

    OH_CHECK(doc.Count() == 0);
    OH_CHECK(doc.FindEntity(id) == nullptr);
}

// F-004: deleting one entity leaves the others correctly resolvable --
// the Command-layer counterpart to COPY-001's D-007 (reindexing
// correctness), exercised here through DeleteCommand's Undo/Redo cycle
// rather than Document::RemoveEntity directly.
static void TestF004_DeleteMiddleEntity_OthersStillResolveThroughUndoRedo() {
    Document doc;
    const EntityId a = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    const EntityId b = doc.Add(MakeLine(1.0, 1.0, 2.0, 2.0));
    const EntityId c = doc.Add(MakeLine(2.0, 2.0, 3.0, 3.0));
    DeleteCommand cmd(b);

    OH_CHECK(cmd.Execute(doc));
    OH_CHECK(doc.Count() == 2);
    OH_CHECK(doc.FindEntity(a) != nullptr);
    OH_CHECK(doc.FindEntity(c) != nullptr);

    cmd.Undo(doc);
    OH_CHECK(doc.Count() == 3);
    OH_CHECK(doc.FindEntity(a) != nullptr);
    OH_CHECK(doc.FindEntity(b) != nullptr);
    OH_CHECK(doc.FindEntity(c) != nullptr);

    cmd.Redo(doc);
    OH_CHECK(doc.Count() == 2);
    OH_CHECK(doc.FindEntity(a) != nullptr);
    OH_CHECK(doc.FindEntity(b) == nullptr);
    OH_CHECK(doc.FindEntity(c) != nullptr);
}

// F-005: delete leaves layers and other entities' shapes bit-for-bit
// unchanged.
static void TestF005_DeleteElsewhere_LayersAndOtherShapesUnchanged() {
    Document doc;
    const EntityId keep = doc.Add(MakeLine(9.0, 9.0, 8.0, 8.0), "Walls");
    const EntityId gone = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0), "Scratch");
    DeleteCommand cmd(gone);

    OH_CHECK(cmd.Execute(doc));

    OH_CHECK(doc.FindLayer("Walls") != nullptr);
    OH_CHECK(doc.FindLayer("Scratch") != nullptr); // layer itself is retained
    const Entity* kept = doc.FindEntity(keep);
    OH_CHECK(kept != nullptr);
    OH_CHECK(kept->layer == "Walls");
    const auto* line = std::get_if<Line2d>(&kept->shape);
    OH_CHECK(line != nullptr);
    OH_CHECK(line->start.x == 9.0 && line->start.y == 9.0);
    OH_CHECK(line->end.x == 8.0 && line->end.y == 8.0);
}

// --- U: Undo/Redo Tests --------------------------------------------------

// U-001: Execute -> Undo.
static void TestU001_ExecuteThenUndo_RestoresExactEntity() {
    Document doc;
    const EntityId id = doc.Add(MakeLine(2.0, 3.0, 5.0, 7.0), "Walls");
    DeleteCommand cmd(id);
    OH_CHECK(cmd.Execute(doc));
    OH_CHECK(doc.Count() == 0);

    cmd.Undo(doc);

    OH_CHECK(doc.Count() == 1);
    const Entity* entity = doc.FindEntity(id);
    OH_CHECK(entity != nullptr);
    OH_CHECK(entity->id == id);
    OH_CHECK(entity->layer == "Walls");
    const auto* line = std::get_if<Line2d>(&entity->shape);
    OH_CHECK(line != nullptr);
    OH_CHECK(line->start.x == 2.0 && line->start.y == 3.0);
    OH_CHECK(line->end.x == 5.0 && line->end.y == 7.0);
}

// U-002: Execute -> Undo -> Redo.
static void TestU002_ExecuteUndoRedo_RemovedAgain() {
    Document doc;
    const EntityId id = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    DeleteCommand cmd(id);
    OH_CHECK(cmd.Execute(doc));
    cmd.Undo(doc);
    OH_CHECK(doc.Count() == 1);

    cmd.Redo(doc);

    OH_CHECK(doc.Count() == 0);
    OH_CHECK(doc.FindEntity(id) == nullptr);
}

// U-003: repeated Execute/Undo/Redo cycles -- no drift, id stable.
static void TestU003_RepeatedUndoRedoCycles_NoDrift() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{1.0, 1.0}, 4.0}, "Walls");
    DeleteCommand cmd(id);
    OH_CHECK(cmd.Execute(doc));

    for (int cycle = 0; cycle < 4; ++cycle) {
        cmd.Undo(doc);
        const Entity* entity = doc.FindEntity(id);
        OH_CHECK(entity != nullptr);
        OH_CHECK(entity->id == id);
        OH_CHECK(entity->layer == "Walls");
        const auto* circle = std::get_if<Circle2d>(&entity->shape);
        OH_CHECK(circle != nullptr);
        OH_CHECK(circle->radius == 4.0); // exact, no accumulated drift
        OH_CHECK(circle->center.x == 1.0 && circle->center.y == 1.0);

        cmd.Redo(doc);
        OH_CHECK(doc.FindEntity(id) == nullptr);
    }
}

// U-004: two independent deletes, Undo/Redo in various orders, no
// cross-contamination.
static void TestU004_TwoIndependentDeletes_NoCrossContamination() {
    Document doc;
    const EntityId a = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    const EntityId b = doc.Add(MakeLine(10.0, 10.0, 11.0, 11.0));
    DeleteCommand deleteA(a);
    DeleteCommand deleteB(b);
    OH_CHECK(deleteA.Execute(doc));
    OH_CHECK(deleteB.Execute(doc));
    OH_CHECK(doc.Empty());

    deleteB.Undo(doc);
    deleteA.Undo(doc);
    OH_CHECK(doc.Count() == 2);
    OH_CHECK(doc.FindEntity(a) != nullptr);
    OH_CHECK(doc.FindEntity(b) != nullptr);

    deleteA.Redo(doc);
    OH_CHECK(doc.Count() == 1);
    OH_CHECK(doc.FindEntity(a) == nullptr);
    OH_CHECK(doc.FindEntity(b) != nullptr);
}

// U-005: Execute -> Undo -> Execute again (same id) -- no stale state
// cached between calls.
static void TestU005_ExecuteAgainAfterUndo_SucceedsIdentically() {
    Document doc;
    const EntityId id = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    DeleteCommand cmd(id);
    OH_CHECK(cmd.Execute(doc));
    cmd.Undo(doc);

    const bool result = cmd.Execute(doc);

    OH_CHECK(result);
    OH_CHECK(doc.Count() == 0);
    OH_CHECK(cmd.Id() == id);
}

// --- E: Error & Edge Cases ----------------------------------------------

// E-001: target id does not exist.
static void TestE001_TargetDoesNotExist_RejectedNoChange() {
    Document doc;
    const EntityId real = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    const EntityId bogus = real + 1000;
    DeleteCommand cmd(bogus);

    const bool result = cmd.Execute(doc);

    OH_CHECK(!result);
    OH_CHECK(doc.Count() == 1);
    OH_CHECK(doc.FindEntity(real) != nullptr);
}

// E-002: target on a locked layer.
static void TestE002_TargetOnLockedLayer_Rejected() {
    Document doc;
    const EntityId id = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0), "Locked");
    doc.FindLayer("Locked")->SetLocked(true);
    DeleteCommand cmd(id);

    const bool result = cmd.Execute(doc);

    OH_CHECK(!result);
    OH_CHECK(doc.Count() == 1);
    OH_CHECK(doc.FindEntity(id) != nullptr);
}

// E-003: target on a hidden layer.
static void TestE003_TargetOnHiddenLayer_Rejected() {
    Document doc;
    const EntityId id = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0), "Hidden");
    doc.FindLayer("Hidden")->SetVisible(false);
    DeleteCommand cmd(id);

    const bool result = cmd.Execute(doc);

    OH_CHECK(!result);
    OH_CHECK(doc.Count() == 1);
    OH_CHECK(doc.FindEntity(id) != nullptr);
}

// E-004: delete the only entity in the Document.
static void TestE004_DeleteOnlyEntity_DocumentBecomesEmpty() {
    Document doc;
    const EntityId id = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    DeleteCommand cmd(id);

    OH_CHECK(cmd.Execute(doc));

    OH_CHECK(doc.Empty());

    cmd.Undo(doc);
    OH_CHECK(!doc.Empty());
    OH_CHECK(doc.Count() == 1);
}

// E-005: Undo before any Execute / Redo without prior Undo -- safe
// no-ops by construction (RemoveEntity/Restore already reject an
// invalid/unoccupied id per their own contracts).
static void TestE005_OutOfOrderCalls_SafeNoOps() {
    Document doc;
    const EntityId decoy = doc.Add(MakeLine(9.0, 9.0, 9.0, 9.0));
    DeleteCommand cmd(decoy + 1000); // never executed, targets a bogus id

    cmd.Undo(doc); // Restore() on a never-issued id -- rejected internally
    cmd.Redo(doc); // RemoveEntity() on a never-issued id -- no-op

    OH_CHECK(doc.Count() == 1);
    OH_CHECK(doc.FindEntity(decoy) != nullptr);
}

int main() {
    TestF001_DeleteLine_RemovesEntity();
    TestF002_DeleteCircle_RemovesEntity();
    TestF003_DeleteArc_RemovesEntity();
    TestF004_DeleteMiddleEntity_OthersStillResolveThroughUndoRedo();
    TestF005_DeleteElsewhere_LayersAndOtherShapesUnchanged();

    TestU001_ExecuteThenUndo_RestoresExactEntity();
    TestU002_ExecuteUndoRedo_RemovedAgain();
    TestU003_RepeatedUndoRedoCycles_NoDrift();
    TestU004_TwoIndependentDeletes_NoCrossContamination();
    TestU005_ExecuteAgainAfterUndo_SucceedsIdentically();

    TestE001_TargetDoesNotExist_RejectedNoChange();
    TestE002_TargetOnLockedLayer_Rejected();
    TestE003_TargetOnHiddenLayer_Rejected();
    TestE004_DeleteOnlyEntity_DocumentBecomesEmpty();
    TestE005_OutOfOrderCalls_SafeNoOps();

    std::puts("DeleteTests: all tests passed.");
    return 0;
}
