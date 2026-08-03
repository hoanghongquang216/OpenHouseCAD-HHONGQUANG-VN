# OpenHouseCAD Execution Log

This document marks the beginning of implementation.

## Phase 1 (historical)
- Foundation Bootstrap
- Runtime
- Math
- Geometry Kernel

Development proceeded incrementally with buildable commits.

## Scope decision (superseding Phase 1's original ambition)

Per `docs/ARCHITECTURE_DECISION_RECORDS/ADR-0004-2d-first-freeze-3d.md`:
**v1.x targets 2D CAD only**, modeled on realistic prior art (LibreCAD,
QCAD class software), with DXF as the primary interchange format. 3D
(`Point3`, `Vector3`, `Matrix4`) is frozen, not actively developed.

## Development model: Spiral, not sequential layers

A repository audit found that building each architectural layer to
completion before the next (Geometry Engine fully done, then Document,
then Renderer, then DXF, then Commands...) creates exactly the risk this
project set out to avoid: architectural problems (e.g. "the topology we
chose doesn't fit," or "the transaction model doesn't support
parametric editing") surface only after months of work in a single
layer, when they are expensive to fix.

Instead, development proceeds as a **Development Spiral**: each
iteration is a thin *vertical* slice through multiple layers, ending in
something runnable/demoable, before returning to deepen any one layer.

```
Geometry (minimal)
      ↓
Document (minimal)
      ↓
Renderer (minimal)
      ↓
Demo that runs, produces visible output
      ↓
back to: extend Geometry
      ↓
extend Document
      ↓
DXF
      ↓
... (repeat)
```

Every Spiral ends with something concretely runnable -- a demo, not just
passing unit tests -- so that a wrong assumption in how layers fit
together is caught within days, not months. See individual module
READMEs and `docs/ARCHITECTURE_DECISION_RECORDS/` for the ADRs recording
decisions made along the way (STL wrapping philosophy, move-semantics
conventions, the windowing/GUI stack, and the 2D-first/3D-freeze
decision above).

## Spiral roadmap

Supersedes the earlier "A1 through A10" sequential-phase table -- the
same scope, reorganized into spirals so that each one produces a
runnable result rather than requiring a prior phase to be "complete"
first.

### Spiral 1 -- COMPLETE
- Geometry (minimal): `Point2`, `Line2`, `Circle2`, `Arc2`,
  `BoundingBox2`/`Bounds()`
- `Document`: ordered shape collection, `Bounds()` (Zoom Extents)
- `SvgDocument` renderer: points, lines, circles, arcs
- `RenderToSvg(Document, SvgDocument&)`: bridges Document to output
- Demo: `document_demo` -- builds a `Document`, renders it to a single
  `.svg` file in one call

### Spiral 2 -- in progress
- `Layer` (name, color, line type, line weight, visibility, locked) --
  **DONE** (DOC-003). Every attribute except `locked` has a real
  consumer in `RenderToSvg`; `locked` awaits Selection. Layers are
  referenced by name, and color is a raw SVG color string -- both are
  deliberate, with `TODO(Spiral4)` markers in `Layer.hpp`/`Document.hpp`
  recording when to revisit.
- `BoundingBox2`/`Bounds()` completeness -- **DONE** (delivered early in
  Spiral 1 per the repository audit's recommendation)
- `Selection` (a set of references into a `Document`) -- not started

### Spiral 3
- DXF Import (a defined, deliberately limited entity subset first --
  LINE/CIRCLE/ARC/LWPOLYLINE, one DXF version -- not the full DXF
  specification; scope the "basic" subset explicitly before starting,
  per the Alpha criteria below)

### Spiral 4
- DXF Export (same deliberately limited entity subset as Import)

### Spiral 5
- Command System (the seam undo/redo and scripting/plugins will build
  on)
- Undo/Redo

### Spiral 6
- Snap (Endpoint, Midpoint, Center, Intersection)

### Spiral 7
- Dimension (Linear, Aligned, Radius, Diameter)

### Spiral 8
- Printing / PDF export

Windowing/interactivity (click-to-select, click-to-draw) depends on the
Qt6 integration described in
`docs/ARCHITECTURE_DECISION_RECORDS/ADR-0003-windowing-gui-stack.md` and
`docs/QT_INTEGRATION_CHECKLIST.md`; these Spirals can and do proceed
without it in the meantime (SVG output remains the verification path
for anything not yet interactive), but interactive editing (Spiral 5's
Move, for example) will need it.

## v0.1 Alpha criteria

To call the project "Alpha", per repository audit discussion, at
minimum:

- Builds successfully on Windows and Linux.
- CI green.
- Unit tests for the core geometry classes.
- Can open and save a basic DXF file.
- Can display Line, Circle, Arc, and Polyline.
- Can perform Move, Copy, and Delete.
- README and developer documentation up to date.

Windows CI is explicitly deferred until closer to this milestone being
otherwise reachable -- adding a second OS to the CI matrix now, before
there's a Windows-relevant feature to validate, would be effort spent
ahead of need, the same failure mode ADR-0004 flags for the 3D work.
