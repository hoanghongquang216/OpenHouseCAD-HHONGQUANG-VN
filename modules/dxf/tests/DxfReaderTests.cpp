#include <openhouse/dxf/DxfReader.hpp>
#include <openhouse/testing/Check.hpp>

#include <cmath>
#include <cstdio>
#include <sstream>
#include <variant>

using namespace openhouse;

namespace {
constexpr double kPi = 3.14159265358979323846;

bool NearlyEqual(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}
} // namespace

static void TestParseLineEntity() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n100.0\n21\n50.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1);

    const auto* line = std::get_if<geometry::Line2d>(&result->Entities()[0].shape);
    OH_CHECK(line != nullptr);
    OH_CHECK(line->start.x == 0.0 && line->start.y == 0.0);
    OH_CHECK(line->end.x == 100.0 && line->end.y == 50.0);
    OH_CHECK(result->Entities()[0].layer == "0");
}

static void TestParseCircleEntity() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nCIRCLE\n8\nWalls\n10\n50.0\n20\n50.0\n40\n25.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1);

    const auto* circle = std::get_if<geometry::Circle2d>(&result->Entities()[0].shape);
    OH_CHECK(circle != nullptr);
    OH_CHECK(circle->center.x == 50.0 && circle->center.y == 50.0);
    OH_CHECK(circle->radius == 25.0);
    OH_CHECK(result->Entities()[0].layer == "Walls");

    // Layer "Walls" must have been auto-created (same mechanism as
    // Document::Add's existing behavior -- DXF import doesn't need its
    // own layer-creation logic, it just reuses Document::Add).
    OH_CHECK(result->FindLayer("Walls") != nullptr);
}

static void TestParseArcEntityConvertsDegreesToRadians() {
    // The most error-prone part of DXF import: group codes 50/51 are
    // DEGREES, but Arc2 stores radians. A bug here would silently
    // produce wildly wrong geometry without any parse error.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nARC\n8\n0\n10\n0.0\n20\n0.0\n40\n10.0\n50\n0.0\n51\n90.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());

    const auto* arc = std::get_if<geometry::Arc2d>(&result->Entities()[0].shape);
    OH_CHECK(arc != nullptr);
    OH_CHECK(NearlyEqual(arc->startAngle, 0.0));
    OH_CHECK(NearlyEqual(arc->endAngle, kPi / 2.0)); // 90 deg -> pi/2 rad, not 90.0
}

static void TestMultipleEntitiesAcrossLayers() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n21\n1.0\n"
        "0\nCIRCLE\n8\nWalls\n10\n0.0\n20\n0.0\n40\n1.0\n"
        "0\nARC\n8\nCenter\n10\n0.0\n20\n0.0\n40\n1.0\n50\n0.0\n51\n45.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 3);
    OH_CHECK(result->Layers().size() == 3); // "0", "Walls", "Center"
}

static void TestUnsupportedEntityTypeIsSkippedNotAnError() {
    // Real DXF files routinely contain entity types this importer
    // doesn't support yet (TEXT here as a stand-in for any of them).
    // The whole point of Spiral 3's scoped-down approach is that this
    // must NOT fail the entire import.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nTEXT\n8\n0\n1\nhello\n"
        "0\nLINE\n8\n0\n10\n1.0\n20\n2.0\n11\n3.0\n21\n4.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1); // only the LINE; TEXT silently skipped
}

static void TestMissingEntitiesSectionIsAnError() {
    std::istringstream dxf("0\nSECTION\n2\nHEADER\n0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(!result.has_value());
}

static void TestMissingRequiredGroupCodeIsAnError() {
    // LINE missing group code 21 (end.y).
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(!result.has_value());
}

static void TestCrlfLineEndingsAreHandled() {
    // DXF files in the wild are commonly CRLF-terminated regardless of
    // the reading platform.
    std::istringstream dxf(
        "0\r\nSECTION\r\n2\r\nENTITIES\r\n"
        "0\r\nLINE\r\n8\r\n0\r\n10\r\n5.0\r\n20\r\n5.0\r\n11\r\n15.0\r\n21\r\n15.0\r\n"
        "0\r\nENDSEC\r\n0\r\nEOF\r\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    const auto* line = std::get_if<geometry::Line2d>(&result->Entities()[0].shape);
    OH_CHECK(line != nullptr);
    OH_CHECK(line->start.x == 5.0); // a stray '\r' would corrupt this parse
}

static void TestEntityWithoutLayerCodeDefaultsToLayerZero() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nCIRCLE\n10\n0.0\n20\n0.0\n40\n5.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Entities()[0].layer == document::Document::kDefaultLayerName);
}

static void TestFileNotFoundIsAnError() {
    auto result = dxf::ParseDxfFile("/nonexistent_dxf_file_12345.dxf");
    OH_CHECK(!result.has_value());
}

static void TestEmptyEntitiesSectionProducesEmptyDocument() {
    std::istringstream dxf("0\nSECTION\n2\nENTITIES\n0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Empty());
    OH_CHECK(result->Layers().size() == 1); // just the default "0"
}

// DXF-003 (audit finding): a group code is never legitimately blank, so
// a stray blank line between two otherwise well-formed entities (from
// naive line-ending conversion, manual editing, etc.) must be skipped,
// not treated as end-of-stream. Before this fix, the tokenizer stopped
// reading right here, which made the ENTITIES section look like it was
// missing its ENDSEC -- a misleading error on a well-formed file, and
// (had ENDSEC appeared before the blank line in a different layout) a
// path to silently losing entities instead of erroring at all.
static void TestBlankLineWithinEntitiesSectionIsSkippedNotFatal() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n21\n1.0\n"
        "\n" // stray blank line between entities
        "0\nCIRCLE\n8\n0\n10\n5.0\n20\n5.0\n40\n2.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 2); // both LINE and CIRCLE, not just the LINE
}

static void TestMultipleConsecutiveBlankLinesAreSkipped() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n21\n1.0\n"
        "\n\n\n" // several stray blank lines in a row
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1);
}

static void TestTrailingBlankLinesAfterEofMarkerDoNotError() {
    // Some editors append trailing blank lines at the very end of a
    // file. Must still resolve to a clean end-of-stream, not an error.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n21\n1.0\n"
        "0\nENDSEC\n0\nEOF\n\n\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1);
}

// --- Strict numeric parsing (DXF-ROBUST-002) --------------------------
//
// A numeric group-code value with trailing garbage (e.g. a corrupted
// or accidentally-concatenated field) must be rejected the same way a
// value that doesn't parse at all is rejected -- not silently accepted
// up to the first unparseable character. See docs/DXF_BACKLOG.md.

static void TestRequiredFieldWithTrailingGarbageIsRejected() {
    // CIRCLE radius "5.0abc" must NOT be silently accepted as 5.0.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nCIRCLE\n8\n0\n10\n0.0\n20\n0.0\n40\n5.0abc\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(!result.has_value());
}

static void TestLineCoordinateWithTrailingGarbageIsRejected() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n1.0x\n20\n0.0\n11\n2.0\n21\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(!result.has_value());
}

static void TestGroupCodeWithTrailingGarbageIsRejected() {
    // "10x" instead of "10" as a group code line: a group code is
    // never legitimately anything but a bare integer.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10x\n1.0\n20\n0.0\n11\n2.0\n21\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(!result.has_value());
}

static void TestLwPolylineVertexCoordinateWithTrailingGarbageIsRejected() {
    // Unlike bulge (below), a malformed X/Y is severe enough that the
    // whole LWPOLYLINE entity is skipped -- consistent with existing
    // DXF-002 behavior for missing/unparseable X or Y.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLWPOLYLINE\n8\n0\n90\n2\n70\n0\n"
        "10\n0.0garbage\n20\n0.0\n"
        "10\n2.0\n20\n0.0\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n21\n1.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1); // only the LINE; the malformed LWPOLYLINE is skipped
}

static void TestLwPolylineBulgeWithTrailingGarbageFallsBackToStraight() {
    // Bulge keeps its existing lenient treatment: malformed (including
    // trailing garbage) falls back to 0 -- a straight segment -- rather
    // than discarding the whole entity, same as an unparseable bulge
    // was already handled before this fix.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLWPOLYLINE\n8\n0\n90\n2\n70\n0\n"
        "10\n0.0\n20\n0.0\n42\n1.0garbage\n"
        "10\n2.0\n20\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1);
    OH_CHECK(std::holds_alternative<geometry::Line2d>(result->Entities()[0].shape));
}

static void TestLegitimateSignedAndScientificNotationStillParse() {
    // Make sure strict parsing doesn't over-reject valid DXF numeric
    // syntax: leading '+', scientific notation.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nCIRCLE\n8\n0\n10\n+1.5e2\n20\n0.0\n40\n2.5\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    const auto* circle = std::get_if<geometry::Circle2d>(&result->Entities()[0].shape);
    OH_CHECK(circle != nullptr);
    OH_CHECK(NearlyEqual(circle->center.x, 150.0));
}

// --- Malformed/truncated vs. clean EOF (DXF-ROBUST-003a) --------------
//
// Tokenizer::Good() previously could never actually distinguish "the
// stream cleanly ended" from "tokenization stopped because something
// was wrong" (both left the underlying stream's eofbit set the same
// way). These tests lock in that the distinction is now real: a
// failure caused by malformed/truncated data gets a message that says
// so, while a file that tokenizes completely cleanly and simply never
// closes its ENTITIES section keeps the original, more specific
// message -- proving the two code paths are genuinely different, not
// just checking has_value() either way (which wouldn't catch a
// regression back to the old behavior).

static void TestMalformedGroupCodeMidEntitiesReportsUnexpectedEnd() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n21\n1.0\n"
        "AB\ngarbage\n" // malformed group code line, not a bare integer
        "0\nCIRCLE\n8\n0\n10\n5.0\n20\n5.0\n40\n2.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(!result.has_value());
    OH_CHECK(result.error().find("unexpectedly") != foundation::string::npos);
}

static void TestTruncatedMidPairReportsUnexpectedEnd() {
    // File ends right after a code line, before its value line arrives.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n21\n1.0\n"
        "0\nCIRCLE\n8\n0\n10");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(!result.has_value());
    OH_CHECK(result.error().find("unexpectedly") != foundation::string::npos);
}

static void TestCleanlyMissingEndsecKeepsOriginalMessage() {
    // No corruption anywhere -- the file just never closes its
    // ENTITIES section. Tokenization itself was clean, so this must
    // keep the ORIGINAL "missing ENDSEC" message, not the
    // malformed/truncated one -- this is what actually proves Good()
    // now tells the two cases apart instead of always agreeing.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n21\n1.0\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(!result.has_value());
    OH_CHECK(result.error().find("unexpectedly") == foundation::string::npos);
    OH_CHECK(result.error().find("ENDSEC") != foundation::string::npos);
}

static void TestMalformedTokenInLaterIgnoredSectionDoesNotFailParse() {
    // Corruption in a section this Spiral doesn't import from (OBJECTS)
    // must NOT fail the parse, as long as it comes after ENTITIES's own
    // ENDSEC was already found -- consistent with "everything outside
    // ENTITIES is ignored". This is the regression this fix must NOT
    // introduce: being stricter about ENTITIES-section corruption must
    // not make the parser less tolerant of irrelevant sections.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n21\n1.0\n"
        "0\nENDSEC\n"
        "0\nSECTION\n2\nOBJECTS\n"
        "AB\ngarbage\n" // malformed, but irrelevant to ENTITIES
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1);
}

// --- LWPOLYLINE (DXF-002) --------------------------------------------

static void TestLwPolylineClosedRectangleProducesFourLines() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLWPOLYLINE\n8\nWalls\n90\n4\n70\n1\n" // 70=1 -> closed
        "10\n0.0\n20\n0.0\n"
        "10\n10.0\n20\n0.0\n"
        "10\n10.0\n20\n10.0\n"
        "10\n0.0\n20\n10.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    // 4 vertices, closed -> 4 segments (including the wraparound one).
    OH_CHECK(result->Count() == 4);
    for (const auto& entity : result->Entities()) {
        OH_CHECK(std::holds_alternative<geometry::Line2d>(entity.shape));
        OH_CHECK(entity.layer == "Walls");
    }
}

static void TestLwPolylineOpenProducesOneFewerSegmentThanVertices() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLWPOLYLINE\n8\n0\n90\n3\n70\n0\n" // 70=0 -> not closed
        "10\n0.0\n20\n0.0\n"
        "10\n5.0\n20\n0.0\n"
        "10\n10.0\n20\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    // 3 vertices, open -> only 2 segments (no wraparound to vertex 0).
    OH_CHECK(result->Count() == 2);
}

// The single most important LWPOLYLINE test: a bulge=1 segment must
// produce an Arc2 matching the well-known "bulge 1 == semicircle"
// case, verified independently (Python + 20 randomized cases) before
// this formula was integrated -- see DXF-002's design review.
static void TestLwPolylineBulgeOneProducesSemicircleArc() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLWPOLYLINE\n8\n0\n90\n2\n70\n0\n"
        "10\n0.0\n20\n0.0\n42\n1.0\n" // vertex 0: bulge 1 for segment 0->1
        "10\n2.0\n20\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1);

    const auto* arc = std::get_if<geometry::Arc2d>(&result->Entities()[0].shape);
    OH_CHECK(arc != nullptr);
    OH_CHECK(NearlyEqual(arc->center.x, 1.0));
    OH_CHECK(NearlyEqual(arc->center.y, 0.0));
    OH_CHECK(NearlyEqual(arc->radius, 1.0));
}

static void TestLwPolylineMixedStraightAndCurvedSegments() {
    // Vertex 0->1: straight (bulge 0, default). Vertex 1->2: curved
    // (bulge != 0). Confirms both code paths coexist correctly within
    // a single entity.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLWPOLYLINE\n8\n0\n90\n3\n70\n0\n"
        "10\n0.0\n20\n0.0\n"                // vertex 0, no bulge -> straight to 1
        "10\n5.0\n20\n0.0\n42\n1.0\n"       // vertex 1, bulge 1 -> arc to 2
        "10\n7.0\n20\n0.0\n"                // vertex 2
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 2);
    OH_CHECK(std::holds_alternative<geometry::Line2d>(result->Entities()[0].shape));
    OH_CHECK(std::holds_alternative<geometry::Arc2d>(result->Entities()[1].shape));
}

// Per DXF-002's design review: a degenerate single-vertex polyline
// (open, so zero possible segments) is SKIPPED, not treated as a parse
// error for the whole file -- a well-formed entity appearing after it
// must still parse correctly.
static void TestLwPolylineSingleVertexIsSkippedNotFatal() {
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLWPOLYLINE\n8\n0\n90\n1\n70\n0\n"
        "10\n5.0\n20\n5.0\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n21\n1.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1); // only the LINE; the degenerate LWPOLYLINE is gone
    OH_CHECK(std::holds_alternative<geometry::Line2d>(result->Entities()[0].shape));
}

static void TestLwPolylineWithoutClosedFlagDefaultsToOpen() {
    // Group code 70 entirely absent -- must default to "not closed",
    // matching DXF's own documented default (flag bit unset).
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLWPOLYLINE\n8\n0\n90\n2\n"
        "10\n0.0\n20\n0.0\n"
        "10\n1.0\n20\n1.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1); // 2 vertices, open -> exactly 1 segment
}

// --- TABLES/LAYER import (DXF-LAYER-PROPS-001) -------------------------

static void TestLayerColorAndLinetypeImportedFromTablesLayerSection() {
    std::istringstream dxf(
        "0\nSECTION\n2\nTABLES\n"
        "0\nTABLE\n2\nLAYER\n"
        "0\nLAYER\n2\nWalls\n62\n1\n6\nCONTINUOUS\n"
        "0\nLAYER\n2\nCenter\n62\n5\n6\nCENTER2\n" // scale-suffixed name
        "0\nENDTAB\n0\nENDSEC\n"
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\nWalls\n10\n0.0\n20\n0.0\n11\n10.0\n21\n0.0\n"
        "0\nLINE\n8\nCenter\n10\n0.0\n20\n4.0\n11\n10.0\n21\n4.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());

    const document::Layer* walls = result->FindLayer("Walls");
    OH_CHECK(walls != nullptr);
    OH_CHECK(walls->Color() == "red"); // ACI 1
    OH_CHECK(walls->GetLineType() == document::LineType::Continuous);

    const document::Layer* center = result->FindLayer("Center");
    OH_CHECK(center != nullptr);
    OH_CHECK(center->Color() == "blue"); // ACI 5
    // "CENTER2" (a scale-suffixed real-world linetype name) must still
    // map via substring match, not require an exact "CENTER" string.
    OH_CHECK(center->GetLineType() == document::LineType::DashDot);
}

static void TestLayerColorNegativeSignTreatedAsOffMagnitudeStillApplies() {
    // A negative color value on a LAYER record is DXF's "this layer is
    // off" encoding -- visibility import is out of scope for this
    // Sprint, but the underlying ACI magnitude must still be read
    // correctly rather than left unmapped because of the sign.
    std::istringstream dxf(
        "0\nSECTION\n2\nTABLES\n"
        "0\nTABLE\n2\nLAYER\n"
        "0\nLAYER\n2\nHidden\n62\n-3\n" // -3 -> magnitude 3 -> green
        "0\nENDTAB\n0\nENDSEC\n"
        "0\nSECTION\n2\nENTITIES\n"
        "0\nCIRCLE\n8\nHidden\n10\n0.0\n20\n0.0\n40\n5.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    const document::Layer* hidden = result->FindLayer("Hidden");
    OH_CHECK(hidden != nullptr);
    OH_CHECK(hidden->Color() == "#00ff00"); // ACI 3 (green)
}

static void TestNoTablesSectionLeavesLayerAppearanceAtDefaults() {
    // Backward-compatibility guard: a DXF file with no TABLES section
    // at all (every DxfReaderTests fixture before this Sprint) must
    // behave exactly as before -- default appearance, no error.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\nWalls\n10\n0.0\n20\n0.0\n11\n10.0\n21\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    const document::Layer* walls = result->FindLayer("Walls");
    OH_CHECK(walls != nullptr);
    OH_CHECK(walls->Color() == "black");
    OH_CHECK(walls->GetLineType() == document::LineType::Continuous);
}

static void TestTablesWithoutLayerTableLeavesLayerAppearanceAtDefaults() {
    // A TABLES section can exist without a LAYER table inside it (e.g.
    // only LTYPE) -- must still fall through to defaults, not error.
    std::istringstream dxf(
        "0\nSECTION\n2\nTABLES\n"
        "0\nTABLE\n2\nLTYPE\n0\nENDTAB\n"
        "0\nENDSEC\n"
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\nWalls\n10\n0.0\n20\n0.0\n11\n10.0\n21\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    const document::Layer* walls = result->FindLayer("Walls");
    OH_CHECK(walls != nullptr);
    OH_CHECK(walls->Color() == "black");
}

static void TestUnrecognizedAciAndLinetypeFallBackToDefaultsNotError() {
    // ACI is a 256-entry palette and DXF linetype names are free-form;
    // this Sprint's mapping tables deliberately cover only the common
    // cases (see docs/DXF_BACKLOG.md). Anything outside them must fall
    // back to the existing defaults, not fail the parse.
    std::istringstream dxf(
        "0\nSECTION\n2\nTABLES\n"
        "0\nTABLE\n2\nLAYER\n"
        "0\nLAYER\n2\nWeird\n62\n200\n6\nSOME_CUSTOM_LT\n"
        "0\nENDTAB\n0\nENDSEC\n"
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\nWeird\n10\n0.0\n20\n0.0\n11\n10.0\n21\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    const document::Layer* weird = result->FindLayer("Weird");
    OH_CHECK(weird != nullptr);
    OH_CHECK(weird->Color() == "black"); // unrecognized ACI 200 -> default
    OH_CHECK(weird->GetLineType() == document::LineType::Continuous); // unrecognized name -> default
}

static void TestLayerTablePropertiesAppliedEvenIfNoEntityUsesThatLayer() {
    // A LAYER record for a layer name no entity ends up referencing is
    // harmless -- matches Document::CreateLayer's own get-or-create
    // idempotence, same as an entity referencing an as-yet-unseen
    // layer name auto-creates it.
    std::istringstream dxf(
        "0\nSECTION\n2\nTABLES\n"
        "0\nTABLE\n2\nLAYER\n"
        "0\nLAYER\n2\nUnused\n62\n2\n6\nDASHED\n"
        "0\nENDTAB\n0\nENDSEC\n"
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n10.0\n21\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    const document::Layer* unused = result->FindLayer("Unused");
    OH_CHECK(unused != nullptr);
    OH_CHECK(unused->Color() == "yellow"); // ACI 2
    OH_CHECK(unused->GetLineType() == document::LineType::Dashed);
}

static void TestUnclosedTablesSectionDoesNotAffectEntitiesImport() {
    // A TABLES section that never closes (no ENDTAB/ENDSEC) must not
    // affect ENTITIES parsing at all, and must not mistakenly treat a
    // LATER, unrelated section's ENDSEC (here, ENTITIES' own) as if it
    // had closed TABLES -- found and fixed during this Sprint's own
    // verification, not a hypothetical case.
    std::istringstream dxf(
        "0\nSECTION\n2\nTABLES\n"
        "0\nTABLE\n2\nLAYER\n"
        "0\nLAYER\n2\nWalls\n62\n1\n"
        // no ENDTAB, no ENDSEC for TABLES
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\nWalls\n10\n0.0\n20\n0.0\n11\n10.0\n21\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    OH_CHECK(result->Count() == 1);
    const document::Layer* walls = result->FindLayer("Walls");
    OH_CHECK(walls != nullptr);
    // Malformed TABLES data is NOT applied -- default color, not "red".
    OH_CHECK(walls->Color() == "black");
}

int main() {
    TestParseLineEntity();
    TestParseCircleEntity();
    TestParseArcEntityConvertsDegreesToRadians();
    TestMultipleEntitiesAcrossLayers();
    TestUnsupportedEntityTypeIsSkippedNotAnError();
    TestMissingEntitiesSectionIsAnError();
    TestMissingRequiredGroupCodeIsAnError();
    TestCrlfLineEndingsAreHandled();
    TestEntityWithoutLayerCodeDefaultsToLayerZero();
    TestFileNotFoundIsAnError();
    TestEmptyEntitiesSectionProducesEmptyDocument();
    TestBlankLineWithinEntitiesSectionIsSkippedNotFatal();
    TestMultipleConsecutiveBlankLinesAreSkipped();
    TestTrailingBlankLinesAfterEofMarkerDoNotError();

    TestRequiredFieldWithTrailingGarbageIsRejected();
    TestLineCoordinateWithTrailingGarbageIsRejected();
    TestGroupCodeWithTrailingGarbageIsRejected();
    TestLwPolylineVertexCoordinateWithTrailingGarbageIsRejected();
    TestLwPolylineBulgeWithTrailingGarbageFallsBackToStraight();
    TestLegitimateSignedAndScientificNotationStillParse();

    TestMalformedGroupCodeMidEntitiesReportsUnexpectedEnd();
    TestTruncatedMidPairReportsUnexpectedEnd();
    TestCleanlyMissingEndsecKeepsOriginalMessage();
    TestMalformedTokenInLaterIgnoredSectionDoesNotFailParse();

    TestLwPolylineClosedRectangleProducesFourLines();
    TestLwPolylineOpenProducesOneFewerSegmentThanVertices();
    TestLwPolylineBulgeOneProducesSemicircleArc();
    TestLwPolylineMixedStraightAndCurvedSegments();
    TestLwPolylineSingleVertexIsSkippedNotFatal();
    TestLwPolylineWithoutClosedFlagDefaultsToOpen();

    TestLayerColorAndLinetypeImportedFromTablesLayerSection();
    TestLayerColorNegativeSignTreatedAsOffMagnitudeStillApplies();
    TestNoTablesSectionLeavesLayerAppearanceAtDefaults();
    TestTablesWithoutLayerTableLeavesLayerAppearanceAtDefaults();
    TestUnrecognizedAciAndLinetypeFallBackToDefaultsNotError();
    TestLayerTablePropertiesAppliedEvenIfNoEntityUsesThatLayer();
    TestUnclosedTablesSectionDoesNotAffectEntitiesImport();

    std::puts("DxfReaderTests: all tests passed.");
    return 0;
}
