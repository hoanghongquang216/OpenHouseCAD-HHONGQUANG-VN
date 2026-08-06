# DELETE-001 — Test Design

Status: Phase 4 (Test Design) — complete, no implementation code written
Precondition: DELETE-001-Design.md → API contract locked

## 1. Functional Tests (`DeleteTests.cpp`)

| ID | Test | Expected result |
|---|---|---|
| F-001 | Delete an existing `Line2d` | Entity removed from `Document`; `Count()` decrements; `FindEntity(id)` returns `nullptr` |
| F-002 | Delete a `Circle2d` | Same as F-001 -- confirms shape-agnostic behavior (no per-shape special-casing exists, matching Design §1) |
| F-003 | Delete an `Arc2d` | Same as F-001 |
| F-004 | Delete leaves other entities untouched | Document with 3 entities, delete 1 -- the other 2 still resolve correctly via `FindEntity` (this is the Command-layer counterpart to COPY-001's D-007, per the Audit's flagged gap -- reindexing correctness exercised through `DeleteCommand`, not just `Document::RemoveEntity` directly) |
| F-005 | Delete preserves nothing else about the Document | Layer list unchanged, other entities' shapes bit-for-bit unchanged after a delete elsewhere in the document |

## 2. Undo/Redo Tests (`DeleteUndoRedoTests.cpp`)

| ID | Sequence | Assertions after each step |
|---|---|---|
| U-001 | Execute → Undo | Entity reappears with the exact same id, shape, and layer it had before deletion; `Count()` back to pre-delete value |
| U-002 | Execute → Undo → Redo | Entity gone again; `Count()` back to post-delete value |
| U-003 | Execute → Undo → Redo → Undo → Redo (repeated cycles) | Every cycle reproduces identical results -- no drift, id stays stable across cycles, matching the same concern already validated for Copy (COPY-001's U-003) and Translate/Rotate/Scale's original Memento justification |
| U-004 | Two independent `DeleteCommand`s (different entities), Undo both, Redo both, in various orders | Each command restores/removes only its own entity -- no cross-contamination, matching COPY-001's U-004 pattern |
| U-005 | Execute → Undo → Execute again (same id) | Second Execute succeeds identically (same id, same shape) -- confirms no stale state is cached between calls, per Design §4's ownership discipline |

## 3. Error & Edge Cases

| ID | Case | Expected result |
|---|---|---|
| E-001 | Target id does not exist | `Execute` returns `false`; `Document` unchanged; no undo-relevant state recorded |
| E-002 | Target on a locked layer | Rejected via `CanTransform`, same convention as every other editing command |
| E-003 | Target on a hidden layer | Rejected via `CanTransform` |
| E-004 | Delete the only entity in the Document | `Document` becomes `Empty()`; Undo restores it; no special-casing needed since `RemoveEntity`/`Restore` don't distinguish "last entity" as a special case |
| E-005 | Undo before any Execute / Redo without prior Undo | Same scoping decision as COPY-001-Test-Design.md's E-005 -- this is Command-History-stack sequencing, not `DeleteCommand`'s own responsibility; `Restore`/`RemoveEntity`'s own no-op-on-invalid-id behavior makes this safe by construction (see Design §3), but a dedicated History-stack test suite (if/when one exists) should own the "is this a legal call sequence" question, not this file |

## 4. Regression Tests (existing suites — must stay green, unmodified)

| Suite | Must still pass |
|---|---|
| `OpenHouseDocumentRemoveRestoreTests` | Unchanged -- `RemoveEntity`/`Restore` themselves are not modified by this sprint |
| `OpenHouseEntityCreationCommandBaseTests` | Unchanged -- confirms `DeleteCommand` NOT subclassing this base didn't require any change to it |
| `OpenHouseCopyTests` | Unchanged -- confirms Delete and Copy don't interfere via any shared state (there is none) |
| `OpenHouseCommandTests`, `OpenHouseTrimTests`, `OpenHouseExtendTests` | Unchanged, same as every prior sprint's regression baseline |

## 5. Sprint completion criteria

Same standard as COPY-001-Test-Design.md §6: all tests above pass, all regression
suites pass unmodified, CI green, and any new `DESIGN_DEBT.md` entry is a
conscious Phase 6 Review decision, not something merged silently. Given the
Audit's zero-new-architecture finding, this sprint's only deliverable beyond
docs is `DeleteCommand` itself plus `DeleteTests.cpp` -- expected to be
implementable as a single PR rather than COPY-001's four, since there is no
Document-infrastructure or shared-base layer to sequence separately.
