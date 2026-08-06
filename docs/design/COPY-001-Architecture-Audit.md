# COPY-001 — Architecture Audit

Status: Phase 2 (Architecture Audit) — complete, no code written
Decision: **GO**

## Scope of audit

Read in full: `Command.hpp`, `Transform.hpp`, `Selection.hpp`, `Document.hpp` (commit `9eacac2`, post-REFACTOR-001).

## Findings

### Entity / Shape (Document.hpp)
- `Shape = variant<Line2d, Circle2d, Arc2d>` — value type, cheap to copy. No blocker; Copy's "clone" step is a plain copy-construction, nothing new needed here.
- `Entity{ id, shape, layer }` — layer stored by name (string), not pointer/index, so a cloned Entity trivially keeps the same layer reference with no dangling-pointer risk.

### Document (Document.hpp)
- `Add(Shape, layerName) -> EntityId` exists, auto-creates the layer if needed, assigns a new monotonic id. **Sufficient for Execute()'s create step as-is.**
- **Gap confirmed: no removal capability exists.** No `Remove`, `Delete`, or `Erase` method anywhere in `Document`. The `TODO(Spiral5)` comment on `FindEntity` explicitly flags that `index_`'s correctness depends on `entities_` staying append-only, and that whoever adds deletion must deliberately resolve the reindexing tradeoff.
- **Gap confirmed: no way to `Add()` with a caller-specified id.** Every `Add()` call assigns from `nextId_`. Redo needs to reproduce the *exact* id from before Undo, which today's `Add()` cannot do.

### Command (Command.hpp)
- `EntityShapeCommandBase` is built entirely around *mutating an existing entity's `shape` field in place* (`shapeBefore_`/`shapeAfter_` snapshots swapped on Undo/Redo). It has no concept of an entity being created or destroyed — `Execute()` assumes the entity already exists (`doc.FindEntity(id_)` at the top; returns `false` if missing).
- **Confirmed: `CopyCommand` cannot subclass `EntityShapeCommandBase`.** Needs a sibling base, e.g. `EntityCreationCommandBase`, whose Undo removes an entity and whose Redo re-adds it — structurally the mirror image of `EntityShapeCommandBase`, reusing the same "resolve by id, never cache a pointer" discipline (still required — `entities_` is a vector, same reallocation hazard applies to a newly-created entity's pointer just as much as an existing one's).

### Transform (Transform.hpp)
- `TranslateEntity(doc, id, delta)` already does exactly the displacement math Copy needs, already gated by `CanTransform`. **Directly reusable — no changes needed to this file.** Confirms the Domain Research conclusion (Section 1/2 of COPY-001.md): Copy = clone + reuse-Translate, not a new geometric primitive.

### Selection (Selection.hpp)
- `SelectionSet` is fully Document-independent, per its own header comment. No coupling risk. Confirms Domain Research's conclusion that selection-follows-copy is an Application-layer concern, not something `CopyCommand` should touch.

## Blast radius

| File | Change |
|---|---|
| `Document.hpp` | **Modified** — add `RemoveEntity(EntityId) -> bool`, add `Add(EntityId, Shape, layerName) -> EntityId` overload (or equivalent explicit-id path) |
| `Command.hpp` | **Modified** — add `EntityCreationCommandBase`, add `CopyCommand` |
| `Transform.hpp` | **Unchanged** — reused as-is |
| `Selection.hpp` | **Unchanged** |
| New test files | `DocumentRemoveEntityTests.cpp` (or added to existing Document tests), `CopyTests.cpp` |

No changes needed to Geometry Kernel (unlike Trim/Extend, which added `ParameterOnLine`/`FindExtendIntersection`) — this is a smaller geometry footprint than either prior sprint, offset by a larger Document-layer footprint.

## Risk assessment

- **Reindexing on `RemoveEntity`:** the exact risk `TODO(Spiral5)` warned about. Must shift `index_` entries for every entity after the removed one, or accept O(n) rebuild of `index_` on every remove. At current/expected document sizes (CAD drawings, not bulk data), O(n) rebuild is acceptable — matches the project's existing "don't prematurely optimize" precedent (e.g. `FindLayer`'s linear scan). Flag for a future ADR only if profiling ever shows this hot.
- **Id-reuse safety:** `Add(EntityId, ...)` with an explicit id must **not** touch `nextId_` — it must only place the entity at that id and update `index_`. Reusing `nextId_`'s bump logic by mistake for the explicit-id path would risk future `Add()` calls colliding with a manually-specified id. This needs a regression test (see Phase 4 Test Design).

## Decision: GO

Document-layer changes are scoped, bounded, and directly justified by both COPY-001 (Redo path) and the near-term DELETE-001 (per Decision Gate, Option A). No open architectural fork remains. Proceed to Phase 3 (Architecture Design).
