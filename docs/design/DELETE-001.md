# DELETE-001 — Domain Research & Functional Specification

Status: Phase 1 (Domain Research) — complete
Sprint: 5
Epic: 4 (Editing)

## 1. Reference systems

| System | Command name | Core mechanic |
|---|---|---|
| AutoCAD / BricsCAD | `ERASE` (no separate "Delete" command; the Delete key is a shortcut that invokes Erase) | Removes selected object(s) from the drawing database. Undoable via the normal Undo stack. A separate `OOPS` command exists for a one-level "un-erase" independent of the Undo stack (can restore the most recently erased set even after other commands ran) -- explicitly out of scope, see §1 below. |
| LibreCAD | `Delete` (`moddelete`, shortcut `er`) | Same effect -- selected entities removed, standard Undo restores them. |
| QCAD / BricsCAD | Equivalent Delete/Erase tool | Same model. |

**Locked/hidden layer behavior:** AutoCAD's default (`PICKLOCKEDOBJECTS=0`) prevents objects on a locked layer from being selected at all, so Erase never even reaches them; `ALL`-selection explicitly excludes frozen/locked layers too. This is exactly the `CanTransform` gate (exists, visible, not locked) already reused by Translate/Rotate/Scale/Trim/Extend/Copy.

**Multi-entity delete is atomic:** selecting several objects and erasing them is one undoable operation in every reference system -- one Undo restores the whole set.

## 2. Functional specification (OpenHouseCAD DELETE-001 scope)

**In scope:** single-entity delete, `DeleteCommand(EntityId id)`, matching the precedent set by TRIM-001/EXTEND-001/COPY-001 of shipping the single-entity case first.

**Out of scope (deferred):**
- Multi-entity delete in one user action -- composable later via `MacroCommand` of N `DeleteCommand`s, the same pattern already used for the Copy→Array relationship. No new infrastructure needed for this.
- `DeleteSelection` free function (the `*Selection` convention used by `TranslateSelection`/`RotateSelection`/`ScaleSelection`) -- mechanically trivial once `DeleteCommand` exists, not required for this sprint's Go/No-Go.
- AutoCAD's `OOPS` (undo-stack-independent single-level un-erase) -- a distinct Application-layer feature, not part of the Command/Undo core model this sprint touches.
- Any "soft delete" / recycle-bin concept -- no reference system works this way for in-session Erase; deleted-then-saved is genuinely gone, same as this project's own Undo/Redo already implies for every other command.

**Behavior:**
1. Resolve target entity by id. If it doesn't exist → reject (return `false`, no-op).
2. Layer permission check — reuse `CanTransform()` (Transform.hpp) as-is, unchanged: target's layer must exist, be visible, and not be locked. Directly matches AutoCAD's default `PICKLOCKEDOBJECTS=0` behavior.
3. Snapshot the entity's `shape` and `layer` (needed for Undo).
4. Remove the entity from the Document via the `Document::RemoveEntity()` already added in COPY-001 (Sprint 4) -- **no new Document API is required for this sprint**, unlike every prior editing sprint (Trim/Extend added geometry; Copy added `RemoveEntity`/`Restore` themselves).
5. Return `true`.

## 3. State machine

```
[Idle]
   │ DeleteCommand::Execute(id)
   ▼
[Resolve entity] ──(not found)──► [Rejected: no-op] ──► [Idle]
   │ found
   ▼
[Check CanTransform(id)] ──(locked/hidden)──► [Rejected: no-op] ──► [Idle]
   │ ok
   ▼
[Snapshot shape+layer] → [Document::RemoveEntity(id)]
   │
   ▼
[Executed] ──Undo()──► [Document::Restore(id, savedShape, savedLayer)] ──► [Idle, ready for Redo]
   │                                                                            │
   └──────────────────────────────Redo()──────────────────────────────────────►┘
        Document::RemoveEntity(id)  ← same id, no recomputation, mirrors Execute directly
```

Note this state machine is the exact mirror of `EntityCreationCommandBase`'s (Execute removes instead of creates, Undo restores instead of removes) -- confirming the Sprint 4 Decision Gate finding that Delete should NOT subclass `EntityCreationCommandBase`, since forcing the inverse direction into that base would invert the meaning of its hooks.

## 4. Edge cases

| Case | Expected behavior | Precedent |
|---|---|---|
| Target entity doesn't exist | Reject, nothing removed, no undo entry pushed | Matches every other command's missing-entity rejection |
| Target on locked layer | Reject | `CanTransform` gate, matches AutoCAD `PICKLOCKEDOBJECTS=0` default |
| Target on hidden layer | Reject | Same `CanTransform` gate |
| Delete, then Undo | Entity fully restored: same id, same shape, same layer | `Document::Restore`, already implemented and tested in COPY-001 |
| Delete a `Circle2`/`Arc2` (not just `Line2`) | Must work -- `RemoveEntity`/`Restore` are shape-agnostic (operate on the `Shape` variant as a whole), so all three variants are in scope by construction, same as Copy | N/A -- structural consequence of reusing existing Document APIs |
| Delete, Undo, Delete again (same id) | Second delete succeeds identically -- `CanTransform`/`FindEntity` re-resolve fresh each call, no stale state | Matches the "never cache a pointer/state across calls" discipline already used throughout Command.hpp |
| Two independent deletes, Undo both, Redo both | Each command's own saved (shape, layer) round-trips correctly regardless of order, same as COPY-001's U-004 | Depends on the History/undo-stack sequencing calls in the correct order -- not something `DeleteCommand` itself must special-case |

## 5. Undo/Redo rules

Same Memento discipline as every other command in this codebase: Undo does not "recompute how to re-add" the entity -- it directly restores the exact saved `(id, shape, layer)` tuple via `Document::Restore`. Redo does not re-run `Execute()`'s resolve-and-snapshot logic -- it directly calls `Document::RemoveEntity(id)` again, since the entity is guaranteed to exist at that id (Undo just put it there) and the shape/layer are unchanged from what was already snapshotted.

## 6. Selection-after-delete rule

AutoCAD/LibreCAD/QCAD all clear the selection after a successful Erase/Delete (the deleted entity obviously can't remain selected). Per the same reasoning already documented for Copy (COPY-001.md §6), `SelectionSet` is deliberately independent of `Document`/`Command` -- `DeleteCommand` does not touch Selection. Any "clear selection after delete" behavior belongs to the future Application layer, calling `selection.Deselect(id)` after a successful `DeleteCommand::Execute()`.
