# Roadmap Review — Sprint 7

Status: Complete
Result: **No Official Work Item selected for Sprint 7 implementation.** This is a valid outcome of the process, not a failure of it.

## Methodology

Bước 1 Kiểm kê sự thật → Bước 2 Dependency Graph → Bước 3 ROI → Bước 3.5
Decision Readiness → Bước 4 Strategic Alignment → Sprint Selection (only
when all axes converge). Source of truth priority: source code and
backlog files first, `ROADMAP_EXECUTION.md` as historical context unless
proven still authoritative.

Refinements introduced during this Review (now part of the standard
process for future reviews):
- **A1/A2/A3 split**: A1 Active Backlog (ROI-eligible) / A2 Deferred by
  Design (waiting on a trigger — "Not Ready", never "low ROI") / A3
  Documentation Maintenance (never competes in Sprint Selection).
- **Hard dependency vs knowledge dependency**: hard = A blocks B outright;
  knowledge = unknown until Domain Research happens.
- **Pre-ROI Sprint Unit Check**: "is this the right size to select as a
  Sprint?" (Yes / No-too-large / Unknown) — avoids comparing ROI across
  work items of very different sizes.
- **Promotion Check** (Candidate → Official, between Sprint Unit Check and
  ROI): 3 criteria — clear MVP scope, infra-maturity evidence, remaining
  foundational Domain Research — before a Bảng B candidate may enter A1.
- **Decision Readiness ≠ Implementation Risk**: low implementation risk
  does not by itself establish Readiness; Readiness requires evidence that
  no Decision Gate remains to close, which only dedicated Domain Research
  can establish.
- **Audit itself must be ROI-justified**: added "is the cost of reducing
  uncertainty worth it?" to Decision Readiness — Roadmap Review decides
  whether/how deeply to audit, never by default.
- **Roadmap-level Decision Gates**: `DG-RR-001` introduced for ambiguity
  in the roadmap document itself (see below), distinct from
  implementation-level Decision Gates like DG-001..003 in a Sprint's own
  Design doc.

## Bước 1 — Kiểm kê sự thật

- DXF-EXPORT-001 shipped (Sprint 6) — removed from A1.
- New backlog item `DXF-EXPORT-002` (layer-name escaping on export)
  classified as refinement, Low priority, A2 — doesn't block anything.
- `DD-001` (`DESIGN_DEBT.md`) confirmed resolved by REFACTOR-001 in
  substance, but the file on disk still says "deferred" — A3, unclosed.
- A1 (unchanged from prior Review) = **Dimension, Printing/PDF, Qt6/UI
  wiring**.

## Bước 2 — Dependency Graph

| Work Item | Dependency type | Note |
|---|---|---|
| Dimension | Knowledge dependency (unconfirmed) | Likely needs text rendering — `SvgDocument.hpp` has no text capability (`AddPoint`/`AddLine`/`AddCircle`/`AddArc` only). Not proven; Dimension's own design hasn't been audited. |
| Printing/PDF | Ambiguity, not yet a dependency question | `ROADMAP_EXECUTION.md`'s Spiral 8 names "Printing / PDF export" together without specifying whether the Official Work Item is headless PDF export, interactive printing, or both. Formalized as **`DG-RR-001`**, left explicitly at "C: chưa xác định" — not guessed. |
| Qt6/UI wiring | No dependency found either direction | Independent of the other two per available evidence. |

**`DG-RR-001` (open)**: What does "Printing / PDF export" mean as an
Official Work Item? Option A (headless PDF export, no Qt6 dependency,
similar shape to DXF Export) / Option B (includes interactive printing,
likely Qt6-dependent) / Option C (undefined until that item's own Domain
Research decides). Only needs resolving if/when Printing/PDF is actually
selected — that Sprint's own Phase 1 first task.

## Promotion Check — Bảng B re-evaluation

Two candidates whose infra-blocker status changed after Sprint 4-6:

- **Multi-select Delete — PROMOTED to Official Work Item.** Clear MVP,
  infra confirmed mature (`DELETE-001-Design.md §8`: "no new base or
  Document API needed"), remaining work is Application/Selection
  integration only.
- **Array — NOT promoted.** "Rectangular-only" scope is an extrapolation
  from this project's own MVP-narrowing precedent (Trim/Extend Line-only,
  DXF-002 subset, DXF-Export 3-entity-only), not a confirmed official
  scope. Needs its own short Domain Research to lock scope (base point,
  spacing, count, rectangular vs polar) before promotion.

## Bước 3 — ROI (qualitative, on Dimension/Printing-PDF/Qt6/Multi-select-Delete)

| Work Item | User Value | Architecture Value | Risk | Cost | Unlock | Evidence Confidence |
|---|---|---|---|---|---|---|
| Dimension | Trung bình (not in v0.1 Alpha criteria) | Chưa rõ | Cao (unknown-size subsystem: style/text/unit/associativity) | Chưa rõ | Thấp-Trung bình | Trung bình |
| Printing/PDF | Unknown (DG-RR-001 open) | Unknown | Unknown | Unknown | **Unknown** (headless-PDF vs interactive-printing unlock very differently) | Trung bình-Thấp |
| Qt6/UI wiring | Unknown (depends on Alpha-definition interpretation) | Not necessarily high (Application layer consumes existing architecture, doesn't change it) | Cao | Rất cao (signs of exceeding 1 sprint) | Rất cao (Product Impact) | Trung bình |
| Multi-select Delete | Trung bình-Thấp — value through a GUI is limited by the Application layer not existing yet (does NOT claim Qt6 is the only path to value — CLI/scripting/API not ruled out) | Thấp | Thấp | Rất thấp | Thấp | **Cao** (highest of the 4) |

Notable finding to carry forward: **highest Evidence Confidence
(Multi-select Delete) is not the same item as highest ROI** — evidence
strength and strategic value are independent axes.

## Bước 3.5 — Decision Readiness

| Work Item | Ready? | Reason |
|---|---|---|
| Dimension | ❌ No | No Domain Research done |
| Printing/PDF | ❌ No | `DG-RR-001` open |
| Qt6/UI wiring | ❌ No | Unaudited |
| Multi-select Delete | ❌ No | Implementation Risk looks low (infra exists), but Readiness is a different axis — no dedicated Domain Research has confirmed no Decision Gate remains (e.g. multi-entity delete order / Undo semantics could differ across reference CAD systems — unverified) |

## Bước 4 — Strategic Alignment (v0.1 Alpha criteria)

| Criterion | Status |
|---|---|
| Builds on Linux | Đạt |
| Builds on Windows | **Có chủ đích trì hoãn** — confirmed both by `ROADMAP_EXECUTION.md`'s own text and by the actual `.github/workflows/*.yml` (only `ubuntu-24.04`, no Windows leg at all) |
| CI green | Đạt |
| Unit tests for core geometry | Đạt |
| Open and save a basic DXF file | Đạt |
| Display Line/Circle/Arc/Polyline | Đạt |
| Move/Copy/Delete | Đạt (Command layer; same GUI-deferrable reading already applied when DXF-EXPORT-001 was selected) |
| README and developer documentation up to date | **Không đạt** — verified: `README.md`'s "Current project status" still reads "Phase: Spiral 2 (in progress — Layer System done)", stale by 6+ sprints (actual state has passed Spiral 3/4/5 and all of Epic 4 Editing + Epic 6 DXF Export) |

**Conclusion**: no evidence of a large remaining functional gap for v0.1
Alpha. None of the 4 A1/promoted candidates (Dimension, Printing/PDF,
Qt6, Multi-select Delete) are named in or clearly serve the v0.1 Alpha
criteria — all are post-v0.1 work. The one concrete, verified, cheap gap
(README) is explicitly **not** an Official Work Item — it's A3
Documentation Maintenance, out of Sprint Selection scope by the project's
own A1/A2/A3 boundary.

## Sprint Selection

**No Official Work Item selected.** All 4 candidates are Not Ready; none
are strategically urgent for v0.1. This is a valid Roadmap Review
outcome — "not selecting" is itself evidence-based, not indecision.

## Immediate next actions (A3, not a Sprint)

1. Update `README.md`'s "Current project status" to reflect actual state
   (through Sprint 6 / Epic 6).
2. Close `DD-001` in `DESIGN_DEBT.md` (mark resolved by REFACTOR-001).

## Sprint 7 — reframed

Once the A3 items above are closed, Sprint 7 opens as **"Sprint 7 —
Discovery"**, topic not yet chosen. Its only goal: move exactly one
Official Work Item from Not Ready to Ready. Which item gets that
investment is itself a decision requiring evidence (expected cost vs.
value of closing its specific Readiness gap) — not to be chosen by
"Unlock is big" / "it's cheap" / "it's easy" / "CAD always needs this"
reasoning, each of which was explicitly considered and rejected as
insufficient justification during this Review.
