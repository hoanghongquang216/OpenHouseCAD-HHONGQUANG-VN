# COPY-001 — Architecture Design

Status: Phase 3 (Architecture Design) — complete
Precondition: COPY-001-Architecture-Audit.md → GO

This document defines **API contracts and behavioral guarantees**, not a locked internal
implementation. Anything described as "implementation note" is illustrative, not binding —
Phase 5 is free to implement it differently as long as the contracts in this document hold
and are tested (Phase 4).

## 1. Goals

**In scope for COPY-001:**
- Single-entity copy: `CopyCommand(sourceId, delta) -> new EntityId`.
- Works uniformly for `Line2d`, `Circle2d`, `Arc2d` (all three `Shape` variants — no
  per-shape special-casing, since Copy reuses `TranslateEntity`'s existing
  `std::visit` dispatch).
- Full Undo/Redo, id-stable across Undo/Redo cycles.
- New Document-level API: entity removal, and entity restoration at a specific id.

**Out of scope for COPY-001** (explicitly deferred, each to its own future sprint):
- Multi-copy in one invocation (AutoCAD's `COPYMODE`) — deferred to ARRAY-001.
- Clipboard / cross-document Paste — deferred to a future PASTE sprint; this design's
  `EntityCreationCommandBase` is expected to be reusable there (Section 4), but no
  clipboard mechanism is designed now.
- Multi-entity `CopySelection` free function — mechanically trivial once `CopyCommand`
  exists (same `*Selection` loop pattern as `TranslateSelection`), but not required for
  this sprint's Go/No-Go and not designed further here.
- Block/Insert Block, Explode — `Shape` has no block concept yet; out of scope until a
  sprint specifically introduces one.
- Selection-follows-copy UX — Application-layer concern (see COPY-001.md §6); not part
  of this design.

## 2. Overall architecture

```
CopyCommand (new)                    EntityCreationCommandBase (new)
  BuildEntity(doc) override    ─────►   Execute/Undo/Redo (shared)
        │                                      │
        │ clones + translates                  │ calls
        ▼                                      ▼
   geometry::Translate()              Document::Add / RemoveEntity / Restore (Document.hpp, extended)
   (existing, reused as-is,
    via the same std::visit
    dispatch TranslateEntity uses)
```

`CopyCommand` owns the **domain logic** (what the new entity's shape/layer should be —
clone the source, translate the clone, reuse `CanTransform` for the permission gate).
`EntityCreationCommandBase` owns the **lifecycle mechanics** (create-on-Execute,
remove-on-Undo, restore-on-Redo) shared by any future command with the same shape.
`Document` owns **storage** (id assignment, indexing, removal, restoration) and knows
nothing about commands or undo — same layering already used by
Transform.hpp/EntityShapeCommandBase.

**Execution flow:**

```
Selection (Application layer, out of scope)
      │ picks sourceId + delta
      ▼
CopyCommand::Execute(doc)
      │
      ├─► BuildEntity(doc): resolve source, CanTransform gate, clone Shape, Translate clone
      │
      ▼
Document::Add(shape, layer) -> new id        [EntityCreationCommandBase, on success]
      │
      ▼
new Entity visible in doc.Entities()
```

## 3. API Contract — Document extensions

### `bool Document::RemoveEntity(EntityId id)`

- **Precondition:** none (safe to call with any id, including one that doesn't exist).
- **Postcondition (success):** the entity is no longer present in `Entities()`,
  `FindEntity(id)`/`FindEntityMutable(id)` return `nullptr` for it, `Count()` decreases
  by 1. `nextId_` is **not** modified (removed ids are never reissued to a new logical
  entity — consistent with the existing "IDs never reused" invariant documented on
  `Clear()`).
- **Postcondition (failure):** no-op; document unchanged.
- **Returns:** `true` if an entity was actually removed, `false` if `id` did not resolve
  to an existing entity — same "did this actually change anything" convention already
  used throughout `SelectionSet` and the `*Entity` transform functions.
- **Error handling:** no exceptions; a missing id is a normal, expected case (e.g. a
  double-Undo, or Undo racing a separate deletion), not an error condition — matches
  `Undo()`'s existing "silently a no-op" precedent in `EntityShapeCommandBase`.

### `bool Document::Restore(EntityId id, Shape shape, foundation::string layerName)`

Deliberately a **distinct method**, not an overload of `Add()` — see rationale below.

- **Precondition:** `id` was previously assigned by this Document (i.e. `id < nextId_`)
  and is not currently occupied by a live entity. Both are debug-time invariants
  (`OH_ASSERT`-style, matching `ScaleEntity`'s existing precedent of asserting instead of
  runtime-rejecting for a condition that should be unreachable through the public
  Command surface).
- **Postcondition (success):** the entity reappears in `Entities()`/`FindEntity(id)` with
  exactly the given `shape`/`layerName`, at exactly `id`. `nextId_` is **not** touched.
- **Returns:** `true` on success; `false` if `id` is invalid per the precondition above
  (defensive — even though this should be unreachable via `EntityCreationCommandBase`,
  a `bool` return keeps this consistent with every other mutating Document method
  instead of being the one exception that asserts/throws).
- **Why not an `Add()` overload:** `Add()`'s contract is "give me a new logical entity,
  you don't get to pick its id." An overload that accepts a caller-chosen id would
  silently change that contract for every caller, inviting misuse from ordinary
  application code (e.g. a UI bug that "restores" over a live id). A separate,
  narrowly-named method documents by construction that it is Undo/Redo machinery, not a
  general-purpose creation path — the API surface stays honest about who is allowed to
  call it.

Both methods must correctly maintain `index_` (the `EntityId -> position` map) —
*how* they do so (in-place shift, full rebuild, etc.) is an implementation detail left to
Phase 5; the audit already noted an O(n) rebuild is acceptable at expected document
sizes.

## 4. `EntityCreationCommandBase`

**Responsibility:** the shared Execute/Undo/Redo mechanics for "this command's job is to
add exactly one new entity to the Document."

```cpp
class EntityCreationCommandBase : public ICommand {
protected:
    // Subclass computes what the new entity should look like. Returns
    // std::nullopt to reject (nothing created, no undo entry pushed) --
    // same convention as EntityShapeCommandBase::DoOperation's bool.
    // The returned Entity's `id` field is ignored (Document assigns it).
    [[nodiscard]] virtual std::optional<Entity> BuildEntity(Document& doc) = 0;

public:
    [[nodiscard]] bool Execute(Document& doc) override {
        auto built = BuildEntity(doc);
        if (!built) return false;
        id_ = doc.Add(built->shape, built->layer);
        shape_ = built->shape;
        layer_ = built->layer;
        return true;
    }
    void Undo(Document& doc) override { doc.RemoveEntity(id_); }
    void Redo(Document& doc) override { doc.Restore(id_, shape_, layer_); }
    [[nodiscard]] EntityId Id() const noexcept { return id_; }

private:
    EntityId id_ = kInvalidEntityId;
    Shape shape_{};
    foundation::string layer_;
};
```

**What subclasses share:** id/shape/layer snapshot storage, the Undo→Remove and
Redo→Restore wiring, the reject-via-`nullopt` convention.

**What deliberately stays OUT of the base** (to avoid the coupling the audit flagged):
- **No knowledge of where the new shape/layer comes from.** `BuildEntity` is the only
  hook — whether that means "clone + translate an existing entity" (Copy) or, later,
  "deserialize from clipboard payload" (Paste) is entirely the subclass's business. The
  base never sees a `sourceId` or a `delta`.
- **No `CanTransform`/permission logic.** That stays in the subclass (`CopyCommand`
  calls it inside `BuildEntity`), because a future Paste command has no "source layer" to
  gate on — it would have its own, different precondition entirely.
- **No Selection interaction**, for the same reason `EntityShapeCommandBase` has none.

`CopyCommand`:

```cpp
class CopyCommand final : public EntityCreationCommandBase {
public:
    CopyCommand(EntityId sourceId, geometry::Vector2d delta) noexcept
        : sourceId_(sourceId), delta_(delta) {}

protected:
    [[nodiscard]] std::optional<Entity> BuildEntity(Document& doc) override {
        const Entity* src = doc.FindEntity(sourceId_);
        if (src == nullptr || !CanTransform(doc, sourceId_)) {
            return std::nullopt;
        }
        Shape clone = std::visit(
            [this](const auto& s) -> Shape { return geometry::Translate(s, delta_); },
            src->shape);
        return Entity{kInvalidEntityId, foundation::move(clone), src->layer};
    }

private:
    EntityId sourceId_;
    geometry::Vector2d delta_;
};
```

## 5. Ownership & Lifetime

- The new entity is owned by `Document`, exactly like every other entity — `CopyCommand`
  holds only its `EntityId` after `Execute()`, never a pointer (same "never cache a
  pointer across calls" discipline `EntityShapeCommandBase` already documents, and for
  the identical reason: `entities_` is a vector that can reallocate).
- On `Undo()`, the entity is fully removed from `Document` — it does not linger in any
  "soft-deleted" state. This matches the Memento philosophy already in place: Undo
  restores the *exact* prior Document state, not an approximation of it.
- On `Redo()`, a *new* `Entity` object is constructed inside `Document::Restore`, but it
  is indistinguishable from the original by id/shape/layer — no external code (Selection,
  a UI reference, another Command) can tell the difference, which is the actual
  correctness requirement (not literal object identity).

## 6. ID Policy

- **On creation (`Execute`):** id comes from `Document`'s existing `nextId_` counter,
  unchanged behavior from today's `Add()`.
- **On Undo:** id is freed (entity removed) but `nextId_` is *not* decremented — the id
  is never handed to a different logical entity. This is a continuation of the existing
  invariant, not a new rule.
- **On Redo:** id is *not* reallocated — `Restore()` places the entity back at its saved
  id without consulting `nextId_` at all.
- **No-collision guarantee:** since `Restore()` only ever restores an id that was
  previously issued by this same Document's `nextId_` (and is currently unoccupied by
  construction — it can only be reached via a prior `RemoveEntity` on that exact id),
  and `nextId_` never decreases, a restored id can never collide with a subsequently
  `Add()`-ed fresh id. This is the precondition asserted in Section 3 and is exactly
  what Phase 4's id-collision regression test must verify.

## 7. Event Model

No event/observer system currently exists anywhere in `Document` or `Command` (confirmed
by audit — no signal/callback/dirty-flag mechanism in any of the four audited files).
COPY-001 introduces none. If an event system is added in a future sprint, `Add`,
`RemoveEntity`, and `Restore` are the natural hook points (all three are the only places
`entities_` structurally changes) — noted here for that future sprint, not designed now.

## 8. Sequence diagrams

**Execute:**
```
Caller          CopyCommand              Document
  │  Execute()      │                        │
  ├────────────────►│                        │
  │                  │  FindEntity(sourceId)  │
  │                  ├───────────────────────►│
  │                  │◄───────────────────────┤ src
  │                  │  CanTransform(sourceId) │
  │                  ├───────────────────────►│
  │                  │◄───────────────────────┤ true
  │                  │  clone + Translate      │
  │                  │  (local, no doc call)   │
  │                  │  Add(shape, layer)      │
  │                  ├───────────────────────►│
  │                  │◄───────────────────────┤ newId
  │◄─────────────────┤ true                    │
```

**Undo:**
```
Caller          CopyCommand              Document
  │  Undo()         │                        │
  ├────────────────►│  RemoveEntity(id_)      │
  │                  ├───────────────────────►│
  │                  │◄───────────────────────┤ true
  │◄─────────────────┤                        │
```

**Redo:**
```
Caller          CopyCommand              Document
  │  Redo()         │                        │
  ├────────────────►│  Restore(id_, shape_,   │
  │                  │          layer_)       │
  │                  ├───────────────────────►│
  │                  │◄───────────────────────┤ true
  │◄─────────────────┤                        │
```

## 9. Risk & Future Reuse

| Future consumer | Fits this design? | How |
|---|---|---|
| **DELETE-001** | Shares storage mechanics, **not** the base class | Own `ICommand`, own `EntityCreationCommandBase`-*mirror* (Execute=Remove, Undo=Restore) — kept separate per Section "answer" above; revisit only if a 3rd same-direction command appears |
| **ARRAY-001** | Yes, via composition | `MacroCommand` of N `CopyCommand`s with different deltas — no new base needed |
| **PASTE** | Yes, via `EntityCreationCommandBase` | New subclass with a `BuildEntity` that deserializes from clipboard instead of cloning a source id — base class unchanged |
| **Insert Block** | No — deferred | `Shape` variant has no block type yet; needs its own design once blocks exist |
| **Explode** | No — deferred | Inverse-and-multiple shape (1 remove + N creates); would compose a removal command with N creation commands via `MacroCommand`, but not designed now |

No changes to `Transform.hpp`, `Selection.hpp`, or the Geometry Kernel are required by
this design — confirms the audit's blast-radius table.
