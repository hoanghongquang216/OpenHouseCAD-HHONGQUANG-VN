#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Expected.hpp>
#include <openhouse/foundation/String.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>
#include <openhouse/math/Angle.hpp>

#include <cctype>
#include <fstream>
#include <istream>
#include <optional>
#include <string>
#include <vector>

namespace openhouse::dxf {

// DXF's ASCII format is a flat sequence of (integer group code, string
// value) pairs, two lines each. What the value MEANS depends entirely on
// context (which section, which entity type, which code) -- there is no
// self-describing type tag beyond the group code number itself. This is
// the raw pair, before any entity-specific interpretation.
struct GroupCodePair {
    int code{};
    foundation::string value;
};

// Reads raw (code, value) pairs from a DXF ASCII stream. Purely
// mechanical -- knows nothing about sections or entities, just the two-
// line-per-pair tokenization. Trims a trailing '\r' from each value
// line, since real-world DXF files are commonly CRLF-terminated
// regardless of platform.
class Tokenizer {
public:
    explicit Tokenizer(std::istream& in) noexcept : in_(&in) {}

    // Returns std::nullopt at end of stream. A malformed pair (a code
    // line present but no matching value line, i.e. the file ends mid-
    // pair) also yields std::nullopt -- callers that need to distinguish
    // "clean EOF" from "truncated/malformed" should check Good() after
    // Next() returns nullopt (see Good()'s own comment: this is now an
    // actual, working distinction, not just a comment's promise).
    //
    // A blank (or whitespace-only) *code* line is skipped rather than
    // treated as end-of-stream: real-world DXF files occasionally pick
    // up a stray blank line (naive line-ending conversion, manual
    // editing, some non-CAD export paths) between otherwise well-formed
    // pairs, and a group code is never legitimately blank. Terminating
    // tokenization there -- as an earlier version of this function did
    // -- silently drops everything after the blank line for the rest of
    // the file, which (depending on where the entities section's ENDSEC
    // happened to fall) surfaced either as an outright wrong "missing
    // ENDSEC" parse error or, worse, a successful-looking parse that had
    // quietly lost entities. Skipping is safe because it can never
    // consume a legitimate pair: a pair's code line is always a number.
    [[nodiscard]] std::optional<GroupCodePair> Next();

    // True as long as nothing has gone wrong other than reaching a
    // clean end of stream. Turns false the moment Next() hits either
    // a genuinely truncated pair (a code line with no following value
    // line) or a malformed group code (not a bare integer) -- both
    // mean the stream stopped for a real reason, not because the file
    // is simply over. Meaningful after Next() returns std::nullopt;
    // before that (or after a successful Next()) it's simply "nothing
    // has gone wrong yet".
    //
    // DXF-ROBUST-003a: an earlier version of this method computed
    // `in_->eof() || in_->good()`, which -- because a failed
    // std::getline() sets eofbit regardless of *why* it failed -- was
    // true in both the "clean EOF" and "truncated mid-pair" cases and
    // so could never actually distinguish them, contradicting Next()'s
    // own doc comment. This tracks the real reason directly instead of
    // inferring it from stream flags.
    [[nodiscard]] bool Good() const noexcept { return good_; }

private:
    std::istream* in_;
    bool good_ = true;
};

// A parsed DXF entity, still in "generic" form (matches document::Shape's
// closed variant) before being wrapped into a document::Entity with its
// layer. Kept separate from document::Entity because DXF parsing needs a
// place to report per-entity errors (an unsupported entity type, or a
// malformed one) without necessarily aborting the whole file.
struct ParsedEntity {
    document::Shape shape;
    foundation::string layer;
};

// Parses everything between (0 SECTION / 2 ENTITIES) and (0 ENDSEC) into
// a Document.
//
// Scope (deliberately limited, per
// docs/ARCHITECTURE_DECISION_RECORDS/ADR-0004 and
// docs/ROADMAP_EXECUTION.md's Spiral 3 entry): only LINE, CIRCLE, and
// ARC entities are recognized. Any other entity type encountered (TEXT,
// DIMENSION, INSERT, LWPOLYLINE, etc. -- all of which appear in real-
// world DXF files) is silently SKIPPED, not treated as an error --
// real DXF files routinely contain entity types beyond whatever a given
// reader supports, and failing the entire import over one unsupported
// entity would make this importer useless on anything but hand-crafted
// test files. LWPOLYLINE support specifically is deferred to a follow-
// up task (DXF-002), not because it's unimportant, but because it needs
// its own design decision (a variable-length vertex list) that
// shouldn't be rushed into this first slice.
//
// Genuine errors (malformed group-code pairs, an entity missing a
// group code it requires, a value that isn't a valid number where a
// number is expected) DO fail the parse -- returning a description via
// the error channel rather than silently producing a wrong or partial
// Document.
[[nodiscard]] foundation::expected<document::Document, foundation::string> ParseDxfStream(
    std::istream& in);

[[nodiscard]] foundation::expected<document::Document, foundation::string> ParseDxfFile(
    const foundation::string& path);

// ============================================================================
// Implementation. Header-only (inline), matching this project's existing
// convention (foundation/geometry/math/document/render/testing are all
// header-only INTERFACE libraries) -- DXF parsing has no template
// parameters and doesn't strictly need to be header-only, but introducing
// a compiled-library build target would be new build-system surface for
// this one module alone, for no benefit this project currently needs.
// ============================================================================

namespace detail {

// Trims a trailing '\r' (and, defensively, any other trailing
// whitespace) -- see Tokenizer's own comment on CRLF handling.
inline foundation::string TrimTrailing(foundation::string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' ||
                           s.back() == '\t')) {
        s.pop_back();
    }
    return s;
}

// Leading whitespace also needs trimming: DXF group-code lines are
// commonly right-aligned with leading spaces (e.g. "  0" for a group
// code, produced by many real-world DXF writers), and std::stoi/from_chars
// on a string with leading spaces would either fail or silently succeed
// depending on the function used -- trim explicitly rather than rely on
// that.
inline foundation::string Trim(foundation::string s) {
    std::size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t')) {
        ++start;
    }
    s.erase(0, start);
    return TrimTrailing(std::move(s));
}

struct EntityChunk {
    foundation::string type; // "LINE", "CIRCLE", "ARC", "LAYER", or anything else
    std::vector<GroupCodePair> codes; // everything after the initial (0, type) pair
};

// Splits pairs[begin, end) into chunks, one per record: a bare group
// code 0 starts a new chunk, everything up to the next 0 belongs to
// it. Shared by ENTITIES-section entity splitting and TABLES/LAYER
// table-record splitting (DXF-LAYER-PROPS-001) -- both are the exact
// same convention applied to a different section, so this is one
// function with two call sites rather than the loop duplicated. A
// stray non-zero-code pair before the first (0, ...) in the range is
// silently ignored -- it can't belong to any record, so there's
// nothing meaningful to report it against (same reasoning as
// ParseDxfStream's original entity-splitting loop, which this
// replaces without changing behavior).
[[nodiscard]] inline std::vector<EntityChunk> SplitIntoChunks(
    const std::vector<GroupCodePair>& pairs, std::size_t begin, std::size_t end) {
    std::vector<EntityChunk> chunks;
    for (std::size_t i = begin; i < end; ++i) {
        if (pairs[i].code == 0) {
            chunks.push_back(EntityChunk{pairs[i].value, {}});
        } else if (!chunks.empty()) {
            chunks.back().codes.push_back(pairs[i]);
        }
    }
    return chunks;
}

// Parses `text` as a double, requiring the ENTIRE string to be valid
// numeric syntax -- unlike a bare std::stod() call, which silently
// stops at the first character it can't parse and returns whatever it
// managed to consume up to that point. A DXF numeric field with
// trailing garbage (e.g. a corrupted or accidentally-concatenated
// value like "10.5abc") is not a valid number and must be rejected
// the same way a value that doesn't parse at all is rejected --
// otherwise the caller silently gets a plausible-looking but wrong
// number instead of an error. See DXF-ROBUST-002 in
// docs/DXF_BACKLOG.md for the case this closes.
[[nodiscard]] inline std::optional<double> ParseStrictDouble(const foundation::string& text) {
    if (text.empty()) {
        return std::nullopt;
    }
    std::size_t consumed = 0;
    double value = 0.0;
    try {
        value = std::stod(text, &consumed);
    } catch (...) {
        return std::nullopt;
    }
    if (consumed != text.size()) {
        return std::nullopt; // trailing characters after the number: reject, don't truncate-accept
    }
    return value;
}

// Finds a specific group code's value within a chunk, converted to
// double. Returns std::nullopt if the code isn't present or isn't a
// valid number -- callers turn a missing REQUIRED code into a
// foundation::expected error with a specific message; this function
// itself doesn't know which codes are required for which entity type.
inline std::optional<double> FindDouble(const EntityChunk& chunk, int code) {
    for (const auto& pair : chunk.codes) {
        if (pair.code == code) {
            return ParseStrictDouble(pair.value);
        }
    }
    return std::nullopt;
}

inline foundation::string FindString(const EntityChunk& chunk, int code,
                                      foundation::string_view fallback) {
    for (const auto& pair : chunk.codes) {
        if (pair.code == code) {
            return pair.value;
        }
    }
    return foundation::string(fallback);
}

// A single LWPOLYLINE vertex. `bulge` describes the segment FROM this
// vertex TO the next one (per DXF convention) -- 0 means a straight
// line, non-zero means an arc (see BulgeToArc below).
struct LwPolylineVertex {
    double x = 0.0;
    double y = 0.0;
    double bulge = 0.0;
};

// Extracts an LWPOLYLINE's vertex list from its chunk. Unlike LINE/
// CIRCLE/ARC (one value per group code), LWPOLYLINE repeats group code
// 10 once per vertex -- each 10 (X) starts a new vertex; subsequent 20
// (Y, required) and 42 (bulge, optional, defaults to 0) belong to that
// same vertex until the next 10 appears.
//
// Returns std::nullopt if the vertex data itself is malformed (a
// missing/unparseable X or Y) -- that's severe enough that nothing
// trustworthy can be built from this entity, so the caller skips it
// entirely (see DXF-002's design review: individual malformed entities
// are skipped, not fatal to the whole file). A malformed bulge value is
// treated more leniently -- it just falls back to 0 (a straight
// segment), since losing curvature information for one segment is a
// much smaller degradation than discarding the entity's position data
// entirely.
[[nodiscard]] inline std::optional<std::vector<LwPolylineVertex>> ExtractLwPolylineVertices(
    const EntityChunk& chunk) {
    std::vector<LwPolylineVertex> vertices;
    for (const auto& pair : chunk.codes) {
        if (pair.code == 10) {
            const auto x = ParseStrictDouble(pair.value);
            if (!x.has_value()) {
                return std::nullopt;
            }
            vertices.push_back(LwPolylineVertex{*x, 0.0, 0.0});
        } else if (pair.code == 20) {
            if (vertices.empty()) {
                return std::nullopt; // Y with no preceding X -- malformed
            }
            const auto y = ParseStrictDouble(pair.value);
            if (!y.has_value()) {
                return std::nullopt;
            }
            vertices.back().y = *y;
        } else if (pair.code == 42 && !vertices.empty()) {
            // Malformed bulge specifically (including trailing garbage
            // after an otherwise-valid number): leave it at the default
            // 0 (straight segment) rather than discarding the whole
            // entity -- see this function's own comment above. Uses the
            // same strict parse as X/Y so "1.0abc" is treated as
            // malformed here too, not silently accepted as 1.0.
            if (const auto bulge = ParseStrictDouble(pair.value); bulge.has_value()) {
                vertices.back().bulge = *bulge;
            }
        }
    }
    return vertices;
}

// Converts a bulge-defined polyline segment (P1 -> P2, signed bulge)
// into an Arc2. Returns std::nullopt only for a genuinely degenerate
// input (P1 and P2 coincide, so no chord/arc exists).
//
// The sign handling here was the single most error-prone part of
// DXF-002 (bulge's sign indicates CCW vs CW, and getting that backwards
// silently produces a mirror-image arc through the WRONG side of the
// chord -- a bug that looks like reasonable geometry, not a crash).
// Verified before integration against 20+ randomized (point, point,
// bulge) triples by independently reconstructing P1/P2 from the
// resulting Arc2 via its own start/end-angle evaluation and confirming
// they match the original input to within floating-point tolerance --
// see this Spiral's design-review discussion for the derivation.
[[nodiscard]] inline std::optional<geometry::Arc2d> BulgeToArc(geometry::Point2d p1,
                                                                 geometry::Point2d p2,
                                                                 double bulge) {
    const double dx = p2.x - p1.x;
    const double dy = p2.y - p1.y;
    const double chordLen = foundation::hypot(dx, dy);
    if (chordLen == 0.0) {
        return std::nullopt;
    }

    const double theta = 4.0 * foundation::atan(bulge); // signed, CCW-positive
    const double alpha = theta / 2.0;
    const double sinAlpha = foundation::sin(alpha);
    if (sinAlpha == 0.0) {
        return std::nullopt; // defensive; shouldn't occur for bulge != 0
    }

    // Deliberately kept SIGNED through this step (not abs'd yet) -- the
    // sign of signedRadius, combined with cos(alpha), is what correctly
    // places the center on the CW vs CCW side of the chord. Taking the
    // absolute value here (a natural-looking "radius must be positive"
    // simplification) was the actual bug caught during verification: it
    // collapses the CW and CCW cases onto the same center, which is
    // wrong.
    const double signedRadius = chordLen / (2.0 * sinAlpha);
    const double apothem = signedRadius * foundation::cos(alpha);

    const double ux = dx / chordLen;
    const double uy = dy / chordLen;
    const double perpX = -uy; // rotate chord direction +90 degrees (CCW)
    const double perpY = ux;

    const geometry::Point2d center{
        (p1.x + p2.x) / 2.0 + perpX * apothem,
        (p1.y + p2.y) / 2.0 + perpY * apothem,
    };
    const double radius = foundation::abs(signedRadius); // abs only at the very end

    const double startAngle = foundation::atan2(p1.y - center.y, p1.x - center.x);
    // endAngle is startAngle + theta directly, NOT atan2(p2 - center) --
    // the latter could differ from startAngle+theta by a multiple of
    // 2*pi depending on atan2's branch, which would silently produce
    // the wrong sweep direction/magnitude even though both angles
    // describe the same two points.
    const double endAngle = startAngle + theta;

    return geometry::Arc2d{center, radius, startAngle, endAngle};
}

// --- TABLES/LAYER import (DXF-LAYER-PROPS-001) --------------------------
//
// Everything below applies layer *appearance* (color, linetype) read
// from the DXF TABLES/LAYER section onto a Document's layers. Scope is
// deliberately limited to exactly these two properties -- see
// docs/DXF_BACKLOG.md's DXF-LAYER-PROPS-001 entry (this Sprint) and
// the DXF-004 appearance audit that produced it. Explicitly NOT here:
// layer visibility (a LAYER record's sign-encoded on/off state, and
// DXF's separate frozen/off distinction -- DXF-LAYER-PROPS-002/audit
// notes), lineweight (needs a unit-mapping decision first --
// DXF-LAYER-PROPS-002), entity-level overrides (needs an Entity data-
// model change -- DXF-LAYER-PROPS-003), and BYBLOCK/true-color (no
// evidenced need -- see DXF_BACKLOG.md's "Explicitly not backlogged").

// Maps a DXF ACI (AutoCAD Color Index) to an SVG-compatible color
// string. Deliberately covers only the 9 standard/most common indices
// -- ACI is a 256-entry palette; anything outside this small table is
// std::nullopt, and the caller leaves the layer's existing color
// (default "black") untouched rather than guessing. ACI 7 is DXF's
// "foreground" color (conventionally white on a dark viewer background
// or black on a light one); this project's SVG output always has an
// implicit light/white background (see SvgDocument.hpp and every demo
// under modules/render/examples), so 7 maps to "black" here -- a
// deliberate choice for this project's one render target, not a
// general-purpose ACI-7 rule.
[[nodiscard]] inline std::optional<foundation::string> AciToSvgColor(int aci) {
    switch (aci) {
        case 1: return foundation::string("red");
        case 2: return foundation::string("yellow");
        case 3: return foundation::string("#00ff00"); // green
        case 4: return foundation::string("cyan");
        case 5: return foundation::string("blue");
        case 6: return foundation::string("magenta");
        case 7: return foundation::string("black"); // see comment above
        case 8: return foundation::string("#414141"); // dark gray
        case 9: return foundation::string("#808080"); // light gray
        default: return std::nullopt;
    }
}

// Maps a DXF linetype NAME (free-form string -- "CONTINUOUS", "DASHED",
// "HIDDEN2", "CENTER2", "DASHDOT", ... often with a numeric scale
// suffix real DXF writers append) onto this project's own small closed
// LineType enum. Matched by substring, case-insensitively, rather than
// exact string, so a scale-suffixed name like "HIDDEN2" still maps
// sensibly. Order matters: the more specific compound patterns
// (DASHDOT/CENTER/PHANTOM) are checked before the plain "DOT"/"DASH"
// substrings they'd otherwise be misclassified by (e.g. "DASHDOT"
// contains "DOT"). Anything unrecognized -- including a name this
// project has simply never seen -- defaults to Continuous, matching
// Layer's own existing default; there is no "unknown linetype" error
// path for the same reason an unsupported entity type isn't an error
// (see this header's top-of-file comment on scope).
[[nodiscard]] inline document::LineType LineTypeNameToEnum(const foundation::string& name) {
    foundation::string upper = name;
    for (char& c : upper) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    if (upper.find("DASHDOT") != foundation::string::npos ||
        upper.find("DASH_DOT") != foundation::string::npos ||
        upper.find("CENTER") != foundation::string::npos ||
        upper.find("PHANTOM") != foundation::string::npos) {
        return document::LineType::DashDot;
    }
    if (upper.find("DOT") != foundation::string::npos) {
        return document::LineType::Dotted;
    }
    if (upper.find("DASH") != foundation::string::npos ||
        upper.find("HIDDEN") != foundation::string::npos) {
        return document::LineType::Dashed;
    }
    return document::LineType::Continuous;
}

// Reads the (optional) TABLES/LAYER section from the full tokenized
// stream and applies each record's color (group code 62) and linetype
// (group code 6) to the matching Document layer, via the existing
// Document::CreateLayer + Layer::SetColor/SetLineType API -- no new
// Document/Layer surface needed. Absent entirely (no TABLES section,
// no LAYER table within it, or either failing to close) is NOT an
// error: layers simply keep their existing default appearance, exactly
// as before this Sprint -- consistent with this header's established
// tolerance for sections/data outside what it's specifically looking
// for. This runs independently of the ENTITIES-section pass, so a
// malformed/missing TABLES section never affects ENTITIES parsing
// (which has its own, separate error handling), and vice versa.
inline void ApplyLayerTableProperties(const std::vector<GroupCodePair>& allPairs,
                                       document::Document& doc) {
    std::size_t tablesStart = allPairs.size();
    std::size_t tablesEnd = allPairs.size();
    for (std::size_t i = 0; i + 1 < allPairs.size(); ++i) {
        if (allPairs[i].code == 0 && allPairs[i].value == "SECTION" &&
            allPairs[i + 1].code == 2 && allPairs[i + 1].value == "TABLES") {
            tablesStart = i + 2;
            break;
        }
    }
    if (tablesStart == allPairs.size()) {
        return; // no TABLES section at all -- nothing to apply
    }
    for (std::size_t i = tablesStart; i < allPairs.size(); ++i) {
        if (allPairs[i].code == 0 && allPairs[i].value == "ENDSEC") {
            tablesEnd = i;
            break;
        }
        if (allPairs[i].code == 0 && allPairs[i].value == "SECTION") {
            // A new SECTION started before TABLES's own ENDSEC was
            // found -- TABLES doesn't close cleanly. Stop the search
            // here (tablesEnd stays allPairs.size(), the "doesn't
            // close" case below) rather than continuing on and
            // mistakenly treating a LATER, unrelated section's ENDSEC
            // (e.g. ENTITIES') as if it closed TABLES.
            break;
        }
    }
    if (tablesEnd == allPairs.size()) {
        return; // TABLES section never closes -- tolerate, don't apply
    }

    std::size_t layerTableStart = tablesEnd;
    std::size_t layerTableEnd = tablesEnd;
    for (std::size_t i = tablesStart; i + 1 < tablesEnd; ++i) {
        if (allPairs[i].code == 0 && allPairs[i].value == "TABLE" &&
            allPairs[i + 1].code == 2 && allPairs[i + 1].value == "LAYER") {
            layerTableStart = i + 2;
            break;
        }
    }
    if (layerTableStart == tablesEnd) {
        return; // TABLES has no LAYER table -- nothing to apply
    }
    for (std::size_t i = layerTableStart; i < tablesEnd; ++i) {
        if (allPairs[i].code == 0 && allPairs[i].value == "ENDTAB") {
            layerTableEnd = i;
            break;
        }
    }

    for (const auto& chunk : SplitIntoChunks(allPairs, layerTableStart, layerTableEnd)) {
        if (chunk.type != "LAYER") {
            continue; // shouldn't normally occur within a LAYER table; defensive
        }
        const foundation::string name = FindString(chunk, 2, "");
        if (name.empty()) {
            continue; // a nameless record can't be matched to anything
        }
        document::Layer& layer = doc.CreateLayer(name); // get-or-create, idempotent

        if (const auto aci = FindDouble(chunk, 62); aci.has_value()) {
            // A negative color value is DXF's encoding for "this layer
            // is off" -- the underlying ACI index is the magnitude.
            // Visibility import is explicitly out of scope for this
            // Sprint (see docs/DXF_BACKLOG.md); only the color itself
            // is applied here, so the sign is discarded via abs().
            if (const auto svgColor = AciToSvgColor(static_cast<int>(foundation::abs(*aci)));
                svgColor.has_value()) {
                layer.SetColor(*svgColor);
            }
        }

        if (const foundation::string linetypeName = FindString(chunk, 6, "");
            !linetypeName.empty()) {
            layer.SetLineType(LineTypeNameToEnum(linetypeName));
        }
    }
}

} // namespace detail


inline std::optional<GroupCodePair> Tokenizer::Next() {
    std::string codeLine;
    // Loop past blank code lines instead of treating one as
    // end-of-stream -- see this method's declaration comment. Each
    // iteration either finds a non-blank candidate code line or hits a
    // genuine getline() failure (true EOF), so this always terminates.
    for (;;) {
        if (!std::getline(*in_, codeLine)) {
            return std::nullopt;
        }
        codeLine = detail::Trim(codeLine);
        if (!codeLine.empty()) {
            break;
        }
    }

    std::string valueLine;
    if (!std::getline(*in_, valueLine)) {
        // Truncated file: a code line was successfully read, but the
        // stream ended before its matching value line arrived. This is
        // NOT a clean end-of-stream -- a code line always implies a
        // value line must follow -- so it's a real problem, not "the
        // file is simply over".
        good_ = false;
        return std::nullopt;
    }

    int code = 0;
    std::size_t codeConsumed = 0;
    try {
        code = std::stoi(codeLine, &codeConsumed);
    } catch (...) {
        // Not a number at all (e.g. "AB") -- malformed, not a clean end
        // of stream. See DXF-ROBUST-003a in docs/DXF_BACKLOG.md.
        good_ = false;
        return std::nullopt;
    }
    if (codeConsumed != codeLine.size()) {
        // Trailing garbage after the group code number (e.g. "10x") --
        // a group code is never legitimately anything but a bare
        // integer, so this is malformed input, not a code to silently
        // truncate-accept. See DXF-ROBUST-002 in docs/DXF_BACKLOG.md.
        good_ = false;
        return std::nullopt;
    }

    return GroupCodePair{code, detail::Trim(valueLine)};
}

inline foundation::expected<document::Document, foundation::string> ParseDxfStream(
    std::istream& in) {
    Tokenizer tokenizer(in);

    // --- Pass 1: tokenize the whole stream, find the ENTITIES section --------
    std::vector<GroupCodePair> allPairs;
    std::optional<GroupCodePair> pair;
    while ((pair = tokenizer.Next())) {
        allPairs.push_back(*pair);
    }
    // Whether tokenization stopped because the stream cleanly ended, vs.
    // a malformed group code or a truncated (code-line-with-no-value-
    // line) pair -- see Tokenizer::Good()'s comment. Only consulted
    // below, and only to pick a more accurate error MESSAGE for a
    // failure that was already going to happen -- it never turns a
    // would-have-succeeded parse into a failure. In particular,
    // malformed/truncated data in a section other than ENTITIES (e.g.
    // OBJECTS, encountered after ENTITIES's own ENDSEC was already
    // captured) still doesn't fail the parse, consistent with this
    // Spiral's documented scope of ignoring everything outside
    // ENTITIES.
    const bool tokenizedCleanly = tokenizer.Good();

    // Find "0 SECTION" / "2 ENTITIES" ... "0 ENDSEC" bracketing the
    // entities we care about. A DXF file has multiple sections (HEADER,
    // TABLES, BLOCKS, ENTITIES, OBJECTS); everything outside ENTITIES is
    // still ignored for this Spiral's geometry-import scope -- block
    // definitions in BLOCKS are not imported -- with one exception:
    // layer color/linetype ARE read from TABLES, separately, by
    // detail::ApplyLayerTableProperties below (DXF-LAYER-PROPS-001).
    std::size_t entitiesStart = allPairs.size();
    std::size_t entitiesEnd = allPairs.size();
    for (std::size_t i = 0; i + 1 < allPairs.size(); ++i) {
        if (allPairs[i].code == 0 && allPairs[i].value == "SECTION" &&
            allPairs[i + 1].code == 2 && allPairs[i + 1].value == "ENTITIES") {
            entitiesStart = i + 2;
            break;
        }
    }
    if (entitiesStart == allPairs.size()) {
        if (!tokenizedCleanly) {
            return foundation::unexpected<foundation::string>(
                "DXF stream ended unexpectedly (malformed group code or "
                "truncated data) before an ENTITIES section could be found");
        }
        return foundation::unexpected<foundation::string>(
            "No ENTITIES section found (looked for '0/SECTION' followed by '2/ENTITIES')");
    }
    for (std::size_t i = entitiesStart; i < allPairs.size(); ++i) {
        if (allPairs[i].code == 0 && allPairs[i].value == "ENDSEC") {
            entitiesEnd = i;
            break;
        }
    }
    if (entitiesEnd == allPairs.size() && entitiesStart < allPairs.size()) {
        if (!tokenizedCleanly) {
            return foundation::unexpected<foundation::string>(
                "DXF stream ended unexpectedly (malformed group code or "
                "truncated data) while reading the ENTITIES section, before "
                "its closing '0/ENDSEC' was found");
        }
        return foundation::unexpected<foundation::string>(
            "ENTITIES section is missing its closing '0/ENDSEC'");
    }

    // --- Pass 2: split the ENTITIES section into per-entity chunks -----------
    // Each entity starts with a (0, <TYPE>) pair; everything up to the
    // next (0, ...) belongs to that entity. A stray non-zero code
    // before any (0, TYPE) pair (malformed input) is silently ignored
    // by SplitIntoChunks -- it can't belong to any entity, so there's
    // nothing meaningful to report it against.
    const std::vector<detail::EntityChunk> chunks =
        detail::SplitIntoChunks(allPairs, entitiesStart, entitiesEnd);

    // --- Pass 3: build entities from recognized chunk types -------------------
    document::Document doc;
    for (const auto& chunk : chunks) {
        const foundation::string layer = detail::FindString(chunk, 8, document::Document::kDefaultLayerName);

        if (chunk.type == "LINE") {
            const auto x1 = detail::FindDouble(chunk, 10);
            const auto y1 = detail::FindDouble(chunk, 20);
            const auto x2 = detail::FindDouble(chunk, 11);
            const auto y2 = detail::FindDouble(chunk, 21);
            if (!x1 || !y1 || !x2 || !y2) {
                return foundation::unexpected<foundation::string>(
                    "LINE entity is missing a required coordinate (group code 10/20/11/21)");
            }
            doc.Add(geometry::Line2d{geometry::Point2d{*x1, *y1}, geometry::Point2d{*x2, *y2}},
                    layer);
        } else if (chunk.type == "CIRCLE") {
            const auto cx = detail::FindDouble(chunk, 10);
            const auto cy = detail::FindDouble(chunk, 20);
            const auto radius = detail::FindDouble(chunk, 40);
            if (!cx || !cy || !radius) {
                return foundation::unexpected<foundation::string>(
                    "CIRCLE entity is missing a required field (group code 10/20/40)");
            }
            doc.Add(geometry::Circle2d{geometry::Point2d{*cx, *cy}, *radius}, layer);
        } else if (chunk.type == "ARC") {
            const auto cx = detail::FindDouble(chunk, 10);
            const auto cy = detail::FindDouble(chunk, 20);
            const auto radius = detail::FindDouble(chunk, 40);
            const auto startDeg = detail::FindDouble(chunk, 50);
            const auto endDeg = detail::FindDouble(chunk, 51);
            if (!cx || !cy || !radius || !startDeg || !endDeg) {
                return foundation::unexpected<foundation::string>(
                    "ARC entity is missing a required field (group code 10/20/40/50/51)");
            }
            // DXF stores arc angles in DEGREES (codes 50/51) -- convert
            // via math::Angle rather than a raw *pi/180 multiplication
            // inline, for exactly the reason Angle exists: making the
            // unit conversion explicit and type-checked instead of a
            // bare number that could be mistaken for radians later.
            const double startRad = math::Angled::FromDegrees(*startDeg).Radians();
            const double endRad = math::Angled::FromDegrees(*endDeg).Radians();
            doc.Add(geometry::Arc2d{geometry::Point2d{*cx, *cy}, *radius, startRad, endRad},
                    layer);
        } else if (chunk.type == "LWPOLYLINE") {
            // LWPOLYLINE is not stored as a single Document entity --
            // it's decomposed into its constituent Line2/Arc2 segments
            // at import time (see DXF-002's design review), matching
            // Shape's existing closed set (variant<Line2, Circle2,
            // Arc2>) exactly rather than extending it for one entity
            // type. Each straight (bulge == 0) segment becomes a Line2;
            // each curved segment becomes an Arc2 via BulgeToArc.
            const auto vertices = detail::ExtractLwPolylineVertices(chunk);
            if (!vertices.has_value()) {
                // Malformed vertex data (bad/missing X or Y) -- skip
                // this entity, same as an unsupported entity type would
                // be (see this file's top-level comment on scope): a
                // single bad entity should not fail the whole import.
                continue;
            }

            const auto flagsValue = detail::FindDouble(chunk, 70);
            const bool closed =
                flagsValue.has_value() && (static_cast<int>(*flagsValue) & 1) != 0;

            // Degenerate: fewer than 2 vertices means not even one
            // segment can be formed. Skipped, not an error -- same
            // reasoning as above.
            if (vertices->size() < 2) {
                continue;
            }

            const std::size_t segmentCount = closed ? vertices->size() : vertices->size() - 1;
            for (std::size_t i = 0; i < segmentCount; ++i) {
                const auto& v1 = (*vertices)[i];
                const auto& v2 = (*vertices)[(i + 1) % vertices->size()];
                const geometry::Point2d p1{v1.x, v1.y};
                const geometry::Point2d p2{v2.x, v2.y};

                if (v1.bulge == 0.0) {
                    doc.Add(geometry::Line2d{p1, p2}, layer);
                } else if (const auto arc = detail::BulgeToArc(p1, p2, v1.bulge);
                           arc.has_value()) {
                    doc.Add(*arc, layer);
                } else {
                    // BulgeToArc only fails for coincident p1/p2 (a
                    // zero-length segment) -- fall back to a Line2 in
                    // that case too; it'll just be a degenerate,
                    // effectively invisible line rather than a lost
                    // segment.
                    doc.Add(geometry::Line2d{p1, p2}, layer);
                }
            }
        }
        // Any other entity type: recognized as a chunk, intentionally
        // skipped -- see this header's top-level comment on scope.
    }

    // --- Pass 4: apply layer color/linetype from TABLES, if present ----------
    // Independent of Pass 1-3's ENTITIES-only error handling above: a
    // missing or malformed TABLES section never fails this function --
    // see ApplyLayerTableProperties's own comment. DXF-LAYER-PROPS-001.
    detail::ApplyLayerTableProperties(allPairs, doc);

    return doc;
}

inline foundation::expected<document::Document, foundation::string> ParseDxfFile(
    const foundation::string& path) {
    std::ifstream in(path);
    if (!in) {
        return foundation::unexpected<foundation::string>("Could not open file: " + path);
    }
    return ParseDxfStream(in);
}

}

