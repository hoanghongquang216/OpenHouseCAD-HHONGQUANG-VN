# OpenHouseCAD-HHONGQUANG-VN

Ứng dụng Thiết kế Công trình Xây dựng.

## Current project status

```
Phase:                 Epic 4 (Editing) and Epic 6 (DXF Export) complete;
                        Epic 5 (Drafting) not started
Primary goal:          2D CAD
Current architecture:  2D-first (3D frozen -- see ADR-0004)
Development model:     Spiral Development (see docs/ROADMAP_EXECUTION.md)
```

A modular C++23 CAD kernel/application, developed via short vertical
"Spirals" (see `docs/ROADMAP_EXECUTION.md`) rather than building each
architectural layer to completion before the next. Current state:
Foundation, Geometry (2D primitives through Arc2 and BoundingBox2),
Math, and Document (layers, selection, and an ordered list of entities)
are functional with test coverage. A Command/Undo-Redo system exists,
covering Translate/Rotate/Scale, Trim, Extend, Copy, and Delete, each
fully undoable/redoable. Snap (Endpoint/Midpoint/Center/Intersection) is
implemented. DXF import and export both exist for the LINE/CIRCLE/ARC/
LWPOLYLINE entity subset targeting DXF R12 (import explodes LWPOLYLINE
into Line/Arc segments; export writes HEADER/TABLES/ENTITIES, round-
tripping layer color and linetype). Render can output SVG (points,
lines, circles, arcs) and render a whole `Document` in one call,
honoring per-layer color, line weight, line type, and visibility. The
application layer (`modules/app`) is scaffolded for Qt6 but not yet
implemented -- see
`docs/ARCHITECTURE_DECISION_RECORDS/ADR-0003-windowing-gui-stack.md`.

## Where to go next

- **Roadmap and development model**: `docs/ROADMAP_EXECUTION.md`
- **Build and test instructions**: `docs/BUILD_AND_TEST.md`
- **Architecture decisions (ADR index)**: `docs/ARCHITECTURE_DECISION_RECORDS/README.md`
- **Engineering principles**: `docs/ENGINEERING_PRINCIPLES.md`
- **Change history**: `CHANGELOG.md`
- **Audit history and rationale for major direction changes**: `docs/REPOSITORY_AUDIT.md`

## Requirements

- A C++23 compiler (GCC 13+ or Clang 17+ recommended)
- CMake 3.25+

## Building

```bash
cmake -S . -B build
cmake --build build --parallel
```

## Running tests

Tests are built automatically as part of the default build (via CTest,
enabled at the top level). Run them with:

```bash
ctest --test-dir build --output-on-failure
```

## Enforcing strict warnings

`docs/CODING_STANDARD.md` requires warnings to be treated as errors in CI.
Locally this is opt-in (off by default, for a smoother edit/compile loop);
to match CI's strictness:

```bash
cmake -S . -B build -DOPENHOUSE_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
```

## Building the application layer (not yet implemented)

`modules/app` (Qt6-based, per ADR-0003) is scaffold-only -- skipped by
default. Do not enable this unless you have Qt6 installed and are
following `docs/QT_INTEGRATION_CHECKLIST.md`; enabling it without Qt6
fails with a clear error rather than building anything:

```bash
cmake -S . -B build -DOPENHOUSE_BUILD_APP=ON
```

## Project structure

```
modules/
  foundation/   Header-only base utilities (containers, memory, threading,
                RAII helpers, etc. -- curated aliases over the standard
                library; see docs/ARCHITECTURE_DECISION_RECORDS/ADR-0001,
                ADR-0002).
  geometry/     Core 2D/3D geometric primitives (Point2/3, Vector2/3,
                Line2, Circle2, Arc2, BoundingBox2/Bounds) and the
                operations between them. 2D is actively developed; 3D
                (Point3/Vector3) is frozen -- see ADR-0004, ADR-0005.
  math/         Angle (unit-safe radians/degrees), Matrix3 (2D affine
                transforms, actively used), Matrix4 (3D, frozen per
                ADR-0004), NumericTraits, tolerance-based floating-point
                comparison.
  document/     Document: layers plus an ordered list of entities (a
                shape + the layer it's on). Auto-creates layers on use,
                always has a default layer "0", and Bounds() (zoom
                extents) skips hidden layers. Also includes Selection
                (SelectionSet), a Command/Undo-Redo system (Translate/
                Rotate/Scale/Trim/Extend/Copy/Delete), and Snap
                (Endpoint/Midpoint/Center/Intersection).
  dxf/          DXF import (DxfReader) and export (DxfWriter) for the
                LINE/CIRCLE/ARC/LWPOLYLINE entity subset, targeting DXF
                R12. Import explodes LWPOLYLINE into Line2/Arc2 segments;
                layer color and linetype round-trip through the TABLES/
                LAYER section on export.
  render/       Zero-dependency SVG output (SvgDocument), plus
                RenderToSvg(Document&) which honors each entity's layer
                color, line weight, line type, and visibility -- the
                current "Spiral" output target; see
                docs/ROADMAP_EXECUTION.md.
  app/          Scaffold only, not yet implemented. Will be the Qt6-based
                application layer; see
                docs/ARCHITECTURE_DECISION_RECORDS/ADR-0003 and
                docs/QT_INTEGRATION_CHECKLIST.md. Skipped by default;
                enable with -DOPENHOUSE_BUILD_APP=ON (requires Qt6).
docs/
  ARCHITECTURE_DECISION_RECORDS/   Major architecture decisions (ADRs);
                                    see that directory's README.md for
                                    the full index.
  BUILD_AND_TEST.md                Full build/test instructions and CI
                                    details (this README has a quick
                                    version below).
  CODING_STANDARD.md               Language/style/CI conventions.
  ENGINEERING_PRINCIPLES.md        Guiding principles for the project.
  DEFINITION_OF_DONE.md            Checklist for calling a sprint complete.
  ROADMAP_EXECUTION.md             Spiral development model and roadmap.
  REPOSITORY_AUDIT.md              Audit findings and the decisions made
                                    in response (why direction changed).
  QT_INTEGRATION_CHECKLIST.md      Steps for integrating Qt6 once available
                                    (see modules/app).
cmake/
  OpenHouseWarnings.cmake          Shared warnings-as-errors helper.
.github/workflows/
  ci.yml                           Build + test matrix (GCC/Clang x
                                    Debug/Release), warnings-as-errors on,
                                    demo executables run (not just built).
```

Each module is a CMake target namespaced under `OpenHouse::` (e.g.
`OpenHouse::Foundation`, `OpenHouse::Geometry`, `OpenHouse::Math`,
`OpenHouse::Document`, `OpenHouse::Dxf`, `OpenHouse::Render`) for use by
downstream targets via `target_link_libraries`. `modules/app` has no
target yet (scaffold only).

See `docs/BUILD_AND_TEST.md` for full build/test/CI details beyond the
quick-start commands above.

## Contributing

See `docs/CODING_STANDARD.md`, `docs/ENGINEERING_PRINCIPLES.md`, and
`docs/DEFINITION_OF_DONE.md` before contributing. Non-trivial architecture
decisions should be recorded as an ADR under
`docs/ARCHITECTURE_DECISION_RECORDS/`, following the template in that
directory's `README.md`.

## License

MIT. See `LICENSE`.
