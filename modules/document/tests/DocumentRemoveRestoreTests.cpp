// Tests for Document::RemoveEntity / Document::Restore, added for
// COPY-001 (Sprint 4). Maps 1:1 to docs/design/COPY-001-Test-Design.md
// Section 3 (Document Integrity Tests, D-001..D-008).
//
// These test Document's new infrastructure directly, independent of
// CopyCommand -- CopyCommand-specific tests (F/U/E groups) belong to
// PR#3, not here.

#include <openhouse/document/Command.hpp> // TranslateCommand, for D-008
#include <openhouse/document/Document.hpp>

#include <openhouse/testing/Check.hpp>
#include <cstdio>
#include <variant>

using namespace openhouse::document;
using namespace openhouse::geometry;

static Line2d MakeLine(double x1, double y1, double x2, double y2) {
    return Line2d{Point2d{x1, y1}, Point2d{x2, y2}};
}

// D-001: RemoveEntity on an id that exists.
static void TestRemoveEntity_ExistingId_RemovesAndReturnsTrue() {
    Document doc;
    const EntityId id = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    OH_CHECK(doc.Count() == 1);

    const bool removed = doc.RemoveEntity(id);

    OH_CHECK(removed);
    OH_CHECK(doc.Count() == 0);
    OH_CHECK(doc.FindEntity(id) == nullptr);
}

// D-002: RemoveEntity on an id that never existed.
static void TestRemoveEntity_UnknownId_NoOpReturnsFalse() {
    Document doc;
    const EntityId real = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    const EntityId bogus = real + 1000; // never issued by this Document

    const bool removed = doc.RemoveEntity(bogus);

    OH_CHECK(!removed);
    OH_CHECK(doc.Count() == 1); // untouched
    OH_CHECK(doc.FindEntity(real) != nullptr);
}

// D-003: RemoveEntity twice on the same id -- idempotent-safe.
static void TestRemoveEntity_CalledTwice_SecondCallIsNoOp() {
    Document doc;
    const EntityId id = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));

    OH_CHECK(doc.RemoveEntity(id));
    OH_CHECK(!doc.RemoveEntity(id));
    OH_CHECK(doc.Count() == 0);
}

// D-004: Restore on a freshly-removed id, with the exact saved shape/layer.
static void TestRestore_FreshlyRemovedId_ReappearsExactly() {
    Document doc;
    const Line2d shape = MakeLine(2.0, 3.0, 5.0, 7.0);
    const EntityId id = doc.Add(shape, "Walls");
    OH_CHECK(doc.RemoveEntity(id));
    OH_CHECK(doc.FindEntity(id) == nullptr);

    const bool restored = doc.Restore(id, Shape{shape}, "Walls");

    OH_CHECK(restored);
    const Entity* entity = doc.FindEntity(id);
    OH_CHECK(entity != nullptr);
    OH_CHECK(entity->id == id);
    OH_CHECK(entity->layer == "Walls");
    const auto* restoredLine = std::get_if<Line2d>(&entity->shape);
    OH_CHECK(restoredLine != nullptr);
    OH_CHECK(restoredLine->start.x == shape.start.x && restoredLine->start.y == shape.start.y);
    OH_CHECK(restoredLine->end.x == shape.end.x && restoredLine->end.y == shape.end.y);
}

// D-005: Restore on an id currently occupied by a live entity.
static void TestRestore_OccupiedId_RejectedReturnsFalse() {
    Document doc;
    const EntityId id = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    // id is still live -- never removed.

    const bool restored = doc.Restore(id, Shape{MakeLine(9.0, 9.0, 9.0, 9.0)}, "0");

    OH_CHECK(!restored);
    OH_CHECK(doc.Count() == 1);
    // Original entity must be untouched, not overwritten.
    const Entity* entity = doc.FindEntity(id);
    OH_CHECK(entity != nullptr);
    const auto* line = std::get_if<Line2d>(&entity->shape);
    OH_CHECK(line != nullptr);
    OH_CHECK(line->start.x == 0.0 && line->start.y == 0.0);
    OH_CHECK(line->end.x == 1.0 && line->end.y == 1.0);
}

// D-006: id-collision regression -- Add() after a Restore() must never
// reissue an id equal to or below any previously-issued id, including
// the restored one. This is the direct test for the risk flagged in
// COPY-001-Architecture-Audit.md.
static void TestAdd_AfterRestore_NeverCollidesWithRestoredId() {
    Document doc;
    const EntityId first = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    const EntityId second = doc.Add(MakeLine(1.0, 1.0, 2.0, 2.0));
    OH_CHECK(doc.RemoveEntity(second));
    OH_CHECK(doc.Restore(second, Shape{MakeLine(1.0, 1.0, 2.0, 2.0)}, "0"));

    const EntityId third = doc.Add(MakeLine(2.0, 2.0, 3.0, 3.0));

    OH_CHECK(third != first);
    OH_CHECK(third != second);
    OH_CHECK(third > second);
    OH_CHECK(third > first);
}

// D-007: reindexing correctness when a NON-LAST entity is removed --
// the exact case TODO(Spiral5) originally warned about. Every remaining
// entity, not just the ones after the removed one, must still resolve
// correctly, and a subsequent Add() must not corrupt the index either.
static void TestRemoveEntity_MiddleOfThree_RemainingEntitiesStillResolve() {
    Document doc;
    const EntityId a = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    const EntityId b = doc.Add(MakeLine(1.0, 1.0, 2.0, 2.0));
    const EntityId c = doc.Add(MakeLine(2.0, 2.0, 3.0, 3.0));
    OH_CHECK(doc.Count() == 3);

    OH_CHECK(doc.RemoveEntity(b)); // remove the middle one

    OH_CHECK(doc.Count() == 2);
    OH_CHECK(doc.FindEntity(b) == nullptr);
    const Entity* entityA = doc.FindEntity(a);
    const Entity* entityC = doc.FindEntity(c);
    OH_CHECK(entityA != nullptr);
    OH_CHECK(entityC != nullptr);
    OH_CHECK(entityA->id == a);
    OH_CHECK(entityC->id == c);
    OH_CHECK(std::get<Line2d>(entityA->shape).start.x == 0.0);
    OH_CHECK(std::get<Line2d>(entityC->shape).start.x == 2.0);

    // Adding a new entity afterward must not corrupt the repaired index.
    const EntityId d = doc.Add(MakeLine(3.0, 3.0, 4.0, 4.0));
    OH_CHECK(doc.Count() == 3);
    OH_CHECK(doc.FindEntity(a) != nullptr);
    OH_CHECK(doc.FindEntity(c) != nullptr);
    OH_CHECK(doc.FindEntity(d) != nullptr);
}

// D-008: shape independence -- a copy's clone must be a real value
// copy, not an aliased reference. Mutating the copy (via
// TranslateCommand, exercising the existing Command machinery) must
// leave the original entity's shape untouched.
static void TestClonedShape_IsIndependent_MutationDoesNotAffectSource() {
    Document doc;
    const Line2d original = MakeLine(0.0, 0.0, 1.0, 1.0);
    const EntityId sourceId = doc.Add(original, "0");

    // Simulate "clone" the way CopyCommand::BuildEntity will (PR#3) --
    // copy the Shape value and add it as a new entity.
    const Entity* source = doc.FindEntity(sourceId);
    OH_CHECK(source != nullptr);
    const EntityId cloneId = doc.Add(source->shape, source->layer);
    OH_CHECK(cloneId != sourceId);

    TranslateCommand translate(cloneId, Vector2d{5.0, 5.0});
    OH_CHECK(translate.Execute(doc));

    const Entity* sourceAfter = doc.FindEntity(sourceId);
    const Entity* cloneAfter = doc.FindEntity(cloneId);
    OH_CHECK(sourceAfter != nullptr);
    OH_CHECK(cloneAfter != nullptr);
    OH_CHECK(std::get<Line2d>(sourceAfter->shape).start.x == 0.0);
    OH_CHECK(std::get<Line2d>(sourceAfter->shape).start.y == 0.0);
    OH_CHECK(std::get<Line2d>(sourceAfter->shape).end.x == 1.0);
    OH_CHECK(std::get<Line2d>(sourceAfter->shape).end.y == 1.0);
    OH_CHECK(std::get<Line2d>(cloneAfter->shape).start.x == 5.0);
    OH_CHECK(std::get<Line2d>(cloneAfter->shape).start.y == 5.0);
    OH_CHECK(std::get<Line2d>(cloneAfter->shape).end.x == 6.0);
    OH_CHECK(std::get<Line2d>(cloneAfter->shape).end.y == 6.0);
}

// Regression guard: Add()'s existing contract (fresh monotonic id,
// auto-create layer) must be completely unaffected by the new methods
// existing alongside it.
static void TestAdd_StillAssignsFreshMonotonicIds_Unaffected() {
    Document doc;
    const EntityId first = doc.Add(MakeLine(0.0, 0.0, 1.0, 1.0));
    const EntityId second = doc.Add(MakeLine(1.0, 1.0, 2.0, 2.0));
    OH_CHECK(second > first);
    OH_CHECK(doc.FindLayer(Document::kDefaultLayerName) != nullptr);
}

int main() {
    TestRemoveEntity_ExistingId_RemovesAndReturnsTrue();
    TestRemoveEntity_UnknownId_NoOpReturnsFalse();
    TestRemoveEntity_CalledTwice_SecondCallIsNoOp();
    TestRestore_FreshlyRemovedId_ReappearsExactly();
    TestRestore_OccupiedId_RejectedReturnsFalse();
    TestAdd_AfterRestore_NeverCollidesWithRestoredId();
    TestRemoveEntity_MiddleOfThree_RemainingEntitiesStillResolve();
    TestClonedShape_IsIndependent_MutationDoesNotAffectSource();
    TestAdd_StillAssignsFreshMonotonicIds_Unaffected();

    std::puts("DocumentRemoveRestoreTests: all tests passed.");
    return 0;
}
