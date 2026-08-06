# Design Debt Register

Design decisions that are neither a feature (belongs in a `*_BACKLOG.md`)
nor a bug — a choice that was deliberately deferred, with the evidence
and the condition under which it should be revisited. Each entry stays
open until its "Review After" condition is met; at that point it is
either acted on, re-deferred with updated evidence, or closed as no
longer relevant.

---

## DD-001: Generalize TransformEntityCommandBase

**Decision:** Acted on — resolved by REFACTOR-001 (merged `9eacac2`).

**Resolution:** `TransformEntityCommandBase` was generalized into
`EntityShapeCommandBase` (the `DoTransform` hook renamed to
`DoOperation`). `TrimCommand` and `ExtendCommand` were rewritten to
inherit it instead of duplicating the Memento pattern, each shrinking
from ~54 lines to ~26-28 lines. Verified with zero regression (31/31
tests passing at merge time, including the Translate/Rotate/Scale,
Trim, and Extend suites).

**Original reason (kept for context):** TRIM-001 and EXTEND-001 both
intentionally wrote independent `ICommand` implementations
(`TrimCommand`, `ExtendCommand`) rather than sharing
`TransformEntityCommandBase`'s Memento logic, per each sprint's own Go
decision (insufficient evidence of duplication at the time). By
EXTEND-001, three independent implementations shared the exact same
Memento pattern, meeting the evidence threshold for a shared base.

**Review after (met):** Resolved before COPY-001 implementation began,
as planned. `COPY-001`'s `EntityCreationCommandBase` and `DELETE-001`'s
standalone `ICommand` implementation were both designed with
`EntityShapeCommandBase` already in place as the precedent for when
(and when not) to share a base — see
`docs/design/COPY-001-Design.md` §4 and
`docs/design/DELETE-001-Design.md` §2.

---

## DD-002: Projection.hpp / Intersection.hpp boundary

**Decision:** Not yet made — deferred.

**Reason:** `FindExtendIntersection` (added in EXTEND-001) is, by
nature, an intersection computation with a relaxed bounds-check — it
belongs with `Intersect(Line2, Line2)`'s family more than with
`ParameterOnLine`'s "position along a line" family, but currently lives
in `Projection.hpp` for expediency at the time it was written.

**Evidence:** `Projection.hpp` now holds two functions
(`ParameterOnLine`, `FindExtendIntersection`) that don't share a single
clear theme. No functional bug — both compile, both are tested — this
is a cohesion/naming concern only.

**Impact if deferred further:** Low on its own; compounds if a future
sprint (e.g. Offset, Fillet) adds another intersection-family function
to `Projection.hpp` "because it's already there," instead of
`Intersection.hpp` where it more naturally belongs.

**Priority:** Low.

**Review after:** Next time a new function is added to either file
(natural trigger to decide the split), or at the next Epic Review,
whichever comes first.

---

## DD-003: Evaluate need for Domain Decision Records (DDR)

**Decision:** Not yet made — deferred.

**Reason:** EDGEMODE (EXTEND-001's "No extend" vs "implied edge"
choice) is a CAD domain-behavior decision, not an architecture
decision — it doesn't naturally belong in an ADR (`ADR-0006` and future
ADRs should stay about module/layer boundaries, not CAD behavior
semantics). But a single such decision doesn't yet justify a new
`docs/DOMAIN_DECISIONS/` document tier.

**Evidence:** Epic 4 (Editing) has begun accumulating CAD behavioral
decisions (EDGEMODE so far). More are expected as Offset, Fillet, and
Chamfer are audited (boundary semantics, multiple-intersection
pick-first rules, snap priority interactions).

**Impact if deferred further:** None yet — EDGEMODE's rationale is
still readable from EXTEND-001's PR description and this project's
chat history. Risk grows only once 3+ such decisions exist without a
dedicated home.

**Priority:** Low — revisit, don't act.

**Review after:** Offset and Fillet sprints are both complete. If ≥2
more domain-behavior decisions have accumulated by then, create
`docs/DOMAIN_DECISIONS/` and backfill DDR-0001 (EDGEMODE) into it at
that point.
