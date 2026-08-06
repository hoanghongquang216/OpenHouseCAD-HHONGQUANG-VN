# DXF-EXPORT-001 — Test Design

Status: Phase 4 (Test Design) — complete, no implementation code written
Precondition: DXF-EXPORT-001-Design.md → API contract locked

This Sprint introduces a category prior editing Sprints (COPY-001/DELETE-001)
didn't need: **Round-trip Tests (R-series)** -- Import → Export → Import,
since fidelity across that cycle is this Sprint's actual purpose (per Phase
1 §0's Strategic Alignment finding).

## 1. Functional Tests (`DxfWriterTests.cpp`)

| ID | Test | Expected result |
|---|---|---|
| F-001 | Write an empty `Document` | Output is a structurally valid DXF: `SECTION`/`ENTITIES`/`ENDSEC`/`EOF` present, zero entity records between them |
| F-002 | Write a single `Line2d` | Output contains one `LINE` record with correct `10`/`20`/`11`/`21` values matching the source entity exactly |
| F-003 | Write a single `Circle2d` | Output contains one `CIRCLE` record with correct `10`/`20`/`40` |
| F-004 | Write a single `Arc2d` | Output contains one `ARC` record with correct `10`/`20`/`40`, and `50`/`51` correctly converted from the source's radians to degrees |
| F-005 | Write entities on two different layers | Each entity's `8` value matches its own `entity.layer`, not a shared/default value |
| F-006 | Write a `Document` with layers `"0"`, `"Walls"`, `"Doors"` | `TABLES`/`LAYER` contains exactly 3 entries, one per `doc.Layers()`, including `"0"` (no special-casing that skips the default layer) |
| F-007 | Layer color is one of the 9 `AciToSvgColor`-known strings (e.g. `"red"`) | That layer's `LAYER` entry contains group code `62` with the exact corresponding ACI (`1` for `"red"`) |
| F-008 | Layer color is `"black"` (the `Layer` default) | `62` = `7`, per DG-003's table |
| F-009 | Layer color is a string outside the known 9 (e.g. `"#3a7bd5"`, set via `SetColor` directly, not via Import) | `62` is **absent** from that layer's entry entirely -- not a fallback value, an omission |
| F-010 | Layer linetype is each of `Continuous`/`Dashed`/`Dotted`/`DashDot` in turn | `6` = `"CONTINUOUS"`/`"DASHED"`/`"DOTTED"`/`"DASHDOT"` respectively, exact match, no ambiguity (closed enum, unlike color) |
| F-011 | Entity on a layer with `Visible() == false` | Entity still appears in the `ENTITIES` section (per DG-002 -- no visibility filter) |
| F-012 | `WriteDxfFile` to an unwritable path (e.g. a directory that doesn't exist) | Returns `false`, no exception thrown |
| F-013 | `WriteDxfStream`/`WriteDxfFile` on a normal, writable target | Returns `true` |

## 2. Round-trip Tests (`DxfRoundTripTests.cpp`)

Each test: build a `Document` in memory (or `ParseDxfStream` a hand-written
DXF), `WriteDxfStream` it out, `ParseDxfStream` the result back in, compare.

| ID | Test | Expected result |
|---|---|---|
| R-001 | `Document` with one of each supported shape (`Line2d`, `Circle2d`, `Arc2d`) → export → re-import | Re-imported `Document::Count()` matches; each shape's geometry matches the original within floating-point round-trip tolerance (DXF text formatting introduces no meaningful precision loss for `double` at reasonable coordinate magnitudes) |
| R-002 | Entities on 2 different named layers → export → re-import | Each re-imported entity's `layer` field matches its original layer name |
| R-003 | A layer with a known-mappable color (e.g. imported originally from ACI `3`) → export → re-import | Re-imported layer's color string is bit-identical to the original (`AciToSvgColor(3)` both times) -- confirms DG-003's Option A is a true bijection for the 9 known entries, not just "close" |
| R-004 | A layer with an unknown color (`SetColor("#3a7bd5")`, never touched by Import) → export → re-import | Re-imported layer's color reverts to `Layer`'s own default (`"black"`) -- this is the **documented, accepted** lossy edge from DG-003, not a bug; the test exists to pin this exact, known behavior so a future change to it is deliberate, not accidental |
| R-005 | A layer with each `LineType` value → export → re-import | Re-imported `LineType` is bit-identical to the original for all 4 values -- lossless, since this mapping is a closed bijection unlike color |
| R-006 | An `Arc2d` with a negative sweep (clockwise, per `Arc2.hpp`'s own convention) → export → re-import | Sweep direction survives the radian→degree→radian round-trip; re-imported `startAngle`/`endAngle` reproduce the original sweep sign, not just its magnitude |

## 3. Error & Edge Cases

| ID | Case | Expected result |
|---|---|---|
| E-001 | Layer name containing characters DXF treats specially (not yet researched -- flagged in Phase 1 §4) | **Deferred, not tested this Sprint** -- Phase 1 explicitly flagged this as unresearched; testing it now would mean inventing untested behavior. If a real layer-naming need surfaces, it gets its own audit before a test is written against it, same discipline as A2 Deferred-by-Design items elsewhere in this project |
| E-002 | Very large positive/negative `Arc2d` angles (beyond one full turn) | Degree conversion is a pure linear scale (`radians * 180/pi`) -- no wraparound/normalization is applied (matches `Arc2.hpp`'s own documented "not clamped/normalized" stance on `Sweep()`), so this is expected arithmetic behavior, not an edge case requiring special handling; one test confirms the raw conversion is exact, not that any normalization occurs |

## 4. Regression Tests (existing suites — must stay green, unmodified)

| Suite | Must still pass |
|---|---|
| `OpenHouseDxfReaderTests` | Unchanged -- Export is new code, `DxfReader.hpp` is not modified |
| `OpenHouseSvgDocumentTests`, `OpenHouseRenderDocumentTests` | Unchanged -- confirms `DxfWriter` genuinely doesn't depend on or alter `SvgDocument`/`RenderToSvg`, only reuses their dispatch *principle* per Design §2 |
| `OpenHouseSvgPipelineIntegrationTests` | Unchanged |
| `OpenHouseDocumentTests`, `OpenHouseLayerTests` | Unchanged -- confirms no `Document`/`Layer` schema drift, per Phase 2's Q1/Q4 finding |

## 5. Sprint completion criteria

Same standard as COPY-001/DELETE-001: all tests in Sections 1–3 pass, all
Section 4 regression suites pass unmodified, CI green, and any new
`DESIGN_DEBT.md` entry (e.g. if E-001's layer-naming question is judged
worth tracking as a deferred item) is a conscious Phase 6 Review decision,
not something merged silently. Given R-004 documents an intentionally lossy
round-trip case, Phase 6 Review should explicitly confirm this lossiness is
still judged acceptable against DG-003's own reasoning, not silently
carried forward.
