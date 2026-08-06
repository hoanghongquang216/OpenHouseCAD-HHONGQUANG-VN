# COPY-001 — Test Design

Status: Phase 4 (Test Design) — complete, no implementation code written
Precondition: COPY-001-Design.md → API contracts locked

Tests below are written against the exact contracts from COPY-001-Design.md §3–4
(`Document::RemoveEntity`, `Document::Restore`, `EntityCreationCommandBase`,
`CopyCommand::BuildEntity`). Each test id maps 1:1 to a test case to be written in
Phase 5, before any production code.

## 1. Functional Tests (`CopyTests.cpp`)

| ID | Test | Expected result |
|---|---|---|
| F-001 | Copy `Line2d` with `delta=(3,4)` | New entity created; its `Line2d` endpoints equal source endpoints + delta; source entity's endpoints unchanged |
| F-002 | Copy `Circle2d` | New entity's radius equals source radius exactly (no drift); center shifted by delta |
| F-003 | Copy `Arc2d` | New entity's start/end/sweep angles equal source exactly; center shifted by delta, radius unchanged |
| F-004 | Copy N entities via a loop of `CopyCommand`s (not `CopySelection` — out of scope per Design §1) | All N new entities present, each independently correct; `Document::Count()` increases by exactly N |
| F-005 | `delta = (0, 0)` | New entity created at the exact same position as source — not rejected (per COPY-001.md §4, this is a valid LibreCAD-`Duplicate`-style case, not treated as a no-op) |
| F-006 | Two sequential `CopyCommand`s from the same source | Second copy is unaffected by the first; source entity unchanged after both |
| F-007 | Copy source on a layer other than `"0"` | New entity's `layer` equals source's layer exactly (not the default layer) |

## 2. Undo/Redo Tests (`CopyUndoRedoTests.cpp`)

The core assertion after **every** step below: check entity count, the copy's id,
its geometry, and its layer — not just "did Undo/Redo not crash."

| ID | Sequence | Assertions after each step |
|---|---|---|
| U-001 | Execute → Undo | Count back to pre-copy value; `FindEntity(newId) == nullptr`; source entity unchanged |
| U-002 | Execute → Undo → Redo | Entity reappears with the **same** `EntityId` as originally returned by Execute; same shape; same layer |
| U-003 | Execute → Undo → Redo → Undo → Redo (per the sequence proposed) | Every cycle reproduces the identical id/shape/layer — no drift across repeated cycles (this is the direct COPY-001 analogue of the Scale-drift concern that originally justified the Memento pattern — must be checked explicitly, not assumed from U-002 passing once) |
| U-004 | Execute Copy A, Execute Copy B (different sources), Undo, Undo, Redo, Redo | Undo/Redo affect the correct entity in LIFO order; no cross-contamination between A's and B's ids |
| U-005 | Execute → Undo → Execute again (same source, same delta) | Second Execute produces a **different** `EntityId` than the first (fresh `Add()`, not an accidental `Restore()` reuse of the freed id) |

## 3. Document Integrity Tests (`DocumentRemoveRestoreTests.cpp`)

These validate `Document`'s new API directly — independent of `CopyCommand` — since
`RemoveEntity`/`Restore` are general Document infrastructure, not Copy-only.

| ID | Test | Expected result |
|---|---|---|
| D-001 | `RemoveEntity` on an id that exists | Returns `true`; entity gone from `Entities()`; `Count()` decremented |
| D-002 | `RemoveEntity` on an id that never existed | Returns `false`; document unchanged (no-op) |
| D-003 | `RemoveEntity` twice on the same id | First call `true`, second call `false` — idempotent-safe, matches `SelectionSet::Deselect`'s existing convention |
| D-004 | `Restore` on a freshly-removed id, with the exact saved shape/layer | Returns `true`; entity reappears at that exact id; `FindEntity` resolves it |
| D-005 | `Restore` on an id that is currently occupied by a live entity | Returns `false` (precondition violation, per Design §3's defensive-return-over-assert choice for this path); document unchanged |
| D-006 | `nextId_` behavior: `Add()` a fresh entity immediately after a `Restore()` | New id is strictly greater than every id ever issued (including the restored one) — **the direct regression test for the id-collision risk flagged in the Audit** |
| D-007 | `entities_`/`index_` consistency after `RemoveEntity` on a **non-last** entity, then `Add()` a new one | Every existing entity (not just the removed one) still resolves correctly via `FindEntity` — this is the exact reindexing risk `TODO(Spiral5)` warned about; must be tested with ≥3 entities and removal of the middle one, not just tail removal |
| D-008 | Shape independence: mutate the clone (via a subsequent `TranslateCommand` on the new id) | Source entity's shape is provably unaffected — confirms clone is a real value copy, not an aliased reference (relevant because `Shape` is a `variant`, not a pointer, but worth asserting explicitly since it's the actual copy-correctness guarantee the whole sprint exists for) |

## 4. Error & Edge Cases

| ID | Case | Expected result | Notes |
|---|---|---|---|
| E-001 | `sourceId` does not exist | `BuildEntity` returns `nullopt`; `Execute` returns `false`; no entity created, no undo entry should be pushed by the caller | Matches existing `TranslateEntity`-style rejection convention |
| E-002 | Source entity on a locked layer | Rejected via `CanTransform`, same as E-001 | Direct reuse of the existing gate — no new logic to test beyond confirming `CopyCommand` actually calls it |
| E-003 | Source entity on a hidden layer | Rejected via `CanTransform` | Same as E-002 |
| E-004 | "Restore with an id that already exists" | Covered by D-005 above | Listed here for traceability to the original ask |
| E-005 | "Undo before any Execute" / "Redo with no prior Undo" | **Not** `CopyCommand`'s responsibility — sequencing (has Execute run? is there Undo history?) belongs to whatever Command-History/undo-stack owns the stack discipline, which is out of scope for this sprint (no such stack exists yet in the audited files). Flag as a **History-stack test**, to be written whenever that stack is introduced — not a COPY-001 test | Explicitly scoping this out rather than silently dropping it |
| E-006 | Copy after source entity was deleted | **Currently unreachable** — `Document` has no deletion capability until DELETE-001 ships (this sprint only adds `RemoveEntity`/`Restore` as Command-internal machinery, not a public "delete" user action). Add this test in **DELETE-001's** suite once a real Delete path exists that a Copy could race against | Explicitly deferred, not skipped silently |
| E-007 | "Null shape" | Not applicable — `Shape` is a closed `std::variant<Line2d, Circle2d, Arc2d>`, never null/empty by construction | Documenting why this isn't a real case for this codebase, per the original ask |

## 5. Regression Tests (existing suites — must stay green, not modified)

`RemoveEntity`/`Restore` are new methods; they must not change behavior of anything
already relying on `Document`, `Add()`, or the existing Command classes.

| Suite | Must still pass |
|---|---|
| `OpenHouseCommandTests` (Translate/Rotate/Scale via `EntityShapeCommandBase`) | Unchanged — `EntityShapeCommandBase` is not touched by this sprint |
| `OpenHouseTrimTests` | Unchanged — Trim doesn't call `Add`/`Remove`/`Restore` |
| `OpenHouseExtendTests` | Unchanged — same reasoning |
| Existing `Document` tests (layer creation, `Bounds()`, `Clear()`, `FindEntity`/`FindEntityMutable`) | Must still pass unmodified — confirms `Add()`'s existing behavior (auto-id assignment, layer auto-creation) is untouched by adding the new `Restore` method alongside it |

## 6. Sprint completion criteria

COPY-001 is done only when **all** of the following hold simultaneously:

- ✅ Design (Phase 3) reviewed and accepted — already the case.
- ✅ All tests in Sections 1–4 above pass.
- ✅ All Section 5 regression suites pass unmodified.
- ✅ CI green.
- ✅ No new item added to `DESIGN_DEBT.md` without an explicit reviewed rationale (an
  item *may* be added — e.g. the O(n) reindex note from the Audit could become a
  tracked-but-accepted debt entry — but it must be a conscious decision at Phase 6
  Review, not something merged silently).

Meeting these leaves `Document::RemoveEntity`/`Restore` and `EntityCreationCommandBase`
in place and tested, so DELETE-001 can begin directly at its own Phase 1 (Domain
Research) without any Document/architecture rework.
