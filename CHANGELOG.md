# Changelog

All notable changes to OpenHouseCAD are documented here.

Format is loosely based on [Keep a Changelog](https://keepachangelog.com/);
versioning follows the Spiral milestones in `docs/ROADMAP_EXECUTION.md`
rather than semantic versioning, since the project has no public API
consumers yet.

## [Unreleased]

Nothing yet.

## [snap-core-001] - Snap query: Endpoint, Midpoint, Center

First CAD-core Sprint after the DXF-003/ROBUST-002/003a/LAYER-PROPS-001
sequence. Closes the Snap gap found by the dedicated Snap audit
(Spiral 6, previously pure comment-only placeholders, no code).
Deliberately Endpoint/Midpoint/Center only -- see
`docs/SNAP_BACKLOG.md`'s `SNAP-INTERSECTION-001` for why Intersection
is its own, later Sprint. No UI/Qt/cursor/marker/rubber-band -- pure
query API only, mirroring how `document::HitTest()` was itself built
and shipped well before any interactive UI exists.

### Added
- `geometry::Midpoint(const Arc2<T>&)` (`Arc2.hpp`) -- the point at an
  arc's angular midpoint (halfway between `startAngle` and `endAngle`
  along the sweep), matching Snap's Midpoint semantics for an arc. Not
  the chord midpoint between `StartPoint`/`EndPoint` -- that would cut
  inside the arc for anything but a semicircle. `Line2` already had its
  own `Midpoint()`; `Arc2` didn't, until now.
- `document::SnapType` (`Snap.hpp`, new file) -- `Endpoint`, `Midpoint`,
  `Center`. Included as a real enum from the start (unlike
  `DxfErrorCode`, deferred in `DXF-ROBUST-003b` for lack of a
  consumer) because distinguishing snap kind is the feature itself, not
  an internal detail -- a future UI needs it to show a different marker
  per kind.
- `document::SnapResult` -- entity id, `SnapType`, the matched point,
  and the distance from the query point (same "keep distance, avoid
  recomputing" reasoning as `document::HitResult`).
- `document::FindSnapPoint(doc, point, tolerance)` -- the query itself.
  Mirrors `document::HitTest()`'s exact shape (`Document + Point2 +
  tolerance -> single nearest result`), including its behavior on
  hidden layers (excluded) and locked layers (NOT excluded -- Locked
  means "cannot be edited," not "cannot be queried," same reasoning as
  `HitTest`'s own comment on this).
- 12 regression tests in `SnapTests.cpp`, including a dedicated
  regression guard (`TestSnapFindsArcCenterFarOutsideArcsOwnBoundingBox`)
  for a design decision made during this Sprint -- see "Design decision"
  below.

### Design decision: no AABB-dilate fast-reject (unlike `HitTest`)
`document::HitTest()` dilates each shape's `Bounds()` by the tolerance
and cheaply rejects a query point outside that box before running the
more expensive `DistanceToShape()`. `FindSnapPoint()` deliberately does
**not** do this, and it would have been a real bug, not just a missed
optimization, to copy the pattern: an `Arc2`'s `Center` candidate can
sit well outside the arc's own `Bounds()` (which covers only the drawn
curve) -- e.g. a short arc segment far around its circle from its own
center. Filtering by the curve's bounding box first would silently
discard an in-tolerance `Center` candidate whenever the query point was
near the center but far from the swept curve itself. Snap's per-entity
work is also already cheap (a handful of point-to-point distance
checks, not `HitTest`'s projection math), so the optimization had less
to offer here even before that correctness problem. Found and avoided
during this Sprint's own design pass, not discovered as a bug later --
see `Snap.hpp`'s own comment and the dedicated regression test.

### Docs
- `docs/SNAP_BACKLOG.md` (new file) -- `SNAP-INTERSECTION-001` entry,
  plus a note that this is now the *second* module-specific backlog
  file, which is itself a documentation-organization question
  (`backlog/` subdirectory?) left open rather than decided mid-Sprint.

## [dxf-layer-props-001] - Import layer color and linetype from TABLES/LAYER

Closes the gap found by the DXF-004 appearance audit: `document::Layer`
color/linetype storage and `RenderToSvg` rendering of both have been
complete since Spiral 2 (`DOC-003`); only DXF import was missing.

### Added
- `detail::ApplyLayerTableProperties()` in `DxfReader.hpp`: parses the
  (optional) `TABLES`/`LAYER` section and applies each record's color
  (group code `62`, ACI) and linetype (group code `6`, name) to the
  matching `Document` layer via the existing `Document::CreateLayer` +
  `Layer::SetColor`/`SetLineType` -- no `Document`/`Layer` API change.
  Runs as an independent pass after `ENTITIES` is parsed; a missing,
  malformed, or unclosed `TABLES` section is not an error -- layers
  simply keep their existing default appearance, same as before this
  Sprint.
- `detail::AciToSvgColor()`: maps DXF's ACI (AutoCAD Color Index) 1-9
  to an SVG color string; anything outside that range is left
  unmapped (existing default color kept), not guessed at. A layer's
  color value being negative (DXF's "this layer is off" encoding) is
  handled by taking the magnitude for the color lookup -- visibility
  import itself is out of scope (see `DXF-LAYER-PROPS-002` in
  `docs/DXF_BACKLOG.md`).
- `detail::LineTypeNameToEnum()`: maps a DXF linetype name (free-form,
  often with a numeric scale suffix like `"CENTER2"`) onto the
  existing small `LineType` enum by case-insensitive substring match;
  unrecognized names default to `Continuous`.
- `detail::SplitIntoChunks()`: the entity-chunk-splitting loop
  previously inlined in `ParseDxfStream`'s Pass 2, extracted into a
  shared helper now also used for `LAYER`-table-record splitting
  (identical DXF convention -- a bare group code `0` starts a new
  record -- applied to a second section). No behavior change to the
  existing `ENTITIES` splitting.
- 8 regression tests in `DxfReaderTests.cpp`: color+linetype import
  (including a scale-suffixed linetype name), the negative-color/
  magnitude case, absent-`TABLES` and `TABLES`-without-`LAYER`-table
  backward-compatibility guards, unrecognized ACI/linetype falling
  back to defaults, a `LAYER` record for a layer no entity references,
  and an unclosed `TABLES` section not leaking into (or being confused
  with) `ENTITIES`'s own section boundary.

### Fixed (found during this Sprint's own verification, not a
pre-existing bug report)
- The `TABLES` section's own closing-boundary search could, for a
  malformed/never-closing `TABLES` section, mistake a *later* and
  unrelated section's `ENDSEC` (e.g. `ENTITIES`'s) for `TABLES`'s own.
  Harmless in practice (only genuine `LAYER`-type chunks are ever
  applied), but tightened anyway: the search now stops at the next
  `0/SECTION` it encounters, correctly falling through to "`TABLES`
  doesn't close" instead.

### Explicitly not done this Sprint (see `docs/DXF_BACKLOG.md`)
Layer visibility, lineweight (needs a unit-mapping decision --
`DXF-LAYER-PROPS-002`), entity-level color/linetype override (needs an
`Entity` data-model change -- `DXF-LAYER-PROPS-003`), BYBLOCK, true
color, plot style.

## [dxf-robust-003a] - Tokenizer::Good() actually distinguishes clean EOF from malformed/truncated data

Scoped-down first half of DXF-ROBUST-003 (see design discussion):
internal EOF-vs-truncation distinction only, no public API change, no
error-code taxonomy, no position/section tracking. The larger
diagnostics work (line/section-aware messages, a structured
`DxfErrorCode`) is deferred to `DXF-ROBUST-003b` in
`docs/DXF_BACKLOG.md`, pending a concrete consumer that needs it.

### Fixed
- `Tokenizer::Good()` was implemented as `in_->eof() || in_->good()`,
  which is `true` after *both* a clean end-of-stream and a truncated
  mid-pair failure (a failed `getline()` sets `eofbit` either way) --
  so it could never actually tell the two apart, contradicting its own
  declaration comment. `Tokenizer` now tracks a small internal flag
  set the moment `Next()` hits a real problem (a code line with no
  following value line, or a malformed group code that isn't a bare
  integer) -- left untouched on a genuine clean end-of-stream.
- `ParseDxfStream` now consults this to pick a more accurate message
  when it can't find what it needs: "DXF stream ended unexpectedly
  (malformed group code or truncated data) ..." when tokenization
  itself broke down, vs. the original "No ENTITIES section found" /
  "ENTITIES section is missing its closing '0/ENDSEC'" when
  tokenization completed cleanly and the file is just genuinely
  missing what it claims to have. Malformed/truncated data in a
  section other than ENTITIES (encountered *after* ENTITIES's own
  ENDSEC was already captured) still does not fail the parse --
  unchanged, consistent with this Spiral's documented scope of
  ignoring everything outside ENTITIES.

### Added
- 4 regression tests in `DxfReaderTests.cpp`: a malformed group code
  mid-ENTITIES, a genuinely truncated mid-pair file, a cleanly-
  tokenized-but-missing-ENDSEC file (confirms the *original* message
  is kept when nothing is actually malformed -- the case that proves
  the distinction is real), and malformed data in a later, ignored
  section not failing the parse (guards against the leniency
  regression this fix could have introduced if scoped carelessly).

## [dxf-robust-002] - Strict numeric parsing in DxfReader

### Fixed
- `detail::FindDouble()`, the group-code integer parse in
  `Tokenizer::Next()`, and the X/Y/bulge parsing in
  `detail::ExtractLwPolylineVertices()` all previously accepted a
  numeric field with trailing garbage (e.g. `"5.0abc"`) by silently
  truncate-accepting up to the first unparseable character
  (`std::stod`/`std::stoi` only checked that *something* was parsed,
  not that the *whole* trimmed value was consumed). A corrupted or
  accidentally-concatenated numeric field would import as a
  plausible-looking but wrong number instead of failing the parse.
  Now rejected the same way an unparseable value already was: a
  required field with trailing garbage fails the entity (LINE/CIRCLE/
  ARC) or is skipped (LWPOLYLINE X/Y, consistent with existing
  DXF-002 per-entity-skip behavior); a malformed *bulge* specifically
  keeps its existing lenient fallback to a straight segment, now
  including the trailing-garbage case.
- Added `detail::ParseStrictDouble()` -- a single shared helper for
  the ≥3 existing call sites that all needed this same "must consume
  the entire string" check (`FindDouble`, and the X/Y/bulge parses in
  `ExtractLwPolylineVertices`), rather than repeating the check three
  times. Not a new architectural layer -- see `docs/AI-Working-
  Agreement.md` rule 3, whose own bar ("at least one concrete,
  currently-existing use case") this clears with room to spare.

### Added
- 6 regression tests in `DxfReaderTests.cpp` covering: a required
  field (LINE coordinate, CIRCLE radius) with trailing garbage being
  rejected; a group-code line with trailing garbage being rejected; an
  LWPOLYLINE vertex X/Y with trailing garbage skipping the entity
  (not the whole file); a malformed bulge still falling back to a
  straight segment; and legitimate signed/scientific-notation values
  (`"+1.5e2"`) still parsing correctly (guards against over-rejection).

Was `DXF-ROBUST-002` in `docs/DXF_BACKLOG.md`; entry removed from the
backlog now that it's shipped (see that file's own convention on
where deferred-vs-shipped items live).

## [dxf-003] - Tokenizer: don't let a blank line silently truncate the file

### Fixed
- `Tokenizer::Next()` treated any blank (or whitespace-only) code line
  as end-of-stream. A group code is never legitimately blank, but a
  stray blank line is something real-world DXF files can pick up
  (line-ending conversion, manual edits, some non-CAD export paths).
  Depending on where in the file it landed, this produced either a
  misleading `"ENTITIES section is missing its closing '0/ENDSEC'"`
  error on an otherwise well-formed file, or -- worse -- a
  successful-looking parse that had silently dropped entities after
  the blank line. `Next()` now skips blank code lines and keeps
  reading; only a genuine `getline()` failure (true end-of-stream) is
  treated as end-of-stream.

### Added
- 3 regression tests in `DxfReaderTests.cpp`: a blank line between two
  entities (both must still import), several consecutive blank lines,
  and trailing blank lines after the file's own `EOF` marker (must
  still resolve cleanly, no false regression from the fix).

### Also
- Logged two related-but-out-of-scope findings from the same audit to
  `docs/DXF_BACKLOG.md` rather than bundling them into this patch:
  `DXF-ROBUST-002` (shipped above) and `DXF-ROBUST-003` (numeric
  trailing-garbage acceptance, and `Tokenizer::Good()`'s EOF-vs-
  truncation contract not actually holding as implemented).

## [svg-pipeline] - DXF → Document → SVG integration tests

### Added
- `modules/dxf/tests/SvgPipelineIntegrationTests.cpp` -- 10 test cases
  proving the DXF -> Document -> SVG pipeline composes end to end
  through a real `Document`, not just that each stage passes its own
  unit tests in isolation (`DxfReaderTests.cpp` for DXF -> Document,
  `RenderDocumentTests.cpp` for Document -> SVG). Covers LINE/CIRCLE/
  ARC/LWPOLYLINE-with-bulge rendering, an unsupported entity (TEXT)
  being skipped end to end, an empty DXF producing an empty SVG shell,
  a malformed DXF stopping before any render is attempted, post-import
  layer styling (color/visibility) applied correctly, and a
  multi-layer floor-plan-shaped scenario combining all of the above.

### Changed (design)
- The originally proposed approach (a `tests/data/dxf/` fixture
  directory of ~16 hand-written `.dxf` files, a matching
  `tests/expected/svg/` directory of golden `.svg` files compared
  byte-for-byte, and GoogleTest) was revised during implementation
  review. Review against the existing codebase found it duplicated
  coverage already present in `DxfReaderTests.cpp` (inline DXF via
  `std::istringstream`) and introduced a golden-file comparison
  pattern with no precedent anywhere in the project, which itself uses
  `OH_CHECK` + plain `main()`, not GoogleTest. See Principle 16 in
  `docs/ENGINEERING_PRINCIPLES.md`.

## [dxf-002] - LWPOLYLINE support

Extends the existing `DxfReader.hpp` (no new file, no new class --
see `docs/AI-Working-Agreement.md` rule 3 on not building abstraction
ahead of a second real need) to decompose LWPOLYLINE entities into
`Line2`/`Arc2` segments, matching `Shape`'s existing closed variant
rather than introducing a new entity type.

### Added
- Bulge-to-arc conversion (`detail::BulgeToArc`): a curved LWPOLYLINE
  segment (non-zero bulge) becomes an `Arc2`; a straight segment
  (bulge == 0) becomes a `Line2`. The bulge formula's sign handling
  (which side of the chord the arc's center lands on, for CW vs CCW)
  was independently verified in Python against 20+ randomized
  `(point, point, bulge)` triples -- by reconstructing the original
  two points from the resulting `Arc2` and confirming they match --
  before being ported into `DxfReader.hpp`. See
  `docs/AI-Working-Agreement.md` rule 2 for why this verification step
  is mandatory for this class of formula, not optional.
- `closed` (group code 70, bit 1) support: when set, an extra segment
  connects the last vertex back to the first, using the last vertex's
  own bulge -- per DXF's documented convention, with no special-cased
  exception (see design review: exceptions here would only make the
  parser harder to reconcile against real files from AutoCAD/
  LibreCAD/QCAD).

### Design decisions
- A degenerate entity (fewer than 2 vertices after parsing, or a
  vertex whose X/Y coordinate could not be parsed at all) is SKIPPED,
  not treated as a file-level parse error -- consistent with how an
  unrecognized entity type is already handled. Only structural
  failures (a missing `ENTITIES` section, an unclosed section) return
  an error via `expected`. A malformed *bulge* value specifically is
  treated even more leniently: it falls back to 0 (a straight
  segment) rather than discarding the whole entity, since losing
  curvature on one segment is a much smaller loss than losing the
  entity's position data entirely.

## [spiral-5-command] - Command + Undo/Redo

Snapshot (Memento)-based Undo/Redo for Translate/Rotate/Scale, plus a
composite command for multi-entity operations.

### Added
- `document::ICommand` -- `Execute`/`Undo`/`Redo` interface.
- `document::TransformEntityCommandBase`, `TranslateCommand`,
  `RotateCommand`, `ScaleCommand` -- each stores only an `EntityId`
  (never a raw `Entity*`) plus a before/after `Shape` snapshot taken at
  `Execute()` time; `Undo`/`Redo` restore the exact snapshot rather than
  recomputing an inverse transform.
- `document::MacroCommand` -- composite of `ICommand`s executed/undone/
  redone as one atomic history entry, with `SuccessCount()`/
  `TotalCount()` for partial-success reporting (e.g. some entities
  locked) without collapsing that into `Execute()`'s `bool` return.
- `document::CommandHistory` -- undo/redo stacks; executing a new
  command after an `Undo` clears the redo stack (standard discipline).

### Fixed / design note
- An early draft of the Command design stored raw `Entity*` pointers
  cached at construction time. Demonstrated with real code that this
  produces a dangling pointer once `Document::Add()` reallocates the
  underlying `vector<Entity>` -- and that the resulting undefined
  behavior does not reliably crash (a corrupted value like `-0.0`
  instead of `0.0` was observed, not a crash). Every command now
  re-resolves its `EntityId` at the moment it's needed instead.
- An early draft implemented Undo via "store the transform's
  parameters, recompute the inverse" (e.g. `Scale(shape, 1/factor,
  pivot)`). Demonstrated with real code that 5 cycles of Scale-then-
  inverse-Scale with an off-origin pivot drifts by ~1.4e-14 in a
  coordinate. The Memento/snapshot design was adopted specifically to
  eliminate this: 100 cycles of Execute+Undo now show zero drift
  (bit-exact), verified in `CommandTests.cpp`.

## [spiral-4-transform] - Transform System (Translate/Rotate/Scale)

### Added
- `geometry::Translate/Rotate/Scale` for `Line2`/`Circle2`/`Arc2` --
  pure functions with no dependency on `document` or `math` (angles are
  plain radians, matching `Arc2`'s own convention).
- `document::TranslateEntity/RotateEntity/ScaleEntity` and their
  `*Selection` counterparts -- `Entity` versions return `bool` (did
  this actually change anything), `Selection` versions return
  `std::size_t` (how many of the selected entities actually changed).
- Scale is restricted to a single uniform factor (`sx == sy`); a
  non-uniform scale would turn a `Circle2`/`Arc2` into an ellipse, which
  this project has no type to represent. Deferred until/unless an
  `Ellipse2` type exists.
- `factor <= 0` is rejected at the `document` layer (not `geometry`,
  which only asserts it as a debug sanity check) -- `0` collapses
  geometry to a point, negative is an unsupported reflection for this
  Spiral.
- `Layer::Locked()` gets its first real consumer here: Transform
  rejects locked entities outright (shape verifiably unchanged), while
  `HitTest`/`Selection` (Spiral 3) deliberately do not check `Locked`
  at all -- a locked entity can still be selected/inspected, just not
  edited.

### Known process incident (documented for the historical record)
- `TRF-002` (Rotate) and `TRF-003` (Scale) were designed, implemented,
  and locally verified, but were never actually committed to git --
  the `spiral-4-transform` tag was mistakenly created against a commit
  that only contained `TRF-001` (Translate) plus an unrelated
  `.gitignore` fix. This went undetected for several work sessions
  because delivery confirmation (a screenshot of `verify.sh` passing
  and `git push` succeeding) was not consistently required before
  moving on to the next task.
- Root-caused via `git log --all -- <path>`, which showed the file had
  only ever been touched by the `TRF-001` commit. Recovered by
  re-verifying and re-committing the `TRF-002`/`TRF-003`/Spiral-5
  content together as a single commit, then deleting and recreating
  the `spiral-4-transform` tag against the corrected commit.
- Process change going forward: every delivery requires an explicit
  screenshot of `verify.sh` (both Debug and Release) passing AND
  `git push` succeeding before it is considered complete.

## [spiral-3-selection] - Selection System

### Added
- `document::EntityId` -- stable identifier assigned by
  `Document::Add()`, independent of position in `Document::Entities()`.
  `Document::FindEntity()`/`FindEntityMutable()` resolve it in O(1) via
  an internal `unordered_map<EntityId, size_t>` index.
- `document::SelectionSet` -- a set of selected `EntityId`s, explicitly
  NOT part of `Document` (session/UI state, not drawing data).
  `Select`/`Deselect`/`Toggle` return `[[nodiscard]] bool` indicating
  whether the call actually changed anything.
- `geometry::DistanceToShape` (`Line2`/`Circle2`/`Arc2`) and
  `document::HitTest` -- nearest-entity-within-tolerance query, with a
  `BoundingBox2` fast-reject before the exact distance calculation.
- `render::RenderOptions` -- extensible container for render-time
  overlays (`selection` is the first field used; others are reserved
  for future Spirals). Selected entities render with a fixed highlight
  style (`#ff0000`, `+1.5` stroke width) as a second render pass, always
  on top regardless of the entity's position in `Document::Entities()`.
- `modules/dxf/examples/sample_house.dxf` kept in-tree as a real (not
  hand-authored) test asset for Selection/HitTest, per a mid-Spiral
  roadmap reordering decision (see below).

### Changed (roadmap)
- Originally, Spiral 3 was DXF Import. Reordered after Spiral 2: a
  drawing that can be imported but not selected/moved/undone only
  enables *viewing*, not editing -- the defining capability of a CAD
  tool. Selection/Transform/Command were moved ahead of DXF Import/
  Export; DXF-001 (LINE/CIRCLE/ARC import, already fully designed) was
  kept as a backlog item and its output (`sample_house.dxf`) repurposed
  as real test data for the reordered Spirals, rather than discarded.
  See `docs/ROADMAP_EXECUTION.md`'s own reordering note.

## [spiral-2-doc-003] - Layer System

First feature of Spiral 2. Layers are a real, functioning part of the
render pipeline -- every Layer attribute except `locked` has a concrete
consumer, per the Spiral principle that nothing is added "to have later."

### Added
- `document::Layer` -- name, color, line type, line weight, visibility,
  locked flag. Name is the layer's identity (DXF/AutoCAD convention) and
  is immutable after construction.
- `document::LineType` -- `Continuous`, `Dashed`, `Dotted`, `DashDot`.
- `document::Entity` -- a shape plus the name of the layer it's on.
- `Document::CreateLayer()` (idempotent get-or-create),
  `Document::FindLayer()`, `Document::Layers()`.
- Every `Document` now starts with a default layer named `"0"`, matching
  the DXF/AutoCAD convention.
- `SvgDocument::AddLine/AddCircle/AddArc` accept an optional
  `dashArray` parameter (SVG `stroke-dasharray`).
- `layers_demo` example -- renders a Walls (black, solid), Center (gray,
  dashed), and Hidden (`visible=false`) layer, demonstrating that a
  hidden layer produces no SVG output at all.
- `LayerTests`, plus new Layer-focused cases in `DocumentTests`,
  `RenderDocumentTests`, and `SvgDocumentTests`.

### Changed
- `Document::Add(shape)` now takes an optional layer name
  (`Add(shape, "Walls")`), auto-creating that layer if it doesn't exist.
  **Calls without a layer name are unchanged** and still go to layer
  `"0"` -- verified by rebuilding `document_demo` and confirming its SVG
  output is byte-identical to before this change.
- `RenderToSvg()` now honors each entity's layer: color -> `stroke`,
  line weight -> `stroke-width`, line type -> `stroke-dasharray`, and
  `visible == false` -> the entity is not rendered at all.
- `Document::Bounds()` now excludes entities on hidden layers ("zoom
  extents" should fit what's visible, not what isn't).

### Breaking
- `Document::Shapes()` -> `Document::Entities()`, returning
  `vector<Entity>` (shape + layer) instead of `vector<Shape>`. All
  in-repo call sites were updated in the same change.

### Known limitations (deliberate, tracked in code)
- Layers are referenced by name, not by a stable ID -- see
  `TODO(Spiral4)` in `Layer.hpp` and `Document.hpp`. Renaming a layer is
  therefore not implemented.
- Color is a raw SVG color string, not an abstraction -- see
  `TODO(Spiral4)` in `Layer.hpp`. This cannot represent DXF ACI indices.
- `Layer::Locked()` is stored but has no behavior yet; there is no
  Selection/Editing system for it to affect.

## [spiral-1] - First vertical slice

Foundation -> Geometry -> Document -> Render, end to end, producing a
real `.svg` file.

### Added
- `foundation` -- curated wrappers over the standard library
  (see `docs/ARCHITECTURE_DECISION_RECORDS/ADR-0001`).
- `geometry` -- `Point2/3`, `Vector2/3`, `Line2`, `Circle2`, `Arc2`,
  `BoundingBox2` and per-shape `Bounds()`.
- `math` -- `Angle` (unit-safe), `Matrix3` (2D affine transforms),
  `Matrix4` (3D, now frozen per ADR-0004), `NumericTraits`, tolerance
  comparison.
- `document` -- `Document` holding an ordered list of shapes, with
  `Bounds()` for zoom-extents.
- `render` -- `SvgDocument` (zero-dependency SVG writer: points, lines,
  circles, arcs) and `RenderToSvg(Document&)`.
- CMake build with per-module targets, CTest integration, and a
  warnings-as-errors option (`OPENHOUSE_WARNINGS_AS_ERRORS`).
- GitHub Actions CI (Linux, GCC 13 + Clang 18, Debug + Release).
- Example programs: `spiral1_demo`, `spiral2_demo`, `primitives_demo`,
  `arc_demo`, `transform_demo`, `document_demo`.

### Fixed
- `Foundation.hpp` was missing most of the module's own headers.
- `NonCopyable` unintentionally suppressed move operations; `NonMovable`
  documented as disabling both copy and move deliberately (see
  ADR-0002).
- `Mutex.hpp` used `std::shared_mutex`/`shared_lock` without including
  `<shared_mutex>`.
- `Thread.hpp` used a `using`-declaration for the `std::this_thread`
  *namespace*, which is not valid.
- `Chrono.hpp` used a blanket `using namespace std::chrono`, contrary to
  the module's curated-wrapper convention (ADR-0001).
- `LengthSquared` was constrained to floating-point types despite
  requiring no division or square root (see ADR-0005).
