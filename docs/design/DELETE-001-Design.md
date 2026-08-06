# DELETE-001 — Architecture Design

Status: Phase 3 (Architecture Design) — complete
Precondition: DELETE-001-Architecture-Audit.md → GO

This document defines the API contract for `DeleteCommand`. As with COPY-001's
design document, this describes required behavior, not a mandatory internal
implementation -- Phase 5 may implement it differently as long as the contracts
here hold and are tested (Phase 4).

## 1. Goals

**In scope:** `DeleteCommand(EntityId id) -> ICommand`, full Undo/Redo, works
uniformly across `Line2d`/`Circle2d`/`Arc2d` (no shape-specific logic --
`RemoveEntity`/`Restore` are shape-agnostic).

**Out of scope** (see DELETE-001.md §2 for full rationale): multi-entity delete
in one action (compose via `MacroCommand` later, no new infrastructure needed),
`DeleteSelection` free function, AutoCAD's `OOPS`, any soft-delete concept,
selection-clears-after-delete UX (Application layer).

## 2. Overall architecture

```
DeleteCommand (new, implements ICommand directly)
      │
      ├─► CanTransform(doc, id)        (Transform.hpp, reused unchanged)
      │
      ├─► Document::RemoveEntity(id)   (Document.hpp, already exists from COPY-001)
      │
      └─► Document::Restore(id, shape, layer)   (already exists from COPY-001)
```

Unlike `CopyCommand`, `DeleteCommand` does **not** subclass any shared base --
per the Sprint 4 Decision Gate (COPY-001-Design.md §4) and DELETE-001-Architecture-Audit.md's
confirmation, its Execute/Undo direction is the mirror image of
`EntityCreationCommandBase`'s, and forcing it into that base would invert the
meaning of its hooks. `DeleteCommand` is a complete, self-contained `ICommand`
implementation -- the simplest command class in the codebase, since it needs no
new abstraction at all, only composition of two already-existing Document
methods.

## 3. API Contract — DeleteCommand

```cpp
class DeleteCommand final : public ICommand {
public:
    explicit DeleteCommand(EntityId id) noexcept : id_(id) {}

    [[nodiscard]] bool Execute(Document& doc) override {
        const Entity* entity = doc.FindEntity(id_);
        if (entity == nullptr || !CanTransform(doc, id_)) {
            return false;
        }
        shape_ = entity->shape;
        layer_ = entity->layer;
        return doc.RemoveEntity(id_);
    }

    void Undo(Document& doc) override { doc.Restore(id_, shape_, layer_); }

    // Does not re-resolve or re-snapshot -- the entity is guaranteed to
    // exist at id_ (Undo just restored it there) with the same shape_/
    // layer_ already saved from Execute(). Mirrors the "don't
    // recompute, use the saved snapshot" Memento discipline used
    // throughout Command.hpp.
    void Redo(Document& doc) override { doc.RemoveEntity(id_); }

    [[nodiscard]] EntityId Id() const noexcept { return id_; }

private:
    EntityId id_;
    Shape shape_{};
    foundation::string layer_;
};
```

- **Precondition on `Execute`:** none beyond what `FindEntity`/`CanTransform`
  already check internally -- any `id_` is safe to pass, including one that
  doesn't resolve to a live entity.
- **Postcondition on successful `Execute`:** entity gone from `Document`;
  `shape_`/`layer_` hold an exact snapshot for `Undo`.
- **Postcondition on rejected `Execute`:** `Document` unchanged; `shape_`/
  `layer_` remain default-constructed (never read before a successful
  `Execute`, so their default state is never observed as meaningful).
- **`Undo`/`Redo` safety:** both are safe no-ops if called before a successful
  `Execute` -- `Restore`/`RemoveEntity` already reject a `kInvalidEntityId`-shaped
  or otherwise invalid call per their own contracts (COPY-001-Design.md §3);
  `DeleteCommand` inherits this safety for free by construction, with no
  extra guard code needed.

## 4. Ownership & Lifetime

Identical discipline to every other command in this codebase: `DeleteCommand`
never caches an `Entity*`/`Shape*` across calls, only the `EntityId` and a
value-copied snapshot. `Document` remains sole owner of every `Entity` at all
times; `Undo` reconstructs a new `Entity` object inside `Document::Restore`
(not literal object resurrection), matching the same "identity by id/shape/
layer, not pointer identity" correctness standard already established.

## 5. ID Policy

No new id policy -- `DeleteCommand` operates entirely within the id rules
`RemoveEntity`/`Restore` already established in COPY-001-Design.md §6.
`id_` is fixed at construction (unlike `EntityCreationCommandBase`'s `id_`,
which starts at `kInvalidEntityId` until a successful `Execute` assigns it) --
Delete's target is always known up front, so there is no "id not yet
assigned" state to design for.

## 6. Event Model

No change -- Section 7's conclusion from COPY-001-Design.md still holds
(no event/observer system exists; `RemoveEntity`/`Restore` remain the natural
future hook points, unaffected by this sprint).

## 7. Sequence diagrams

**Execute:**
```
Caller          DeleteCommand            Document
  │  Execute()      │                        │
  ├────────────────►│  FindEntity(id)         │
  │                  ├───────────────────────►│
  │                  │◄───────────────────────┤ entity
  │                  │  CanTransform(id)       │
  │                  ├───────────────────────►│
  │                  │◄───────────────────────┤ true
  │                  │  snapshot shape/layer   │
  │                  │  (local, no doc call)   │
  │                  │  RemoveEntity(id)       │
  │                  ├───────────────────────►│
  │                  │◄───────────────────────┤ true
  │◄─────────────────┤ true                    │
```

**Undo:**
```
Caller          DeleteCommand            Document
  │  Undo()         │  Restore(id, shape_,   │
  ├────────────────►│          layer_)       │
  │                  ├───────────────────────►│
  │                  │◄───────────────────────┤ true
  │◄─────────────────┤                        │
```

**Redo:**
```
Caller          DeleteCommand            Document
  │  Redo()         │  RemoveEntity(id)      │
  ├────────────────►├───────────────────────►│
  │                  │◄───────────────────────┤ true
  │◄─────────────────┤                        │
```

## 8. Risk & Future Reuse

| Future consumer | Fits this design? | How |
|---|---|---|
| Multi-select Delete (Application layer) | Yes, via composition | `MacroCommand` of N `DeleteCommand`s -- no new base or Document API needed |
| `OOPS`-style un-erase | Not via this class | Would be an Application-layer feature tracking "most recently executed DeleteCommand(s)" outside the normal undo stack -- out of scope, noted for whoever picks up that feature later |
| Explode (1 entity → N new entities) | Partially | Would compose one `DeleteCommand` (remove the source) with N entity-creation commands (add the pieces) via `MacroCommand` -- consistent with the note already left in COPY-001-Design.md §9, not designed further here |

No changes to `Transform.hpp`, `Selection.hpp`, `Document.hpp`, or the
Geometry Kernel required by this design -- confirms the Audit's blast-radius
table.
