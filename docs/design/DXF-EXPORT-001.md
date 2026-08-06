# DXF-EXPORT-001 — Domain Research & Functional Specification

Status: Phase 1 (Domain Research) — complete
Sprint: 6 (selected via Roadmap Review — see Roadmap Review conclusion below)
Epic: 6 (DXF Export, per ROADMAP_EXECUTION.md Spiral 4)

## 0. Why this Sprint (Roadmap Review summary)

Selected over Dimension/Printing-PDF/Qt6-UI-wiring via a 4-step Roadmap Review
(Kiểm kê sự thật → Dependency Graph → ROI → Decision Readiness → Strategic
Alignment), converging on 3 independent axes: (1) ROI — direct precedent from
DXF Import already shipped, acceptable risk; (2) Decision Readiness — the
only Active Backlog item with enough precedent to start without a prior
audit; (3) Strategic Alignment — explicitly named in `ROADMAP_EXECUTION.md`'s
v0.1 Alpha criteria ("Can open and save a basic DXF file"), while GUI/Qt6 is
explicitly deferrable for that same milestone per the same document.

## 1. Reference systems and format evidence

**Minimal valid DXF (Autodesk's own documentation, "Writing a DXF File"):**
> "The entire HEADER section can be omitted if you don't set header
> variables... the entire TABLES section can be dropped if nothing in it is
> required."

A complete, minimal, entities-only file (from a DXF format reference,
quoting Autodesk's own example):
```
0
SECTION
2
ENTITIES
0
LINE
8
0
10 1.0
20 2.0
11 3.0
21 4.0
0
ENDSEC
0
EOF
```
Caveat found directly in the same research: an entities-only file is only
valid in the **DXF R12** file format specifically -- not guaranteed across
all DXF versions.

**LibreCAD (via its `libdxfrw` library):** always writes the full section
set -- HEADER, TABLES (layers, linetypes, styles), BLOCKS, ENTITIES, OBJECTS
-- on every export, regardless of what the source drawing actually uses.
This is the "complete/high-fidelity" reference implementation, not a minimal
one.

**Group codes for the 3 entities OpenHouseCAD's `Shape` variant already
supports** (confirmed against multiple DXF references):

| Entity | Group codes |
|---|---|
| Common to all | `8` = layer name |
| `LINE` | `10`/`20` = start point, `11`/`21` = end point |
| `CIRCLE` | `10`/`20` = center, `40` = radius |
| `ARC` | same as `CIRCLE`, plus `50` = start angle, `51` = end angle (**degrees**, not radians -- `Arc2d::startAngle`/`endAngle` are stored in radians per `Arc2.hpp`, so a unit conversion is needed at the export boundary) |

**Polyline:** no export-side design question exists here. DXF-002 (Import)
already explodes `LWPOLYLINE` into individual `Line2d`/`Arc2d` entities at
import time -- `Document` retains no memory that a set of entities was
originally one polyline. Export therefore naturally emits separate
`LINE`/`ARC` entities; there is no data available to reconstitute an
`LWPOLYLINE` on the way out, and reconstituting one is out of scope (would
require a new "was originally grouped" concept nowhere in the current data
model).

## 2. Functional specification (draft -- scope pending DG-001)

**Common ground, true under either option below:**
- Exports every entity currently in `Document::Entities()` as the
  corresponding `LINE`/`CIRCLE`/`ARC` DXF entity, using the group codes in
  §1's table.
- `Arc2d`'s radian angles are converted to degrees for the `50`/`51` codes.
- Output targets DXF R12 (matches the "entities-only is valid" caveat, and
  is the version DXF-002's own Import work is written against -- see that
  Sprint's own scope note).

**Everything else -- whether a `TABLES`/`LAYER` section is written, and
therefore whether layer color/linetype round-trips through Export -- is
undecided. See DG-001.**

## 3. Decision Gate — DG-001: Export Scope

**The fork:**

| | **Option A — Full section set (HEADER + TABLES + ENTITIES)** | **Option B — ENTITIES only** |
|---|---|---|
| What it does | Writes a `TABLES`/`LAYER` entry for every layer in `Document::Layers()`, carrying over color and linetype (the same properties DXF-LAYER-PROPS-001 already reads on Import) | Writes only the `ENTITIES` section; every entity's layer is referenced by name (group code `8`) but no `LAYER` table entry defines that layer's color/linetype |
| Round-trip fidelity | High -- a file this project exports and then re-imports would reproduce layer color/linetype, matching what Import already captures | Low -- layer color/linetype set in `Document` (e.g. via DXF-LAYER-PROPS-001's own import, or set programmatically) is silently dropped on export |
| Data preservation | High -- consistent with data already held in `Document`/`Layer` | Low -- actively discards fields `Document` already stores |
| Complexity | Higher -- needs a `TABLES` writer, not just an `ENTITIES` writer | Lower -- single-section writer only |
| Testability | Similar either way -- both are straightforward to golden-file/round-trip test | Slightly simpler minimal case |
| Consistency with current architecture | High -- `Document`/`Layer` already model color/linetype (per DXF-LAYER-PROPS-001, Spiral 2's Layer System); an exporter that ignores fields the internal model already carries is an asymmetry inside this project's own data model, not just relative to an external reference | Medium -- valid per the DXF spec itself (Autodesk confirms entities-only files are accepted), but not evidenced as the right choice *for this project's own model* |
| External precedent | Not LibreCAD's approach (writes full sections always) | Is a spec-valid minimal form Autodesk's own docs describe, but adopted here would be a choice not evidenced by either reference CAD system read so far |

**Preliminary lean (not a decision -- Phase 2/3's to make):** the strongest
argument for Option A is internal, not external -- `Document` already holds
layer color/linetype (shipped, tested, in active use since Spiral 2 and
DXF-LAYER-PROPS-001). An exporter that silently drops data the project's own
model already carries would be an inconsistency in OpenHouseCAD's own
architecture, independent of what any reference CAD system does. This is
noted here as an input to Phase 2/3, not a Go/No-Go conclusion -- Domain
Research's role is to lay out the fork and evidence, not resolve it.

## 4. Edge cases (draft, revisit once DG-001 resolves)

| Case | Note |
|---|---|
| Empty `Document` (no entities) | Should still produce a structurally valid DXF (`SECTION`/`ENTITIES`/`ENDSEC`/`EOF` with zero entity records) |
| Entity on a layer with a name containing DXF-special characters | Needs a check against DXF's layer-naming rules -- not yet researched, flag for Phase 2 audit |
| Round-trip: Import a file, Export it immediately, re-Import the result | The natural verification test for this Sprint regardless of DG-001's outcome -- differs only in how much survives the round-trip |
| Very large angle values / negative angles on `Arc2d` | Needs confirmation that the radian-to-degree conversion handles `Arc2d`'s existing "sweep can be negative for clockwise" convention (see `Arc2.hpp`'s own `Sweep()` comment) consistently with what DXF's `50`/`51` codes expect |

## 5. Explicitly out of scope for this Sprint

- Any entity type beyond `Line2d`/`Circle2d`/`Arc2d` (matches `Document::Shape`'s
  current closed variant -- no `LWPOLYLINE` re-composition, no `TEXT`,
  `DIMENSION`, `INSERT`, etc.)
- DXF versions other than R12 (matches Import's own existing scope)
- `BLOCKS` section / block references (no block concept exists anywhere in
  `Document` yet, same absence noted in `DXF_BACKLOG.md`'s "Explicitly not
  backlogged" section)
- Entity-level appearance overrides (group codes `62`/`6`/`370` on an entity
  itself) -- `Document::Entity` has no such field yet (see
  DXF-LAYER-PROPS-003, still deferred)
