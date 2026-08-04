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
13. `Document` is the canonical model. Every importer converts an external format into a `Document`; every exporter converts a `Document` into an external format. Importers and exporters never communicate directly with each other. See `modules/dxf/include/openhouse/dxf/DxfReader.hpp` for the current (import-only) reference implementation.
14. Text-based parsers tokenize, then group, then build -- directly into `Document`, with no separate reusable "parser framework" layer unless a second format actually needs one (see Principle on abstraction in `docs/AI-Working-Agreement.md`). The DXF reader's actual structure (`Tokenizer` -> `EntityChunk` -> `Document`) is the reference shape for any future importer, not a speculative generalization of it.
15. Geometry algorithms are verified numerically, not just reasoned about, before they're integrated into a parser or transform. See `docs/AI-Working-Agreement.md` (rule 2) for the specific incidents that established this and the verification pattern to follow (independently reconstruct expected values, check against randomized cases, not just one hand-picked example).
16. Codebase convention outranks a ticket's original design. When a ticket's
    planned approach conflicts with convention already established and
    proven elsewhere in the codebase, the default is to adapt the design to
    fit the codebase -- not the other way around -- unless the new pattern
    has a concrete, demonstrated benefit the existing convention lacks. See
    `svg-pipeline` (CHANGELOG.md): the originally proposed approach
    (per-format fixture files on disk, a golden-file comparison pattern,
    GoogleTest) was revised during implementation review to
    `modules/dxf/tests/SvgPipelineIntegrationTests.cpp` -- inline DXF text
    via `std::istringstream` and `OH_CHECK`/`main()`, matching
    `DxfReaderTests.cpp` and `RenderDocumentTests.cpp` exactly -- after
    review found the original design duplicated existing unit-test
    coverage and introduced a comparison pattern with no precedent
    anywhere else in the project.
