# Changelog

All notable changes to OpenHouseCAD are documented here.

Format is loosely based on [Keep a Changelog](https://keepachangelog.com/);
versioning follows the Spiral milestones in `docs/ROADMAP_EXECUTION.md`
rather than semantic versioning, since the project has no public API
consumers yet.

## [Unreleased]

Nothing yet.

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
