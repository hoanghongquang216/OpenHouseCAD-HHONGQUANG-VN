# ADR-0003: Windowing/GUI Stack — Commit to Qt6

## Revision note

This ADR originally proposed a two-stage approach (GLFW+Dear ImGui as a
throwaway prototype, Qt evaluated later). After explicit product-priority
clarification -- this project targets a full, long-term 3D CAD kernel,
and rework/migration cost is a priority concern, not just setup speed --
that two-stage plan was reconsidered and replaced with the single-commit
decision below. The two-stage option is kept in "Options Considered" for
the record, but is no longer the decision.

A second revision softened the performance claims around QRhiWidget: the
original text implied QRhiWidget "meets the performance requirement,"
which overstated what this ADR's research actually established (that the
API exists and is documented) versus what it did not (that it is fast
enough for a CAD-scale scene, which is unmeasured). The decision itself
is unchanged; only the performance framing was corrected.

## Context

`docs/ROADMAP_EXECUTION.md`'s Spiral plan reaches a point (Spiral 3:
Selection/Highlight/Move) where output-only artifacts (SVG files, per
RENDER-001/002) are no longer sufficient -- the user needs to click,
select, and manipulate geometry interactively, and eventually (Milestone
4-5) needs a full application chrome: docking panels, a property/model
tree, native file dialogs, menus, and an embedded high-performance 2D/3D
viewport.

This is the most expensive-to-reverse decision made so far. Unlike the
library modules built so far (all zero-dependency, pure C++23 STL per
ADR-0001), any windowing/GUI choice pulls in a real external dependency
with its own build-system integration, licensing terms, and
architectural assumptions that shape the Application layer for years.
Given this project's stated goal (a full 3D CAD kernel, long-term,
avoiding rework), the cost of picking wrong and migrating later outweighs
the cost of a heavier up-front decision.

## Problem

The realistic reference point for "what a serious open-source CAD
application's GUI stack looks like" is FreeCAD and OpenSCAD, both built
on Qt, <cite index="79-1">a cross-platform application framework available under both proprietary and open-source licenses</cite>.

A lightweight alternative (GLFW/SDL + Dear ImGui) is attractive for fast
prototyping: <cite index="78-1">Dear ImGui is a bloat-free, immediate-mode GUI library with minimal dependencies, under a permissive MIT license, well suited to rapid prototyping</cite>. But <cite index="65-1">GLFW itself is not a GUI library -- it is a simple rendering window and input wrapper with no widgets</cite>, and immediate-mode UI does not natively provide the docking-panel/property-tree/file-dialog vocabulary a CAD chrome needs; that would have to be hand-built or bolted on, then likely discarded if the project later moves to Qt anyway.

The key question this ADR must answer given the revised priority (avoid
rework) is: **can Qt alone carry the project from Spiral 3's simple
click-to-select interaction all the way through Milestone 5's full CAD
chrome, without forcing a framework migration partway through?**

Research confirms yes, and specifically at the performance level that
matters for a CAD viewport: Qt6 provides <cite index="84-1">the Qt Rendering Hardware Interface (QRhi), a portable, cross-platform 3D graphics and compute API abstraction over OpenGL, OpenGL ES, Direct3D, Metal, and Vulkan</cite>, and specifically <cite index="87-1">QRhiWidget, a widget for rendering 3D graphics via an accelerated graphics API such as Vulkan, Metal, or Direct3D, integrated directly into a QWidget-based application</cite>. This means the CAD viewport can be a real hardware-accelerated Vulkan/Metal/D3D12 surface embedded natively inside the same widget tree as the docking panels and property editors -- there is no need to bolt together two separate UI paradigms (an immediate-mode overlay on top of a raw GL window) the way a GLFW+ImGui stack would require once real chrome is needed.

One caveat found during research: <cite index="86-1">Qt's RHI abstraction layer generates rendering commands serially from a single thread regardless of backend, so switching from OpenGL to Vulkan under Qt6 does not by itself yield large performance wins today</cite> -- the benefit of QRhi is portability and a modern API surface (compute, explicit resource management), not automatic multithreaded command generation. This is a real, honest limitation to note, not a reason to avoid Qt.

To be precise about what this ADR does and does not establish: research
confirms QRhiWidget **exists, is documented, and provides the API surface**
needed (a widget embedding Vulkan/Metal/D3D12/OpenGL rendering). It does
**not** establish that QRhiWidget will be *fast enough* for a CAD-scale
scene -- no benchmark, prototype, or measurement has been done as part of
this ADR. That question is explicitly left open and deferred to
implementation (see Revisit Criteria). The decision below is made on
portability, ecosystem fit, and avoiding a second framework migration --
not on a performance guarantee this ADR has not verified.

## Options Considered

1. **Two-stage: GLFW+Dear ImGui prototype now, Qt evaluated later.**
   (Original decision in this ADR's first version.)
   - Pros: cheapest, fastest possible path to a Spiral 3 demo.
   - Cons: **directly conflicts with the clarified priority.** The
     interactive shell (event loop, picking, selection UI) built against
     GLFW+ImGui is not reusable under Qt's event/widget model; migrating
     later means re-doing this work, not extending it. For a project
     explicitly optimizing to avoid rework, this is the wrong tradeoff.

2. **Commit to Qt6 now**, build Spiral 3's interactivity directly inside
   a `QWidget`-based application, with the 2D/3D viewport as a
   `QRhiWidget`.
   - Pros: single decision, no migration later; matches the proven
     FreeCAD/OpenSCAD precedent; full widget vocabulary (docking,
     property trees, native file dialogs) available from day one for
     Milestone 4-5, so that work is additive, not a rewrite; QRhiWidget
     provides a portable, hardware-accelerated rendering *path*
     (Vulkan/Metal/D3D12) natively embedded in the widget tree, avoiding
     a second rendering-framework decision later -- whether it is
     *fast enough* for a CAD-scale scene is not proven by this ADR and
     remains to be measured during implementation.
   - Cons: heavier dependency and build-system integration (Qt must be
     installed; CMake integration via `qt_add_executable`/AUTOMOC) is
     adopted now, before Spiral 3's actual interaction requirements are
     validated by real use; QRhi's single-threaded command generation
     (see Problem section) means the performance ceiling for very
     complex scenes is not "free" just from choosing Qt -- scene-graph
     and batching design still matters and will need real engineering
     later, same as it would under any other stack.

## Decision

Adopt **Option 2: commit to Qt6 now**, as a single, deliberate decision
covering both the Spiral 3 interactive prototype and the long-term
application chrome. Concretely:

- Windowing, input, and widget chrome: standard Qt6 Widgets
  (`QMainWindow`, `QDockWidget` for panels once needed, standard dialogs).
- 2D/3D viewport rendering: `QRhiWidget`, giving a portable rendering
  path across Vulkan/Metal/D3D12/OpenGL without committing to a single
  platform API. This is a claim about **portability and API surface**,
  not a proven performance guarantee -- QRhi's single-threaded command
  generation (noted in Problem) means actual CAD-scale performance is an
  open engineering question to be measured during implementation, not
  something resolved by this ADR. Choosing QRhiWidget avoids a *second*
  rendering-framework decision later; it does not by itself guarantee
  the eventual viewport will be fast enough for a complex scene.
- Build integration: CMake's Qt6 support (`find_package(Qt6 ...)`,
  `qt_add_executable`), isolated to a new `OpenHouse::App`-level target;
  this is a new category of target distinct from the header-only
  `INTERFACE` library targets used by `Foundation`/`Geometry`/`Math`/
  `Render` so far.

As with prior ADRs, the hard architectural constraint remains: `Model`,
`Transaction`, `Geometry`, and `Math` stay fully independent of Qt. No Qt
type may appear in those modules' public APIs. Only the new `App`-layer
target depends on Qt; the kernel layers remain framework-agnostic and
independently testable exactly as they are today.

## Consequences

- Qt6 becomes a required build dependency for anything at or above the
  Application layer. `Foundation`/`Geometry`/`Math`/`Render` remain
  buildable and testable with zero external dependencies, as today --
  this ADR does not change that for the kernel layers.
- CI (`.github/workflows/ci.yml`) will need a Qt6 installation step once
  App-layer targets exist; this is out of scope for this ADR but is a
  known, expected follow-up task when Spiral 3 implementation begins.
- Licensing: Qt6's open-source edition is LGPLv3, which permits linking
  from a project under a different (including permissive, e.g. this
  project's own MIT `LICENSE`) license, provided Qt is linked
  dynamically (the standard, default configuration) rather than
  statically. This is not expected to constrain OpenHouseCAD's
  distribution as currently licensed, but should be re-verified against
  the actual distribution plan once one exists (e.g. if static linking
  or a fully offline installer without redistributable Qt libraries is
  ever desired).
- The Spiral 3-5 interactive prototype work is now expected to be
  durable, forward-compatible code (built on the same framework the
  final application chrome will use), not throwaway prototype code. This
  removes the "temporary code that lingers" risk the two-stage option
  carried, at the cost of a heavier setup before Spiral 3 can begin.

## Revisit Criteria

Revisit this decision only if:
- Qt's LGPLv3 terms turn out to conflict with a distribution model
  adopted later (e.g. a fully static, no-redistributable-libraries
  installer requirement).
- A real benchmark/prototype of `QRhiWidget` under a representative
  CAD-scale scene (this ADR did not measure one) shows it cannot meet
  interactive frame-rate requirements -- at that point the appropriate
  first response is likely a specialized rendering path using
  QRhiWidget's native-command-recording escape hatch, not abandoning Qt
  entirely. Running this benchmark should be an explicit early task once
  Spiral 3/App-layer work begins, not something assumed to be fine.
- A genuinely disqualifying limitation of Qt6/QRhiWidget is discovered
  during Spiral 3 implementation that was not visible during this
  research-stage evaluation.
