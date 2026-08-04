#include <openhouse/document/CommandHistory.hpp>
#include <openhouse/document/MacroCommand.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>
#include <memory>
#include <variant>

using namespace openhouse::document;
using namespace openhouse::geometry;

static void TestEmptyHistoryCannotUndoOrRedo() {
    Document doc;
    CommandHistory history;
    OH_CHECK(!history.CanUndo());
    OH_CHECK(!history.CanRedo());
    OH_CHECK(!history.Undo(doc));
    OH_CHECK(!history.Redo(doc));
}

static void TestExecuteSuccessfulCommandPushesToUndoStack() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    CommandHistory history;

    OH_CHECK(history.Execute(doc, std::make_unique<ScaleCommand>(id, 2.0, Point2d{0.0, 0.0})));
    OH_CHECK(history.UndoCount() == 1);
    OH_CHECK(history.CanUndo());
    OH_CHECK(!history.CanRedo());
}

static void TestExecuteFailedCommandIsNotPushedToHistory() {
    // A command whose own Execute() rejects the operation (locked
    // layer, invalid factor, unknown entity) must not enter history at
    // all -- there's nothing to undo for a change that never happened.
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Locked");
    doc.FindLayer("Locked")->SetLocked(true);
    CommandHistory history;

    OH_CHECK(!history.Execute(doc, std::make_unique<ScaleCommand>(id, 2.0, Point2d{0.0, 0.0})));
    OH_CHECK(history.UndoCount() == 0);
    OH_CHECK(!history.CanUndo());
}

static void TestUndoMovesCommandFromUndoStackToRedoStack() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    CommandHistory history;
    OH_CHECK(history.Execute(doc, std::make_unique<ScaleCommand>(id, 2.0, Point2d{0.0, 0.0})));

    OH_CHECK(history.Undo(doc));
    OH_CHECK(history.UndoCount() == 0);
    OH_CHECK(history.RedoCount() == 1);

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->radius == 1.0); // restored
}

static void TestRedoMovesCommandBackToUndoStack() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    CommandHistory history;
    OH_CHECK(history.Execute(doc, std::make_unique<ScaleCommand>(id, 2.0, Point2d{0.0, 0.0})));
    OH_CHECK(history.Undo(doc));

    OH_CHECK(history.Redo(doc));
    OH_CHECK(history.UndoCount() == 1);
    OH_CHECK(history.RedoCount() == 0);

    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->radius == 2.0); // reapplied
}

// The single most commonly-forgotten rule in an undo/redo
// implementation: executing a NEW command after an Undo must discard
// the redo stack -- the "future" that Undo made available to Redo no
// longer makes sense once a different action has been taken instead.
static void TestExecutingNewCommandAfterUndoClearsRedoStack() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    CommandHistory history;

    OH_CHECK(history.Execute(doc, std::make_unique<ScaleCommand>(id, 2.0, Point2d{0.0, 0.0})));
    OH_CHECK(history.Undo(doc));
    OH_CHECK(history.RedoCount() == 1); // redo is available here

    OH_CHECK(history.Execute(doc, std::make_unique<ScaleCommand>(id, 3.0, Point2d{0.0, 0.0})));
    OH_CHECK(history.RedoCount() == 0); // ...and gone after a new action
    OH_CHECK(!history.CanRedo());
}

static void TestMultipleUndoRedoCyclesRestoreCorrectIntermediateStates() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    CommandHistory history;

    OH_CHECK(history.Execute(doc, std::make_unique<ScaleCommand>(id, 2.0, Point2d{0.0, 0.0})));
    OH_CHECK(history.Execute(doc, std::make_unique<ScaleCommand>(id, 3.0, Point2d{0.0, 0.0})));
    // radius: 1.0 -> 2.0 -> 6.0

    const auto* afterBoth = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(afterBoth->radius == 6.0);

    OH_CHECK(history.Undo(doc)); // back to 2.0
    const auto* afterOneUndo = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(afterOneUndo->radius == 2.0);

    OH_CHECK(history.Undo(doc)); // back to 1.0
    const auto* afterTwoUndos = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(afterTwoUndos->radius == 1.0);

    OH_CHECK(!history.Undo(doc)); // nothing left

    OH_CHECK(history.Redo(doc)); // forward to 2.0
    OH_CHECK(history.Redo(doc)); // forward to 6.0
    const auto* afterBothRedos = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(afterBothRedos->radius == 6.0);
}

static void TestClearDiscardsHistoryWithoutTouchingDocument() {
    Document doc;
    const EntityId id = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    CommandHistory history;
    OH_CHECK(history.Execute(doc, std::make_unique<ScaleCommand>(id, 2.0, Point2d{0.0, 0.0})));

    history.Clear();
    OH_CHECK(!history.CanUndo());
    OH_CHECK(!history.CanRedo());
    OH_CHECK(history.UndoCount() == 0);

    // The entity itself is untouched by Clear() -- it keeps whatever
    // the last executed command left it as.
    const auto* circle = std::get_if<Circle2d>(&doc.FindEntity(id)->shape);
    OH_CHECK(circle->radius == 2.0);
}

static void TestHistoryWorksWithMacroCommand() {
    // CommandHistory doesn't care whether a command is a single
    // TranslateCommand/RotateCommand/ScaleCommand or a MacroCommand
    // wrapping several -- ICommand's uniform interface is exactly what
    // makes this work without CommandHistory needing any special case.
    Document doc;
    const EntityId a = doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    const EntityId b = doc.Add(Circle2d{Point2d{10.0, 10.0}, 2.0});

    auto macro = std::make_unique<MacroCommand>();
    macro->Add(std::make_unique<ScaleCommand>(a, 2.0, Point2d{0.0, 0.0}));
    macro->Add(std::make_unique<ScaleCommand>(b, 2.0, Point2d{0.0, 0.0}));

    CommandHistory history;
    OH_CHECK(history.Execute(doc, std::move(macro)));
    OH_CHECK(history.UndoCount() == 1); // ONE history entry for both entities

    OH_CHECK(history.Undo(doc));
    const auto* circleA = std::get_if<Circle2d>(&doc.FindEntity(a)->shape);
    const auto* circleB = std::get_if<Circle2d>(&doc.FindEntity(b)->shape);
    OH_CHECK(circleA->radius == 1.0);
    OH_CHECK(circleB->radius == 2.0);
}

int main() {
    TestEmptyHistoryCannotUndoOrRedo();
    TestExecuteSuccessfulCommandPushesToUndoStack();
    TestExecuteFailedCommandIsNotPushedToHistory();
    TestUndoMovesCommandFromUndoStackToRedoStack();
    TestRedoMovesCommandBackToUndoStack();
    TestExecutingNewCommandAfterUndoClearsRedoStack();
    TestMultipleUndoRedoCyclesRestoreCorrectIntermediateStates();
    TestClearDiscardsHistoryWithoutTouchingDocument();
    TestHistoryWorksWithMacroCommand();

    std::puts("CommandHistoryTests: all tests passed.");
    return 0;
}
