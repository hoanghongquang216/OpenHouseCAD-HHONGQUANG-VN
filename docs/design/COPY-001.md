# COPY-001 — Domain Research & Functional Specification

Status: Phase 1 (Domain Research) — complete
Sprint: 4
Epic: 4 (Editing)

## 1. Reference systems

| System | Command name | Core mechanic |
|---|---|---|
| AutoCAD / BricsCAD | `COPY` | Select entities → base point → destination point (or displacement vector). `COPYMODE` system variable controls single vs. multiple copies per invocation. Original entities are untouched; new entities are added at `displacement = destination - base`. |
| LibreCAD | `Move/Copy` (shared tool, "keep original" toggle) and `Duplicate` (offset = 0, copy in place) | Same displacement math as Move; the only difference from Move is whether the source entity is kept. |
| QCAD | `Copy` / `Copy (Array)` | Same base/destination model; array variant repeats the displacement N times. |

**Universal finding:** Copy is not a distinct geometric operation. It is **Translate, applied to a duplicate instead of the original**. Every reference system reuses their Move/Translate math for Copy — none reimplement displacement logic separately. This confirms the intuition from Sprint 4's audit: COPY-001 should compose `Document::Add()` (duplicate) with the existing `TranslateEntity()` (Sprint 4/Transform.hpp), not introduce new geometry.

`COPYMODE` (multi-copy per invocation) and QCAD's Array variant are explicitly **out of scope** — see ARRAY-001, already on the proposed roadmap as its own sprint.

## 2. Functional specification (OpenHouseCAD COPY-001 scope)

**Input:** one `EntityId` (single-entity copy — matches TRIM-001/EXTEND-001's precedent of shipping the single-entity case first; `CopySelection` follows the same `*Selection` free-function convention as `TranslateSelection`/`RotateSelection`/`ScaleSelection`, but is not required for this sprint's Go/No-Go), plus a `Vector2d delta` (displacement — same type `TranslateEntity` already takes, no new "base point / destination point" abstraction needed at this layer; that UX-level base/destination interaction belongs to the future Application layer).

**Output:** a new `EntityId` for the duplicate, or `kInvalidEntityId` on rejection.

**Behavior:**
1. Resolve source entity by id. If it doesn't exist → reject (return `kInvalidEntityId`, no-op).
2. Layer permission check — reuse `CanTransform()` (Transform.hpp) as-is: source layer must exist, be visible, and not be locked. This matches AutoCAD/LibreCAD/QCAD, all of which refuse to operate on locked-layer entities. (Note: this gates whether the *source* can be read/copied from, mirroring how Trim/Extend already gate their target/boundary entities — not a new rule.)
3. Clone the source entity's `Shape` (cheap value copy — `Shape` is a `variant<Line2d, Circle2d, Arc2d>`, matching the Memento design rationale already established in Command.hpp).
4. Apply `delta` to the clone via the *same* `geometry::Translate` dispatch `TranslateEntity` uses — not hand-rolled — so Copy's displacement math can never drift from Move's.
5. Add the translated clone to the Document on the **same layer** as the source (AutoCAD/LibreCAD default; no reference system defaults a copy to a different layer).
6. Return the new entity's id.

## 3. State machine

```
[Idle]
   │ CopyCommand::Execute(sourceId, delta)
   ▼
[Resolve source] ──(not found)──► [Rejected: no-op] ──► [Idle]
   │ found
   ▼
[Check CanTransform(source)] ──(locked/hidden)──► [Rejected: no-op] ──► [Idle]
   │ ok
   ▼
[Clone Shape] → [Translate clone by delta] → [Document::Add(clone, sourceLayer, newId)]
   │
   ▼
[Executed] ──Undo()──► [Document::RemoveEntity(newId)] ──► [Idle, ready for Redo]
   │                                                             │
   └────────────────────────Redo()─────────────────────────────►┘
        Document::Add(savedShape, savedLayer, savedId)  ← same id, no new allocation
```

Note the Redo branch re-adds with the **saved** id (via the Option-A `Add()`-with-explicit-id overload) rather than calling `Execute()` again — same drift-avoidance rule the Memento design already applies to Undo/Redo elsewhere in Command.hpp.

## 4. Edge cases

| Case | Expected behavior | Precedent |
|---|---|---|
| Source entity doesn't exist | Reject, no entity created, no undo entry pushed | Matches `TranslateEntity`'s `false` return on missing entity |
| Source on locked layer | Reject | Matches `CanTransform` gate already used by Translate/Rotate/Scale/Trim/Extend |
| Source on hidden layer | Reject | Same `CanTransform` gate (Visible=false already blocks Transform today) |
| `delta = (0, 0)` | Succeeds — creates an exact duplicate at the same position | Matches LibreCAD's `Duplicate` (offset-zero copy is a legitimate, named use case, not an error) |
| Copy of a `Circle2`/`Arc2` (not just `Line2`) | Must work — unlike Trim/Extend which scoped to Line↔Line only, Copy's clone+translate has no shape-specific logic, so all three `Shape` variants are in scope by construction | N/A — this is a consequence of Copy reusing `std::visit`-based `Translate`, which already handles all three |
| Undo immediately after Execute | Entity fully removed, `Document::Count()` back to pre-copy value | New `RemoveEntity` (Option A) |
| Redo after Undo | Entity reappears with the *same* `EntityId` it had before Undo | `Add()`-with-explicit-id overload (Option A) |
| Multiple Copy operations, then Undo all, then Redo all | Each entity's id round-trips correctly, order of `Document::Entities()` is restored (append-order is re-created in the same sequence Redo is called, matching Undo/Redo's existing LIFO/FIFO command-stack contract) | Depends on Undo history being processed in the correct order — a Command/History-stack property, not something CopyCommand itself must special-case |

## 5. Undo/Redo rules

- Undo/Redo remain the Document's/history stack's responsibility to sequence — `CopyCommand` itself only needs to correctly reverse **its own** single Add.
- Consistent with the existing Memento principle: Undo does not "compute the inverse" (e.g. it does not re-derive and negate `delta`) — it directly removes the created entity. Redo does not re-run `Execute()`'s clone+translate — it directly re-adds the saved `(id, shape, layer)` tuple. Both avoid any recomputation, matching the drift-avoidance rationale already documented in Command.hpp for Translate/Rotate/Scale.

## 6. Selection-after-copy rule

AutoCAD, BricsCAD, and QCAD all move the **selection** to the newly created copy after a Copy operation (not the original) — this lets an immediate follow-up operation (e.g. another Move) act on the copy, matching the common "copy, then nudge into place" workflow. LibreCAD's behavior is the same for its Move/Copy tool.

**Decision for OpenHouseCAD:** `CopyCommand` itself does **not** touch `SelectionSet` — Selection is documented as deliberately independent of Document/Command (see Selection.hpp's own header comment: "per-session UI/interaction state"). Any selection-follows-copy behavior belongs to the **Application layer** (which already owns SelectionSet manipulation), calling `selection.Clear(); selection.Select(newId);` after a successful `CopyCommand::Execute()`. This keeps `CopyCommand` layer-pure, consistent with how Trim/Extend never touched Selection either. Documented here so the future Application-layer sprint doesn't have to rediscover this.
