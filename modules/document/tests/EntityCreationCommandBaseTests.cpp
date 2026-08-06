// Tests for EntityCreationCommandBase, added for COPY-001 (Sprint 4).
// Maps to docs/design/COPY-001-Design.md Section 4.
//
// Deliberately independent of CopyCommand (PR#3, not written yet):
// TestCreationCommand below is the minimal stub needed to exercise the
// base class's Execute/Undo/Redo lifecycle -- it does NOT clone a
// source entity, does NOT translate, and does NOT gate on
// CanTransform. Keeping it that thin is intentional: it should not
// become "CopyCommand in miniature", or these tests would stop being a
// clean test of the base class in isolation.

#include <openhouse/document/Command.hpp>
#include <openhouse/document/Document.hpp>

#include <openhouse/testing/Check.hpp>
#include <cstdio>
#include <optional>

using namespace openhouse::document;
using namespace openhouse::geometry;

namespace {

// Minimal stub subclass. `shouldSucceed` controls whether BuildEntity()
// returns a value or std::nullopt, so tests can exercise both the
// accept and reject paths without any real domain logic.
class TestCreationCommand final : public EntityCreationCommandBase {
public:
    explicit TestCreationCommand(bool shouldSucceed = true) : shouldSucceed_(shouldSucceed) {}

protected:
    [[nodiscard]] std::optional<Entity> BuildEntity(Document& /*doc*/) override {
        if (!shouldSucceed_) {
            return std::nullopt;
        }
        return Entity{kInvalidEntityId, Shape{Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}}}, "0"};
    }

private:
    bool shouldSucceed_;
};

} // namespace

// --- Execute -----------------------------------------------------------

static void TestExecute_Success_CreatesEntityAndReturnsTrue() {
    Document doc;
    TestCreationCommand cmd;

    const bool result = cmd.Execute(doc);

    OH_CHECK(result);
    OH_CHECK(doc.Count() == 1);
    OH_CHECK(cmd.Id() != kInvalidEntityId);
    OH_CHECK(doc.FindEntity(cmd.Id()) != nullptr);
}

// Strong failure safety: BuildEntity() rejecting must leave Document
// completely untouched -- no orphaned entity, no partial state.
static void TestExecute_BuildEntityRejects_DocumentUnchangedReturnsFalse() {
    Document doc;
    TestCreationCommand cmd(/*shouldSucceed=*/false);

    const bool result = cmd.Execute(doc);

    OH_CHECK(!result);
    OH_CHECK(doc.Empty());
    OH_CHECK(cmd.Id() == kInvalidEntityId); // command's own state also untouched
}

static void TestExecute_DoesNotDisturbOtherEntities() {
    Document doc;
    const EntityId existing = doc.Add(Line2d{Point2d{5.0, 5.0}, Point2d{6.0, 6.0}});
    TestCreationCommand cmd;

    OH_CHECK(cmd.Execute(doc));

    OH_CHECK(doc.Count() == 2);
    OH_CHECK(doc.FindEntity(existing) != nullptr);
    OH_CHECK(cmd.Id() != existing);
}

// --- Undo/Redo -----------------------------------------------------------

static void TestUndo_AfterExecute_RemovesTheCreatedEntity() {
    Document doc;
    TestCreationCommand cmd;
    OH_CHECK(cmd.Execute(doc));
    const EntityId createdId = cmd.Id();

    cmd.Undo(doc);

    OH_CHECK(doc.Empty());
    OH_CHECK(doc.FindEntity(createdId) == nullptr);
}

static void TestRedo_AfterUndo_RestoresSameId() {
    Document doc;
    TestCreationCommand cmd;
    OH_CHECK(cmd.Execute(doc));
    const EntityId originalId = cmd.Id();
    cmd.Undo(doc);
    OH_CHECK(doc.Empty());

    cmd.Redo(doc);

    OH_CHECK(doc.Count() == 1);
    const Entity* entity = doc.FindEntity(originalId);
    OH_CHECK(entity != nullptr);
    OH_CHECK(entity->id == originalId);
    OH_CHECK(entity->layer == "0");
}

// The direct regression test for cross-cycle id/shape stability -- the
// same concern that originally justified the Memento pattern for
// Translate/Rotate/Scale applies here too.
static void TestRepeatedUndoRedoCycles_IdAndShapeStayStable() {
    Document doc;
    TestCreationCommand cmd;
    OH_CHECK(cmd.Execute(doc));
    const EntityId id = cmd.Id();

    for (int cycle = 0; cycle < 3; ++cycle) {
        cmd.Undo(doc);
        OH_CHECK(doc.FindEntity(id) == nullptr);
        cmd.Redo(doc);
        const Entity* entity = doc.FindEntity(id);
        OH_CHECK(entity != nullptr);
        OH_CHECK(entity->id == id);
        const auto* line = std::get_if<Line2d>(&entity->shape);
        OH_CHECK(line != nullptr);
        OH_CHECK(line->start.x == 0.0 && line->start.y == 0.0);
        OH_CHECK(line->end.x == 1.0 && line->end.y == 1.0);
    }
}

// --- Out-of-order call safety (Command State Machine) ---------------------

// Undo() before any successful Execute() must be a safe no-op, not a
// crash or a corrupting operation on an unrelated entity.
static void TestUndo_BeforeExecute_IsSafeNoOp() {
    Document doc;
    const EntityId decoy = doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}});
    TestCreationCommand cmd; // Execute() never called

    cmd.Undo(doc);

    OH_CHECK(doc.Count() == 1); // decoy entity untouched
    OH_CHECK(doc.FindEntity(decoy) != nullptr);
}

// Redo() before any successful Execute() must equally be a safe no-op.
static void TestRedo_BeforeExecute_IsSafeNoOp() {
    Document doc;
    const EntityId decoy = doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}});
    TestCreationCommand cmd; // Execute() never called

    cmd.Redo(doc);

    OH_CHECK(doc.Count() == 1); // decoy entity untouched, nothing new created
    OH_CHECK(doc.FindEntity(decoy) != nullptr);
}

// A failed Execute() must leave the command in the same "never
// executed" state as a freshly-constructed one -- Undo()/Redo() called
// afterward must still be safe no-ops, not act on stale state.
static void TestUndoRedo_AfterFailedExecute_StillSafeNoOps() {
    Document doc;
    TestCreationCommand cmd(/*shouldSucceed=*/false);
    OH_CHECK(!cmd.Execute(doc));

    cmd.Undo(doc);
    cmd.Redo(doc);

    OH_CHECK(doc.Empty());
}

// Calling Undo() twice in a row (double-undo) must not remove anything
// belonging to a different entity, and must not crash.
static void TestUndo_CalledTwice_SecondCallIsSafeNoOp() {
    Document doc;
    const EntityId decoy = doc.Add(Line2d{Point2d{9.0, 9.0}, Point2d{9.0, 9.0}});
    TestCreationCommand cmd;
    OH_CHECK(cmd.Execute(doc));

    cmd.Undo(doc);
    cmd.Undo(doc); // second call -- id_ is still the same, entity already gone

    OH_CHECK(doc.Count() == 1); // only the decoy remains
    OH_CHECK(doc.FindEntity(decoy) != nullptr);
}

int main() {
    TestExecute_Success_CreatesEntityAndReturnsTrue();
    TestExecute_BuildEntityRejects_DocumentUnchangedReturnsFalse();
    TestExecute_DoesNotDisturbOtherEntities();

    TestUndo_AfterExecute_RemovesTheCreatedEntity();
    TestRedo_AfterUndo_RestoresSameId();
    TestRepeatedUndoRedoCycles_IdAndShapeStayStable();

    TestUndo_BeforeExecute_IsSafeNoOp();
    TestRedo_BeforeExecute_IsSafeNoOp();
    TestUndoRedo_AfterFailedExecute_StillSafeNoOps();
    TestUndo_CalledTwice_SecondCallIsSafeNoOp();

    std::puts("EntityCreationCommandBaseTests: all tests passed.");
    return 0;
}
