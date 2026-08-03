# OpenHouseCAD Engineering Principles

## Principle 0
Every technical decision must optimize the long-term health of the project rather than short-term implementation speed.

## Core Principles
1. Architecture before implementation.
2. Incremental development in complete, testable sprints.
3. Buildable main branch.
4. Tests accompany features.
5. Learn from mature open-source projects, adapt rather than copy.
6. Keep module boundaries clean.
7. Document major architectural decisions. See `docs/ARCHITECTURE_DECISION_RECORDS/README.md` for the full index.

## Project Strategy
8. 2D-first development. v1.x targets 2D CAD only; see `docs/ARCHITECTURE_DECISION_RECORDS/ADR-0004-2d-first-freeze-3d.md`.
9. Spiral development. Build thin slices across layers, not one layer to completion before the next; see `docs/ROADMAP_EXECUTION.md`.
10. Deliver vertical slices. Every unit of work should touch enough of the stack (e.g. Geometry -> Document -> Renderer) to prove the layers actually compose, not just that one layer's tests pass in isolation.
11. Every spiral ends with a runnable demo. Passing unit tests alone is not sufficient evidence a spiral is done -- something must actually run and produce visible output.
12. Freeze 3D expansion until 2D Alpha is complete. New geometry/math primitives default to 2D unless a concrete 2D CAD feature genuinely requires 3D; see ADR-0004.
