// SvgPipelineIntegrationTests.cpp -- DS-005R
//
// Verifies the full DXF -> Document -> SVG pipeline end to end, in one
// test binary. DxfReaderTests.cpp already covers DXF -> Document in
// isolation, and RenderDocumentTests.cpp already covers Document ->
// SVG in isolation; neither proves the two stages actually compose
// through a real Document the way Engineering Principle 13 describes
// ("Document is the canonical model... importers and exporters never
// communicate directly with each other"). This file is that missing
// vertical slice (Principle 10).
//
// DXF source text is embedded inline via std::istringstream, matching
// DxfReaderTests.cpp's own convention -- not read from a file on disk.
// Assertions are substring checks against SvgDocument::ToString(),
// matching RenderDocumentTests.cpp's convention -- not a comparison
// against a golden .svg file on disk (no such pattern exists elsewhere
// in this codebase).

#include <openhouse/dxf/DxfReader.hpp>
#include <openhouse/render/RenderDocument.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>
#include <sstream>
#include <string>

using namespace openhouse::document;
using namespace openhouse::geometry;
using namespace openhouse::render;

namespace {

// Small helper shared by every test below: parse DXF text, ASSERT it
// succeeded (a parse failure here would silently turn a real test
// into a no-op), and hand back the resulting Document.
Document ParseOrFail(const std::string& dxfText) {
    std::istringstream dxf(dxfText);
    auto result = openhouse::dxf::ParseDxfStream(dxf);
    OH_CHECK(result.has_value());
    return *result;
}

} // namespace

static void TestLineDxfRendersAsSvgLine() {
    Document doc = ParseOrFail(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n100.0\n21\n50.0\n"
        "0\nENDSEC\n0\nEOF\n");

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<line") != std::string::npos);
    OH_CHECK(content.find(R"(x2="100")") != std::string::npos);
}

static void TestCircleDxfRendersAsSvgCircle() {
    Document doc = ParseOrFail(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nCIRCLE\n8\n0\n10\n50.0\n20\n50.0\n40\n25.0\n"
        "0\nENDSEC\n0\nEOF\n");

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<circle") != std::string::npos);
    OH_CHECK(content.find(R"(r="25")") != std::string::npos);
}

static void TestArcDxfRendersAsSvgPath() {
    // Also exercises the degrees->radians conversion (DxfReaderTests
    // covers the conversion itself; here it just needs to survive
    // intact through to a renderable Arc2d).
    Document doc = ParseOrFail(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nARC\n8\n0\n10\n0.0\n20\n0.0\n40\n10.0\n50\n0.0\n51\n90.0\n"
        "0\nENDSEC\n0\nEOF\n");

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<path") != std::string::npos);
}

static void TestLwPolylineBulgeDxfRendersMixedLineAndArc() {
    // Straight segment (vertex 0->1) plus one bulge=1 segment
    // (vertex 1->2) -- confirms LWPOLYLINE's mixed straight/curved
    // output (already unit-tested in isolation by DxfReaderTests)
    // still renders as the expected mix of <line> and <path> once it
    // reaches SvgDocument.
    Document doc = ParseOrFail(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLWPOLYLINE\n8\n0\n90\n3\n70\n0\n"
        "10\n0.0\n20\n0.0\n"
        "10\n5.0\n20\n0.0\n42\n1.0\n"
        "10\n7.0\n20\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<line") != std::string::npos);
    OH_CHECK(content.find("<path") != std::string::npos);
}

static void TestUnsupportedEntitySkippedEndToEnd() {
    // TEXT is parsed-and-discarded by DxfReader (unit-tested in
    // isolation by DxfReaderTests); this confirms the LINE that
    // follows it still makes it all the way through to SVG output.
    Document doc = ParseOrFail(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nTEXT\n8\n0\n1\nhello\n"
        "0\nLINE\n8\n0\n10\n1.0\n20\n2.0\n11\n3.0\n21\n4.0\n"
        "0\nENDSEC\n0\nEOF\n");
    OH_CHECK(doc.Count() == 1);

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<line") != std::string::npos);
}

static void TestEmptyDxfRendersEmptySvgShell() {
    Document doc = ParseOrFail("0\nSECTION\n2\nENTITIES\n0\nENDSEC\n0\nEOF\n");
    OH_CHECK(doc.Empty());

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<svg") != std::string::npos);
    OH_CHECK(content.find("<line") == std::string::npos);
    OH_CHECK(content.find("<circle") == std::string::npos);
    OH_CHECK(content.find("<path") == std::string::npos);
}

static void TestMalformedDxfStopsBeforeRenderNoPartialSvg() {
    // LINE missing required group code 21 (end.y) -- DxfReaderTests
    // already confirms ParseDxfStream reports this as an error; this
    // test's job is different: confirm the pipeline stops there and
    // never reaches RenderToSvg with a half-built Document. There is
    // deliberately no SvgDocument constructed in this test.
    std::istringstream dxf(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n11\n1.0\n"
        "0\nENDSEC\n0\nEOF\n");
    auto result = openhouse::dxf::ParseDxfStream(dxf);
    OH_CHECK(!result.has_value());
}

static void TestLayerColorSetAfterDxfImportAppliesToRender() {
    // DXF group code 8 only carries a layer NAME (confirmed by
    // DxfReaderTests -- ParseDxfStream never touches color/line
    // weight/line type). Per Principle 13, the Document produced by
    // import is an ordinary Document -- this confirms it's fully
    // usable by the normal post-import styling API (Layer::SetColor,
    // as already exercised for hand-built Documents in
    // RenderDocumentTests) with no special-casing needed for
    // DXF-originated layers.
    Document doc = ParseOrFail(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n8\nWalls\n10\n0.0\n20\n0.0\n11\n10.0\n21\n0.0\n"
        "0\nENDSEC\n0\nEOF\n");

    Layer* walls = doc.FindLayer("Walls");
    OH_CHECK(walls != nullptr);
    walls->SetColor("red");

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find(R"(stroke="red")") != std::string::npos);
}

static void TestHiddenLayerFromDxfExcludedFromRender() {
    Document doc = ParseOrFail(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nCIRCLE\n8\nHidden\n10\n0.0\n20\n0.0\n40\n5.0\n"
        "0\nENDSEC\n0\nEOF\n");

    Layer* hidden = doc.FindLayer("Hidden");
    OH_CHECK(hidden != nullptr);
    hidden->SetVisible(false);

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<circle") == std::string::npos);
}

static void TestRealisticMultiLayerFloorPlanEndToEnd() {
    // Mirrors RenderDocumentTests' TestThreeLayerScenarioMatchesSpiral2Spec,
    // but the Document here comes from parsing DXF text (multiple
    // entities, multiple layers, one bulge segment) rather than being
    // hand-built -- the closest thing in this suite to Principle 11's
    // "every spiral ends with a runnable demo" for the DXF->SVG path,
    // short of writing to disk (which dxf_import_demo.cpp already
    // does, using the real sample_house.dxf).
    Document doc = ParseOrFail(
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLWPOLYLINE\n8\nWalls\n90\n4\n70\n1\n"
        "10\n0.0\n20\n0.0\n"
        "10\n10.0\n20\n0.0\n42\n0.5\n"
        "10\n10.0\n20\n8.0\n"
        "10\n0.0\n20\n8.0\n"
        "0\nLINE\n8\nCenter\n10\n0.0\n20\n4.0\n11\n10.0\n21\n4.0\n"
        "0\nCIRCLE\n8\nHidden\n10\n5.0\n20\n4.0\n40\n1.0\n"
        "0\nENDSEC\n0\nEOF\n");
    // Closed LWPOLYLINE, 4 vertices -> 4 segments (per
    // TestLwPolylineClosedRectangleProducesFourLines in
    // DxfReaderTests.cpp), one of which is curved (the bulge=0.5
    // segment) -- plus 1 LINE (Center) + 1 CIRCLE (Hidden) = 6.
    OH_CHECK(doc.Count() == 6);
    // Document always has a pre-existing default layer "0" (confirmed
    // by DxfReaderTests' TestEmptyEntitiesSectionProducesEmptyDocument),
    // even before any entity is added. None of the entities below use
    // layer "0" explicitly, so it stays present but empty alongside
    // the 3 layers actually used: "0" (default, unused), "Walls",
    // "Center", "Hidden" = 4.
    OH_CHECK(doc.Layers().size() == 4);

    doc.FindLayer("Walls")->SetColor("black");
    Layer* center = doc.FindLayer("Center");
    center->SetColor("gray");
    center->SetLineType(LineType::Dashed);
    doc.FindLayer("Hidden")->SetVisible(false);

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find(R"(stroke="black")") != std::string::npos);
    OH_CHECK(content.find(R"(stroke="gray")") != std::string::npos);
    OH_CHECK(content.find("stroke-dasharray=") != std::string::npos);
    OH_CHECK(content.find("<circle") == std::string::npos); // Hidden excluded
    OH_CHECK(content.find("<path") != std::string::npos);   // the bulge segment
}

int main() {
    TestLineDxfRendersAsSvgLine();
    TestCircleDxfRendersAsSvgCircle();
    TestArcDxfRendersAsSvgPath();
    TestLwPolylineBulgeDxfRendersMixedLineAndArc();
    TestUnsupportedEntitySkippedEndToEnd();
    TestEmptyDxfRendersEmptySvgShell();
    TestMalformedDxfStopsBeforeRenderNoPartialSvg();
    TestLayerColorSetAfterDxfImportAppliesToRender();
    TestHiddenLayerFromDxfExcludedFromRender();
    TestRealisticMultiLayerFloorPlanEndToEnd();

    std::puts("SvgPipelineIntegrationTests: all tests passed.");
    return 0;
}
