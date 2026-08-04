# DXF Backlog

Deferred DXF-specific decisions and future work. Each entry: what's
deferred, why, and what would trigger picking it up.

This is intentionally flat (`docs/DXF_BACKLOG.md`, not a `backlog/`
subdirectory) -- a subdirectory implies grouping multiple files, which
isn't justified until a second module has a comparable backlog to
group alongside this one. See `docs/AI-Working-Agreement.md` rule 3.

For decisions already made and shipped, see `CHANGELOG.md`. This file
is only for things NOT yet implemented.

---

## PERF-001

**Title:** Optimize `EntityChunk` group-code lookup.

**Priority:** Low

**Status:** Deferred

**Reason:** `FindDouble()`/`FindString()` (in `DxfReader.hpp`) perform a
linear scan over an entity's group-code pairs. The current
implementation is acceptable for the entity sizes DXF files actually
have (a handful to a few dozen group codes per entity). Optimizing
this now would be exactly the kind of premature optimization
`docs/ENGINEERING_PRINCIPLES.md`/`docs/AI-Working-Agreement.md` argue
against -- there is no evidence yet that this scan is a real
bottleneck for any drawing size this project has actually imported.

**Trigger to revisit:** importing a real-world drawing where profiling
shows entity-chunk lookup as a measurable fraction of total import
time.

---

## DXF-ROBUST-001

**Title:** Best-effort entity import (broader scope than DXF-002's
current per-entity skip behavior).

**Priority:** Medium

**Status:** Deferred -- CONTENT INCOMPLETE, needs follow-up

**Current behaviour:**

*(This entry was truncated in the source material it was transcribed
from -- everything below "Current behaviour:" needs to be supplied
before this item is actionable. Do not implement against this entry
until it's complete; a partial description risks solving the wrong
problem.)*

**Proposed behaviour:** _TODO -- not yet provided._

**Reason:** _TODO -- not yet provided._

**Trigger to revisit:** _TODO -- not yet provided._
