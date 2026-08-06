# DXF-EXPORT-001 — Architecture Design

Status: Phase 3 (Architecture Design) — complete
Precondition: DXF-EXPORT-001-Architecture-Audit.md → GO, DG-001/DG-002/DG-003 all DECIDED

This document defines `DxfWriter`'s **API contract and behavior**, not a
mandatory internal implementation -- Phase 5 may implement differently as
long as the contracts here hold and are tested (Phase 4). Per this Sprint's
own review discipline: this design reuses the *dispatch principle*
`RenderToSvg` established (visit `Shape` by type, separate serialization
from `Document` itself) -- it does not copy `SvgDocument`'s class shape,
builder API, or helpers. Where `DxfWriter`'s needs differ from SVG's, this
document says so explicitly rather than defaulting to mirroring.

## 1. Design Goals

- Round-trip fidelity for the entity/layer subset this project supports
  (`Line2d`/`Circle2d`/`Arc2d`, plus layer name/color/lineType) -- not
  round-trip for DXF in general.
- No data `Document` already models is silently discarded (per DG-001 and
  DG-002's resolutions).
- No change to `Document`/`Layer`'s existing data model (confirmed
  unnecessary by Phase 2's Q1/Q4).
- No inferred/guessed data is ever written (per DG-003 -- a color with no
  exact reverse mapping produces no group-code-62 output, not a
  best-guess one).

## 2. API Contract

```cpp
namespace openhouse::dxf {

// Writes `doc` as a DXF R12 stream: HEADER, TABLES/LAYER, ENTITIES, EOF.
// Returns true on success. The only failure mode is a stream write
// error (`out.fail()` at any point) -- there is no "malformed Document"
// case, since Document/Shape/Layer are already well-typed C++ data,
// unlike DxfReader's job of validating untrusted external text.
[[nodiscard]] bool WriteDxfStream(const document::Document& doc, std::ostream& out);

// Convenience wrapper, matching SvgDocument::WriteToFile's existing
// convention (opens the file, delegates to WriteDxfStream, returns
// false without throwing on I/O failure).
[[nodiscard]] bool WriteDxfFile(const document::Document& doc, const foundation::string& path);

}
```

**Deliberately a pair of free functions, not a class** (unlike
`SvgDocument`, which is a stateful builder a caller incrementally adds
shapes to across multiple calls). `Document` → DXF is a single, complete
transformation with no reason for a caller to interleave other content
mid-stream -- there is no equivalent of "call `AddLine` a few times, then
hand the accumulated result to something else" for this Sprint's scope.
This is exactly the kind of implementation-shape decision Q3 flagged as
NOT inherited by symmetry with `RenderToSvg`/`SvgDocument`; a class would
be speculative structure with no current caller need. Matches
`DxfReader.hpp`'s own top-level shape too, incidentally (`ParseDxfStream`
is also a free function) -- but that's a coincidence of both being
"transform the whole thing in one call," not evidence copied from Reader.

**Ownership:** `WriteDxfStream` only reads `doc`; no entity/layer state is
retained after the call returns. No lifetime concerns beyond the
stream/file argument's own normal C++ lifetime rules.

**Error handling:** stream-write failure only. No exceptions thrown by
this Sprint's code (matches `SvgDocument::WriteToFile`'s existing
no-throw convention).

## 3. Serialization Pipeline

```
WriteDxfStream(doc, out)
      │
      ├─► WriteHeader(out)              -- minimal: $ACADVER = AC1009 (R12) only
      │
      ├─► WriteLayerTable(doc, out)     -- TABLES/LAYER, one entry per doc.Layers()
      │
      ├─► WriteEntities(doc, out)       -- ENTITIES, one record per doc.Entities()
      │
      └─► out << "0\nEOF\n"
```

Each stage is a private, free `detail::` function taking `(doc, out)` or
`(layer, out)` / `(entity, out)` -- plain functions over the pipeline
stages, not object state, consistent with §2's "no builder class" decision.

## 4. Entity Serialization

### Layer table entries (`WriteLayerTable`)

For each `Layer` in `doc.Layers()`:
```
0
LAYER
2
<layer name>
70
0
62
<ACI, per DG-003 -- OMITTED if no exact reverse mapping exists>
6
<linetype name, per the table below>
```

**Color (group code `62`), per DG-003:** a fixed reverse table, the exact
inverse of `DxfReader.hpp`'s `AciToSvgColor`:

| `Layer::Color()` string | ACI written |
|---|---|
| `"red"` | 1 |
| `"yellow"` | 2 |
| `"#00ff00"` | 3 |
| `"cyan"` | 4 |
| `"blue"` | 5 |
| `"magenta"` | 6 |
| `"black"` | 7 |
| `"#414141"` | 8 |
| `"#808080"` | 9 |
| any other string | *(group code `62` omitted entirely)* |

When no exact reverse mapping exists for the layer's stored color, no
inferred ACI data is generated. The writer omits group code `62` for that
layer, relying on the DXF-standard default this Sprint's Phase 2/3
research confirmed (an omitted `62` on a `LAYER` entry defaults to ACI 7)
-- not on a guess. This is a deliberate absence of output, not a fallback
value chosen by this project.

**Linetype (group code `6`):** the exact inverse of `DxfReader.hpp`'s
`LineTypeNameToEnum` (a closed, unambiguous mapping -- unlike color, no
information loss is possible here, since `LineType` is this project's own
enum with an unambiguous canonical name per value):

| `Layer::GetLineType()` | linetype name written |
|---|---|
| `Continuous` | `"CONTINUOUS"` |
| `Dashed` | `"DASHED"` |
| `Dotted` | `"DOTTED"` |
| `DashDot` | `"DASHDOT"` |

### Entities (`WriteEntities`)

Dispatches per entity via `std::visit`/`if constexpr` over `entity.shape`
-- reusing `RenderToSvg`'s *dispatch principle*, applied to this Sprint's
own three write functions (`WriteLine`/`WriteCircle`/`WriteArc`), which
have no relationship to `SvgDocument::AddLine`/`AddCircle`/`AddArc` beyond
sharing the same three type names being matched.

Per Phase 1's confirmed group codes:

| Entity | Group codes written |
|---|---|
| Common | `8` = `entity.layer` |
| `LINE` | `10`/`20` = `start.x`/`start.y`, `11`/`21` = `end.x`/`end.y` |
| `CIRCLE` | `10`/`20` = `center.x`/`center.y`, `40` = `radius` |
| `ARC` | same as `CIRCLE`, plus `50`/`51` = `startAngle`/`endAngle` **converted from radians to degrees** (`Arc2d` stores radians per `Arc2.hpp`; DXF's `50`/`51` are degrees) |

Per DG-002: **every** entity is written, regardless of its layer's
`Visible()` state -- no filtering loop, unlike `RenderToSvg`'s two-pass
visible/selected split. `WriteEntities` is a single pass over
`doc.Entities()`.

Per Phase 2's Q4/blast-radius finding and this Sprint's own scope: no
group code `62`/`6`/`370` is written on an entity itself (entity-level
overrides remain deferred, per DXF-LAYER-PROPS-003) -- every entity relies
on its layer's appearance (DXF's own BYLAYER default), matching how
`Entity` has no such field to read from in the first place.

## 5. Design Decisions realized

- **DG-001 (Option A):** §3's pipeline includes `WriteHeader`/
  `WriteLayerTable` alongside `WriteEntities` -- the full section set.
- **DG-002 (Option A):** §4's `WriteEntities` has no visibility filter.
- **DG-003 (Option A):** §4's color table is exact-match-or-omit, no
  heuristic.

## 6. Extension Points

Recorded for future Sprints, not designed further here:

- **New entity type:** add one more `if constexpr` branch in
  `WriteEntities`'s dispatch, once `Document::Shape`'s variant itself
  grows a new alternative (that's the actual prerequisite -- this
  function has nothing to extend until the data model does).
- **New TABLES entries** (e.g. `LTYPE`, `STYLE`): add another
  `detail::WriteXTable(doc, out)` function called from `WriteDxfStream`,
  same shape as `WriteLayerTable`.
- **Other DXF versions:** `WriteHeader`'s `$ACADVER` value is the only
  version-specific literal in this design; supporting another version
  would mean parameterizing that (and auditing whether any group codes
  used here differ across versions -- not assumed to be version-stable
  by this document).
- **Blocks/Insert:** no extension point exists yet, matching
  `Document`'s own total absence of a block concept (same conclusion
  `DXF_BACKLOG.md` already recorded).

## Non-Goals

Explicitly out of scope for this Sprint (not deferred-with-a-plan, just
not part of what "DXF Export" means here):

- Polyline reconstruction (`LWPOLYLINE` re-composition from adjacent
  `Line2d`/`Arc2d` entities) -- no data exists in `Document` to detect
  which entities were originally one polyline (Phase 1 §1).
- Dimension, Text, Hatch, or any entity type outside
  `Document::Shape`'s current variant.
- Block/Insert export.
- Any DXF version other than R12.
- File-size optimization (e.g. omitting default-valued group codes the
  way `DXFOUT` itself does) -- correctness first, minimality is not a
  goal of this Sprint.
- Nearest-color color approximation (DG-003, Option B, rejected).
- True-color (`420`) export (DG-003, Option C, out of scope --
  pre-existing project decision per `DXF_BACKLOG.md`).
- Entity-level appearance overrides (`62`/`6`/`370` on an entity itself)
  -- still deferred by DXF-LAYER-PROPS-003.
