# Architecture Decision Records (ADR)

This directory stores all major architectural decisions.

ADR template:
- Context
- Problem
- Options considered
- Decision
- Consequences
- Revisit criteria

No major architecture decision should be made without an ADR.

## Index

| ADR | Status | Summary |
|---|---|---|
| [ADR-0001](ADR-0001-foundation-stl-wrapping.md) | Accepted | Wrap standard library facilities in `openhouse::foundation` via curated `using` declarations |
| [ADR-0002](ADR-0002-noncopyable-nonmovable-semantics.md) | Accepted | `NonCopyable` preserves move; `NonMovable` disables copy too (avoids a silent-copy-on-move footgun) |
| [ADR-0003](ADR-0003-windowing-gui-stack.md) | Accepted | Windowing/GUI stack: commit to Qt6 |
| [ADR-0004](ADR-0004-2d-first-freeze-3d.md) | Accepted | 2D-first scope for v1.x; freeze 3D development |
| [ADR-0005](ADR-0005-geometry-point-vector-semantics.md) | Accepted | Geometry `Point`/`Vector` semantics (distinct types, no `Point + Point`) |

Numbering is registration order, not strict chronological authoring
order -- see ADR-0005's own "Naming note" section for why it is numbered
0005 despite predating ADR-0001 through ADR-0004.
