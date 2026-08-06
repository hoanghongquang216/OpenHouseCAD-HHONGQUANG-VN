# DXF-EXPORT-001 — Architecture Audit

Status: Phase 2 (Architecture Audit) — complete, no code written
Decision: **GO** (scope: DG-001 resolved as Option A — see §5)

## Scope of audit

Read: `Document.hpp`/`Layer.hpp` (from prior sessions), `DxfReader.hpp` (structure
via targeted `grep`, full read of the entity-splitting/parsing sections),
`RenderDocument.hpp`, `SvgDocument.hpp` (both in full).

Structured around 4 questions, per the explicit correction to this Sprint's
Phase 2 goal: audit for what must be symmetric at the **data/format level**,
not assume symmetry at the **source-architecture level**.

## Q1 — What does Document currently store? (scopes Export)

`Entity{ id, shape, layer }`, `Shape = variant<Line2d, Circle2d, Arc2d>`,
`Layer{ name, color, lineType, lineWeight, visible, locked }` (per
DXF-LAYER-PROPS-001 and Spiral 2's Layer System).

**Export's available data surface:** every entity's geometry (3 shape kinds),
every entity's layer name, and — if DG-001 chooses to use it — every layer's
color/lineType/lineWeight. `locked` has no DXF equivalent and is irrelevant
to export (it gates editing, not data, per `Layer.hpp`'s own comment already
noted in DELETE-001's audit).

## Q2 — What does Reader currently parse? (scopes round-trip)

Confirmed via `grep` + targeted read: `ParseDxfStream` splits the token
stream into `EntityChunk`s (`detail::SplitIntoChunks`), then dispatches with
plain `if (chunk.type == "LINE") ... else if (chunk.type == "CIRCLE") ...
else if (chunk.type == "ARC") ... else if (chunk.type == "LWPOLYLINE")`.

**This is a consequence of DXF's own text structure**, not a source-level
design choice available to imitate or reject: DXF entities are identified by
a *string* value on group code `0`, so any DXF parser must branch on that
string somewhere. It says nothing about how a *writer* (which starts from a
typed `Shape` variant, not an untyped string stream) should be structured.

**Round-trip scope (data-level):** `LINE`/`CIRCLE`/`ARC` only, matching
`Document::Shape`'s closed variant exactly. `LWPOLYLINE` is out of scope for
Export as established in Phase 1 (Import already explodes it to
`Line2d`/`Arc2d`; no data survives to reconstruct one).

## Q3 — Does the project already have a Document-to-external-format serialization pattern?

**Yes — `RenderDocument.hpp`'s `RenderToSvg`.** It dispatches per-entity via
`std::visit`/`if constexpr` directly over `entity.shape` (the `Shape`
variant), calling `SvgDocument::AddLine`/`AddCircle`/`AddArc` per matched
type. This is a shipped, tested precedent for exactly this Sprint's shape
(`Document` entities out to a different format's writer type) — closer in
direction and structure to what `DxfWriter` needs than `DxfReader` is.

**Recommendation for Phase 3:** `DxfWriter` should follow `RenderToSvg`'s
`std::visit` dispatch pattern, not `DxfReader`'s string-branch structure —
consistent with this Sprint's Phase 1 revision (data symmetry, not
architecture symmetry).

**Secondary finding, new edge case for Phase 3:** `RenderToSvg` skips
entities on a hidden layer (`!layer->Visible()`) — appropriate for a visual
render, where a hidden layer shouldn't appear in the picture. Whether
`DxfWriter` should do the same is a genuine open question, not inherited by
symmetry: DXF Export exists to preserve *data* for re-import, not to produce
a *picture* — a hidden layer's entities are still real drawing data a user
would reasonably expect to survive a save/reopen cycle. Flagged for Phase 3
Design, not resolved here.

## Q4 — What does DG-001 (Option A vs B) affect?

- **`Document`/`Layer`:** neither option requires any schema change to
  either — the data Option A would write (color/lineType) already exists on
  `Layer`. This audit found no blast radius on `Document.hpp`/`Layer.hpp`
  regardless of which option Phase 3 picks.
- **Future DXF roadmap:** Option A establishes a `TABLES`/`LAYER`-writing
  precedent that DXF-LAYER-PROPS-003 (entity-level overrides, still
  deferred in `DXF_BACKLOG.md`) would extend later if ever picked up; Option
  B would defer that precedent to whenever it's eventually needed. Neither
  option blocks or unblocks DXF-LAYER-PROPS-002/003 — both stay exactly as
  deferred as they are today either way.

## Blast radius

| File | Change |
|---|---|
| `Document.hpp` | **Unchanged** |
| `Layer.hpp` | **Unchanged** |
| `DxfReader.hpp` | **Unchanged** — Export is new code, not a modification of Import |
| New: `modules/dxf/include/openhouse/dxf/DxfWriter.hpp` | New file, `std::visit`-based per Q3's recommendation |
| New test file | `DxfWriterTests.cpp` |

Smallest blast radius on existing files of any Sprint audited so far in this
project (COPY-001 touched `Document.hpp`+`Command.hpp`; DELETE-001 touched
only `Command.hpp`; this touches neither).

## §5 — Resolving DG-001

Per Phase 1's preliminary lean and this Audit's Q1/Q4 findings: Option A
(full `HEADER`+`TABLES`+`ENTITIES`) requires zero changes to `Document`/
`Layer` (Q1 confirms the data already exists) and doesn't foreclose any
future DXF work (Q4 confirms neither option affects DXF-LAYER-PROPS-002/003).
Its only real cost (per Phase 1's own comparison table) was implementation
complexity — a `TABLES` writer in addition to an `ENTITIES` writer — which is
a one-time, bounded cost, not an ongoing architectural burden.

Against that bounded cost, Option B's downside is not bounded the same way:
every `Document` that has ever used DXF-LAYER-PROPS-001-imported or
programmatically-set layer color/lineType would silently lose that data on
every export, forever, until someone later revisits this decision — a
recurring data-loss cost, not a one-time implementation cost.

**Decision: Option A.** `DxfWriter` writes `HEADER` (minimal, version only),
`TABLES`/`LAYER` (name + color + lineType, mirroring what DXF-LAYER-PROPS-001
already reads on Import), and `ENTITIES`. Target format: DXF R12, matching
Import's own scope.

## Decision: GO

Zero changes to `Document`/`Layer`/`DxfReader`. New `DxfWriter.hpp` following
the `std::visit` pattern already established by `RenderToSvg`, writing
`HEADER`+`TABLES`+`ENTITIES` (DG-001 resolved as Option A). Proceed to Phase 3
(Architecture Design).
