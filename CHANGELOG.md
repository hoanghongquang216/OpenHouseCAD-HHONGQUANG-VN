# Changelog

All notable changes to OpenHouseCAD are documented here.

Format is loosely based on [Keep a Changelog](https://keepachangelog.com/);
versioning follows the Spiral milestones in `docs/ROADMAP_EXECUTION.md`
rather than semantic versioning, since the project has no public API
consumers yet.

## [Unreleased]

Nothing yet.

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
