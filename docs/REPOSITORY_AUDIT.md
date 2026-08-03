# Repository Audit

This document records findings from a top-to-bottom repository audit and
the architectural decisions made in response. It exists so that future
contributors can see *why* the project's direction changed, not just
*that* it changed -- see the individual ADRs referenced below for the
full reasoning behind each decision.

## Audit findings

### 1. Development Spiral violation: `Matrix4`/`Angle` had no consumer

`Matrix4` (~180 lines) and `Angle` (~140 lines), the largest pieces of
the `math` module at the time, were found to have **no consumer
anywhere in the codebase outside their own test files** -- no demo, no
vertical slice exercised them end to end. This directly contradicted
this project's own Development Spiral principle: building
infrastructure ahead of any demonstrated need is the failure mode
Spiral development exists to prevent (a wrong assumption in unused code
isn't caught until something finally tries to use it -- potentially
much later, at higher cost).

**Resolution**: see
`docs/ARCHITECTURE_DECISION_RECORDS/ADR-0004-2d-first-freeze-3d.md`.
`Matrix4`/`Point3`/`Vector3` are frozen (kept, not extended). `Matrix3`
was built instead, immediately given a real consumer
(`transform_demo`), matching what the project's actual (2D) work needs.

### 2. `Chrono.hpp` violated the foundation module's own curation
   philosophy

`modules/foundation/include/openhouse/foundation/Chrono.hpp` used
`using namespace std::chrono;` -- a blanket import, unlike every other
header in `foundation`, which either curates individual names
(`using std::X;`) or uses a namespace alias (`namespace fs =
std::filesystem;`). This pulled every name in `std::chrono` (including
generic-sounding ones like `days`, `weekday`, `year`, `month`) into
`openhouse::foundation` unfiltered, contradicting ADR-0001's stated
"curated" wrapping philosophy, and creating a real (if latent) risk of
future name collisions.

This was a pre-existing issue (predating most of this project's active
development) that produced no compile error, so it was never caught by
normal build verification -- only surfaced by deliberately auditing
every header against the project's own stated conventions rather than
just reacting to compiler diagnostics.

**Resolution**: fixed to curate explicitly (clock types, duration
typedefs, casts individually; `chrono_literals` specifically kept as a
`using namespace` since literal-suffix operators are meant to be used
unqualified, unlike named types).

### 3. 2D/3D scope mismatch between `geometry` and `math`

`geometry` was almost entirely 2D-focused (`Point2`/`Vector2`/`Line2`/
`Circle2`/`Arc2`, driven by SVG being the Spiral 1 output target), while
the only transform matrix available was `Matrix4` (3D) -- there was no
`Matrix3` for 2D affine transforms, despite 2D being what the project's
actual work needed. This is the same root cause as finding #1: 3D
infrastructure was built without being anchored to an actual, current
need.

**Resolution**: same as #1 -- ADR-0004 explicitly scopes v1.x to 2D;
`Matrix3` fills the gap this created.

### 4. CI did not execute demo/example executables

`.github/workflows/ci.yml`'s `ctest` step only knows about registered
*test* targets, not the example/demo executables (`spiral1_demo`,
`arc_demo`, `transform_demo`, `document_demo`, etc.) -- these were built
in CI but never actually run, so a runtime-only bug in a demo (as
opposed to a compile error) could go unnoticed by CI.

**Resolution**: added a "Run demo executables" CI step that executes
each demo and requires a clean exit code.

### 5. No persisted backlog/roadmap tracking

Task-by-task progress (e.g. "MATH-001 through MATH-004 complete") was
reported conversationally but never captured in a file a future
contributor (or a differently-scoped tool session) could read without
the full conversation history.

**Resolution**: `docs/ROADMAP_EXECUTION.md` now tracks Spiral-level
progress; individual task IDs (e.g. `GEO-004`, `DOC-002`) are referenced
in commit-equivalent groupings within this audit and the roadmap rather
than only existing in conversation.

### 6. README drift

`README.md`'s project structure section listed only `foundation` and
`geometry` after `math`, `render`, `document`, and a scaffolded `app`
module had already been added -- a routine but real case of
documentation lagging code. Fixed as part of this audit; worth noting as
a process reminder: **update `README.md`'s structure section in the same
change that adds a new top-level module**, not as a separate cleanup
pass later.

## Scope and process decisions made in response

1. **2D-first for v1.x; 3D frozen.** See
   `docs/ARCHITECTURE_DECISION_RECORDS/ADR-0004-2d-first-freeze-3d.md`.
2. **Development model: Spiral, not sequential architectural layers.**
   See `docs/ROADMAP_EXECUTION.md`'s "Development model" section. The
   originally proposed "A1 through A10" phase table (Geometry Engine →
   CAD Document → File IO → Rendering → Editing → Snapping → Dimension →
   Printing → Plugin → Release) is retained as the *scope* map but
   re-sequenced into 8 Spirals, each ending in a runnable demo rather
   than requiring a prior phase's full completion first.
3. **`BoundingBox2` added to Spiral 1** (ahead of when it would
   otherwise have come up) because its cost was low and its reuse value
   is high across several planned future features (Zoom Extents, Hit
   Test, Selection, spatial indexing, Renderer culling, DXF bounds).
   Implemented alongside correct arc bounding-box handling (an arc's
   bounds are not simply its endpoints' bounding box if the arc sweeps
   through an axis-aligned extreme -- see
   `modules/geometry/include/openhouse/geometry/Bounds.hpp`).
4. **CI now runs demo executables**, not just registered tests (finding
   #4 above).

## What this audit does not cover

- CMake configuration has still not been verified against a real
  `cmake` invocation (a persistent limitation of the environment this
  audit and the surrounding development work were done in -- see
  `README.md`/commit history for the standing request to run `cmake -S
  . -B build`, `cmake --build build`, and `ctest` in a real environment
  and report results).
- Qt6 integration (`modules/app`) remains scaffold-only, per
  `docs/ARCHITECTURE_DECISION_RECORDS/ADR-0003-windowing-gui-stack.md`
  and `docs/QT_INTEGRATION_CHECKLIST.md` -- unaffected by this audit.
