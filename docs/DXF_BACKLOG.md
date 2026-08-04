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

---

## DXF-ROBUST-003b

**Title:** Context-aware DXF parse diagnostics (which section, which
group code, which line) and a structured `DxfErrorCode` taxonomy.

**Priority:** Low

**Status:** Deferred -- only take this up when a concrete consumer
needs it (see Trigger below)

**Current behaviour:** `Tokenizer::Good()` now correctly distinguishes
"clean end of stream" from "malformed/truncated data" (shipped as
DXF-ROBUST-003a), and `ParseDxfStream` uses that to pick a more
accurate top-level message. But the error channel is still a single
`foundation::string`: no line number, no "which section was being
read", no "which group code was last seen", and no machine-checkable
error code -- just prose. A caller that wants to react differently to
different failure kinds (a UI showing a different icon per error type,
a batch importer that wants to tally failures by category, or reliable
error-message testing that doesn't depend on wording) has no way to do
that today except parsing the message string.

**Proposed behaviour:** Two related but separable pieces of future
work, from the design discussion that scoped this out of
DXF-ROBUST-003a:
1. Position/context tracking in `Tokenizer`/`ParseDxfStream` (current
   line number, current section, last group code read) so error
   messages can say e.g. "unexpected end of file after group code 10"
   instead of the current section-level-only message.
2. A structured `DxfErrorCode` enum (e.g. `UnexpectedEof`,
   `MalformedGroupCode`, `MalformedNumber`, `MissingEndSec`,
   `MissingEntitiesSection`, `UnsupportedEntity`, ...) alongside (or
   instead of) the current `foundation::string` error, so callers can
   `switch`/compare on a code rather than matching message text.

**Reason:** Deferred rather than folded into DXF-ROBUST-003a because
(1) is new capability (position tracking doesn't exist in any form
today, not a refactor of something broken) and (2) is a public
API-breaking change (`expected<Document, foundation::string>` would
become `expected<Document, DxfError>` or similar) with **no concrete
consumer yet** -- no UI, no batch importer, no test today depends on
distinguishing error kinds programmatically. Per `docs/AI-Working-
Agreement.md` rule 3, building this ahead of a real use case is
exactly the kind of premature abstraction this project has
deliberately avoided before (see that rule's own `DxfImporter`/
`DxfExporter`/... example). Bundling it with DXF-ROBUST-003a would
have turned a ~15-line, non-breaking fix into a 150-300 line,
API-breaking one for no immediate payoff.

**Trigger to revisit:** A real, concrete need appears -- e.g. a UI
needs to show different error treatment per failure kind, a batch/
bulk DXF importer needs to tally or filter failures by category, or a
public API consumer needs to branch on error kind rather than display
text. Until then, `ParseDxfStream`'s current section-level messages
(now at least accurately distinguishing malformed/truncated from
"genuinely well-formed but missing ENDSEC/ENTITIES") are enough.

---

## DXF-LAYER-PROPS-002

**Title:** Decide the unit/semantics mapping for DXF lineweight (group
code `370`) before importing it into `Layer::LineWeight()`.

**Priority:** Low

**Status:** Deferred -- needs a design decision before implementation,
not a drop-in fix

**Current behaviour:** DXF's `370` lineweight code is an *enumerated*
integer in hundredths of a millimeter (0, 5, 9, 13, 15, 18, ..., 200),
plus three sentinel values (`-1`=BYLAYER, `-2`=BYBLOCK, `-3`=DEFAULT)
that don't represent a thickness at all. `Layer::LineWeight()` is a
plain, unitless `double` that `RenderToSvg` passes straight through as
SVG `stroke-width` (default `1.0`). There is no existing conversion
between "hundredths of a mm" and "SVG stroke-width units" anywhere in
the codebase, and no precedent for how a sentinel value should be
handled at the layer-table level (BYLAYER/BYBLOCK only make sense as
an *entity-level* override choice, which doesn't exist yet either --
see DXF-LAYER-PROPS-003 below).

**Proposed behaviour:** Not decided -- this entry exists to record the
question, not answer it. At minimum needs: a chosen scale factor (or
explicit "1 hundredth-mm = 1 SVG unit" passthrough, if that's judged
good enough for now), and a decision on what to do with a `370` value
of `-1`/`-2`/`-3` when read from a `TABLES`/`LAYER` record specifically
(entity-level BYLAYER/BYBLOCK is out of scope here regardless -- see
DXF-LAYER-PROPS-003).

**Reason:** Deferred out of DXF-LAYER-PROPS-001 specifically because
implementing it without deciding the unit question first would mean
guessing at a scale factor with no evidence for what's "visually
right" -- exactly the kind of unverified formula
`docs/AI-Working-Agreement.md`'s Principle 2 (independently verify
error-prone formulas, per the bulge-to-arc precedent in DXF-002) warns
against, just for a unit conversion instead of a geometric one.

**Trigger to revisit:** A design decision is made for the unit
mapping (possibly informed by how a real target renderer/viewer -- SVG
in a browser -- is meant to interpret `stroke-width` at the scale this
project's documents are drawn at), or a real DXF file's lineweight
being visibly wrong now that DXF-LAYER-PROPS-001 has shipped makes the
gap concrete rather than theoretical.

---

## DXF-LAYER-PROPS-003

**Title:** Entity-level appearance override (DXF group codes `62`/`6`/
`370` on an entity itself, not just its layer) -- and, as a
prerequisite, BYLAYER/BYBLOCK precedence semantics.

**Priority:** Low

**Status:** Deferred -- separate Sprint; changes the `Entity` data
model, not just the importer

**Current behaviour:** `document::Entity` (`Document.hpp`) has exactly
three fields: `id`, `shape`, `layer`. There is no field anywhere to
hold a per-entity color/linetype/lineweight override. Confirmed by
probe: a `LINE` entity with its own group code `62` (color) set to a
value different from its layer parses successfully but the `62` value
is never even read -- `ParseDxfStream`'s `LINE` branch only looks at
codes `8`/`10`/`11`/`20`/`21`. Same probe confirmed the `256`
(BYLAYER) sentinel is likewise never read -- there's currently nothing
for it to be a sentinel *for*.

**Proposed behaviour:** Not decided. Would need, at minimum: an
`std::optional<...>` (or similar) override field (or fields) added to
`Entity`; a decision on how `RenderToSvg` resolves precedence (entity
override present -> use it; absent, or explicitly BYLAYER -> fall
through to the entity's layer, exactly like today's only path; a DXF
`0`/BYBLOCK value has no meaning without `INSERT`/`BLOCKS` support,
which doesn't exist -- see the audit's own conclusion that BYBLOCK
stays out of scope entirely until block references exist).

**Reason:** Deferred as its own item, separate from
DXF-LAYER-PROPS-001, because it changes `document::Entity` -- a type
used throughout the Document/Render/Selection/Transform/HitTest code,
not something local to the DXF importer -- which is a materially
bigger, more architecturally-visible change than "read two more layer-
level properties into fields that already exist." Bundling it with
DXF-LAYER-PROPS-001 would turn a small, evidenced, non-API-changing
Sprint into a data-model change with no separately-verified need yet.

**Trigger to revisit:** A real DXF file relying on per-entity color/
linetype override is reported to import visually wrong even with
DXF-LAYER-PROPS-001's layer-level color/linetype import already
shipped (i.e. the layer-level fix alone isn't enough for that file),
or a deliberate decision to pursue fuller DXF visual fidelity as its
own goal.

---

## Explicitly not backlogged (per the DXF-004 appearance audit)

Recorded here so a future audit doesn't have to re-derive why these
are absent, without giving them their own numbered entries (that would
overstate them as planned work):

- **BYBLOCK sentinel handling** -- meaningless without `INSERT`/
  `BLOCKS` support, which doesn't exist and isn't proposed by any
  entry above. Revisit only if/when block references become a real
  item.
- **True color (24-bit RGB, group code `420`)** -- ACI (`62`, via
  DXF-LAYER-PROPS-001) already covers the demonstrated gap; no
  evidence anything needs the extra precision true color would add.
- **Plot style** -- no concept of plot styles exists anywhere in this
  codebase; nothing here proposes adding one.
