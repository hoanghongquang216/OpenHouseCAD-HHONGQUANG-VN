# AI Working Agreement

This document applies to **any AI assistant** working on this
repository -- Claude, ChatGPT/Codex, GitHub Copilot agents, or any
other tool with write access. It exists because every rule below was
learned the hard way, from a real incident in this project's own
history. If you are an AI reading this before making changes: follow
it. If you are a human reviewing an AI's work: hold it to this
standard.

This is a companion to `docs/ENGINEERING_PRINCIPLES.md` (general
principles) and `docs/CODING_STANDARD.md` (code style) -- this
document is specifically about **how an AI assistant should behave**
when given autonomy over this codebase.

## 1. A delivery is not done until there is proof

**The rule:** after delivering code, do not consider the task complete
-- and do not move on to the next task -- until you have seen explicit
confirmation of:
1. `./scripts/verify.sh --build-type Debug --fresh` passing, AND
2. `./scripts/verify.sh --build-type Release --fresh` passing, AND
3. `git push` succeeding (with the resulting commit hash visible).

A short affirmative reply ("ok", "done", "yes") from the user is
**not** sufficient evidence, no matter how the conversation is
flowing.

**Why this rule exists:** `TRF-002` (Rotate) and `TRF-003` (Scale)
were fully designed, implemented, and verified locally -- but were
never actually committed. This went undetected for multiple work
sessions because delivery confirmation wasn't required consistently.
The gap was only found when a later Spiral's build failed with
"RotateEntity was not declared in this scope", and `git log --all --
<path>` showed the file had only ever been touched by one earlier
commit. Recovering required re-verifying and re-committing two
Spirals' worth of work, and fixing a tag that had been created against
the wrong commit. See `CHANGELOG.md`'s `[spiral-4-transform]` entry
for the full account.

## 2. Verify claims with running code, not with reasoning alone

**The rule:** any nontrivial formula, algorithm, or "this should work"
claim -- especially anything involving floating-point arithmetic,
sign conventions, or geometric transforms -- must be verified by
actually compiling and running code that checks it against known or
independently-computed expected values, before it is presented as
correct. Mental derivation, however careful, is not sufficient on its
own.

**Why this rule exists:** several real bugs in this project were only
caught this way, not by inspection:
- A naive Undo/Redo design (store parameters, recompute the inverse
  transform) was demonstrated to drift by ~1.4e-14 per cycle for an
  off-origin pivot -- invisible by inspection, obvious once run.
- A draft `HitResult` design caching a raw `Entity*` was shown to
  produce silently wrong values (`-0.0` instead of `0.0`) after the
  underlying vector reallocated -- undefined behavior that did not
  crash, and would have been very hard to diagnose later.
- The DXF bulge-to-arc formula (LWPOLYLINE curved segments) has a
  sign-handling step that, done "the natural-looking way" (taking
  `abs()` of the radius too early), produces a mirror-image arc that
  *looks* like reasonable geometry rather than an obvious error. It
  was only confirmed correct after checking it against 20+ randomized
  `(point, point, bulge)` triples.
- `Tolerance.hpp`'s `NearlyEqual`/`IsZero` compiled and passed every
  local test under GCC, and were still broken under Clang+libc++ (a
  non-`constexpr` library `abs()` overload silently won overload
  resolution) -- only caught because CI ran a second compiler.

## 3. Don't build abstraction ahead of a second real need

**The rule:** before introducing a new type, a new layer of
indirection, an options/config struct, or a class hierarchy where a
plain function currently works -- ask: is there at least **one
concrete, currently-existing use case**, or only a plausible-sounding
future one? If only the latter, don't build it yet. Note *why* in a
comment (`TODO(SpiralN): ...`), so the deferred decision is visible
and deliberate, not silently forgotten.

**Why this rule exists:** this project has deferred exactly this kind
of premature abstraction multiple times, each time avoiding real
wasted effort: `LayerId` (deferred until layer rename is a real
feature), a structured `Color` type (deferred until a second render
backend exists), `Ellipse2` (deferred until non-uniform scale is
needed). Most recently, a proposal for Spiral 6 (DXF-000: a full
`DxfImporter`/`DxfExporter`/`DxfReader`/`DxfWriter`/
`DxfEntityFactory`/`DxfVersion`/`ImportOptions`/`ExportOptions`
architecture) was set aside for the same reason -- there is exactly
one format (DXF) and no Export requirement yet, so there is no second
real use case to design the abstraction *against*. The existing plain
function (`ParseDxfFile`) was extended in place instead (see DXF-002).

## 4. When you find a branch, commit, or file you don't recognize: stop and ask, don't guess

**The rule:** if you encounter a branch, commit, or piece of code that
doesn't match what you (this AI, in this conversation) produced, and
you cannot verify who created it or why -- say so explicitly, and ask
the human rather than assuming a story that makes it sound
intentional. Never delete such work without confirming it's genuinely
unwanted; never merge it without independently verifying it's correct
and understanding what it does.

**Why this rule exists:** this repository has, at different times, had
a `codex/transform-kernel-foundation` branch (334 commits, an entirely
different Document architecture) and a `feature/m0-bootstrap-doctor`
branch with commits neither the human nor this AI recognized
authoring, merged into `main` via a PR neither side remembers
approving. Both turned out to be real, unproblematic work from other
tooling -- but that was established by checking, not assuming. The
`codex/...` branch was renamed to `research/codex-transform-kernel-
foundation` (kept, not merged, not deleted) once its scale was
understood via `git log`/`git diff`, not guesswork.

## 5. Architecture decisions go in one place: `docs/ARCHITECTURE_DECISION_RECORDS/`

Do not create a second location for design documentation (e.g. a
`doc/planning/` folder) -- even if it feels like a lighter-weight place
to put a smaller decision. One canonical location, indexed in
`docs/ARCHITECTURE_DECISION_RECORDS/README.md`, is what keeps decisions
findable. If a decision is real but small, it can be a short ADR; it
doesn't need a second, competing convention.

## 6. Prefer the simplest structure that the CURRENT scope justifies

This project's modules are all `INTERFACE` (header-only) CMake
libraries, by deliberate, consistent choice -- not because compiled
static libraries are wrong, but because nothing here has yet needed
what a compiled library provides. If you're proposing a structural
change (a new library type, a new module boundary, a new build target
kind) that the rest of the codebase doesn't already do, that proposal
itself needs a concrete justification, not just "it's more standard" /
"it'll help later."

## 7. Every test failure is either a real bug or a wrong test -- find out which, don't just make it pass

When a `static_assert` or `OH_CHECK` fails, the instinct to "fix the
number until it passes" is exactly backwards. Independently recompute
the expected value (by hand, or with a different tool -- e.g. Python,
as used throughout this project's geometry work) before touching the
implementation. Several bugs in this project's own test suite (not its
implementation) were caught this way -- a hand-typed expected value
that was simply arithmetic-wrong, found because the implementation's
real output didn't match it and the discrepancy was investigated
rather than "corrected" by copying the implementation's output back
into the test.

## 8. This document itself: keep it evidence-based

Every rule above is grounded in a specific incident from this
project's real history, not abstract best-practice advice. If you add
a rule here, add the incident that motivated it. If a rule turns out
not to reflect how this project actually needs to work, revise it --
but note why, the same way `docs/ARCHITECTURE_DECISION_RECORDS/` and
`CHANGELOG.md` document reasoning changes elsewhere in this project.
