# DELETE-001 — Architecture Audit

Status: Phase 2 (Architecture Audit) — complete, no code written
Decision: **GO**

## Scope of audit

Read in full: `Command.hpp`, `Document.hpp` (both as of commit `5b8b1b3`, post-COPY-001). `Transform.hpp`/`Selection.hpp` re-confirmed unchanged from prior audits (COPY-001-Architecture-Audit.md) -- no new read needed, nothing in DELETE-001's scope touches either.

## Findings

### Document (Document.hpp)
- `RemoveEntity(EntityId) -> bool` and `Restore(EntityId, Shape, layerName) -> bool` **already exist**, added in COPY-001 (Sprint 4) specifically because DELETE-001 was identified as a near-term consumer (Decision Gate, Option A). Both match DELETE-001's needs exactly as specified in Section 2 of DELETE-001.md:
  - `RemoveEntity` is Execute's mutating call.
  - `Restore` is Undo's mutating call.
- **No Document-layer changes required for this sprint** -- the first editing sprint (Trim/Extend/Copy all needed at least one Document or Geometry addition) with zero new infrastructure needed. This is the direct payoff of Sprint 4's "optimize for Epic, not Sprint" decision.

### Command (Command.hpp)
- `EntityCreationCommandBase` exists but is confirmed **not reusable** for Delete, per the Decision Gate already recorded in COPY-001-Design.md §4 and reconfirmed by DELETE-001.md §3's state-machine mirror: `EntityCreationCommandBase`'s hooks mean "Execute creates, Undo removes" -- Delete needs the opposite ("Execute removes, Undo creates"), which would require either inverting the base's semantics (confusing for any future reader) or adding a second constructor mode (violates API-surface minimalism). `DeleteCommand` will implement `ICommand` directly.
- `EntityShapeCommandBase` is irrelevant here too -- it mutates an existing entity's `shape` in place; Delete removes/restores whole entities, not shapes within them.

### Transform (Transform.hpp)
- `CanTransform(doc, id)` is directly reusable, unchanged, as DELETE-001's permission gate -- same as every prior editing command.

### Selection (Selection.hpp)
- Unchanged, not touched -- confirms DELETE-001.md §6's conclusion that selection-clears-after-delete is an Application-layer concern.

## Blast radius

| File | Change |
|---|---|
| `Document.hpp` | **Unchanged** -- `RemoveEntity`/`Restore` already sufficient |
| `Command.hpp` | **Modified** -- add `DeleteCommand` (implements `ICommand` directly, no new base class) |
| `Transform.hpp` | **Unchanged** -- `CanTransform` reused as-is |
| `Selection.hpp` | **Unchanged** |
| Geometry Kernel | **Unchanged** -- no geometry computation involved in delete at all |
| New test files | `DeleteTests.cpp` |

Smallest blast radius of any editing sprint so far (Trim/Extend added Geometry Kernel functions; Copy added two Document methods and a Command base; Delete adds only one concrete Command class).

## Risk assessment

- **None identified beyond what's already covered.** `RemoveEntity`'s reindexing correctness and `Restore`'s id-collision safety were already risk-assessed and regression-tested in COPY-001 (D-006, D-007) -- DELETE-001 is a pure consumer of both, introducing no new usage pattern of either (Copy already exercises the identical Remove-then-Restore-then-Remove-again sequence via its own Undo/Redo).
- One thing worth a regression test specifically for Delete (not fully covered by COPY-001's own tests): deleting a **non-last** entity via `DeleteCommand` and confirming Undo/Redo both still work correctly against the reindexed `index_` -- COPY-001's D-007 tested `RemoveEntity` directly, but not through a Command's Undo/Redo cycle. Added to Test Design (see DELETE-001-Test-Design.md).

## Decision: GO

Zero new architecture required. This sprint is implementation + tests only, directly validating the investment made in COPY-001's Decision Gate. Proceed to Phase 3 (Architecture Design) -- expected to be short, since `DeleteCommand`'s shape is already fully determined by DELETE-001.md §3.
