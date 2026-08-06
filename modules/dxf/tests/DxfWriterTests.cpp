// Tests for DxfWriter, added for DXF-EXPORT-001. Maps to
// docs/design/DXF-EXPORT-001-Test-Design.md Section 1 (Functional,
// F-001..013). Round-trip (R-series) and Golden File (G-series) tests
// are PR#2, deliberately not here (per this Sprint's PR split).

#include <openhouse/document/Document.hpp>
#include <openhouse/dxf/DxfWriter.hpp>
#include <openhouse/foundation/String.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>

#include <openhouse/testing/Check.hpp>
#include <cstdio>
#include <iomanip>
#include <sstream>

using namespace openhouse::document;
using namespace openhouse::geometry;
using namespace openhouse::dxf;

namespace foundation = openhouse::foundation;

namespace {

foundation::string WriteToString(const Document& doc) {
    std::ostringstream oss;
    OH_CHECK(WriteDxfStream(doc, oss));
    return oss.str();
}

std::size_t CountOccurrences(const foundation::string& haystack, const foundation::string& needle) {
    std::size_t count = 0;
    std::size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != foundation::string::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}

} // namespace

// F-001: empty Document -> structurally valid, zero entity records.
static void TestF001_EmptyDocument_ProducesValidStructure() {
    Document doc;
    const foundation::string output = WriteToString(doc);

    OH_CHECK(output.find("0\nSECTION\n2\nENTITIES\n") != foundation::string::npos);
    OH_CHECK(output.find("0\nENDSEC\n0\nEOF\n") != foundation::string::npos);
    OH_CHECK(output.find("0\nLINE\n") == foundation::string::npos);
    OH_CHECK(output.find("0\nCIRCLE\n") == foundation::string::npos);
    OH_CHECK(output.find("0\nARC\n") == foundation::string::npos);
}

// F-002: single Line -> correct group codes.
static void TestF002_SingleLine_CorrectGroupCodes() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}});
    const foundation::string output = WriteToString(doc);

    OH_CHECK(output.find("0\nLINE\n8\n0\n10\n0\n20\n0\n11\n1\n21\n1\n") != foundation::string::npos);
}

// F-003: single Circle -> correct group codes.
static void TestF003_SingleCircle_CorrectGroupCodes() {
    Document doc;
    doc.Add(Circle2d{Point2d{5.0, 5.0}, 2.0});
    const foundation::string output = WriteToString(doc);

    OH_CHECK(output.find("0\nCIRCLE\n8\n0\n10\n5\n20\n5\n40\n2\n") != foundation::string::npos);
}

// F-004: single Arc -> correct group codes, angles converted radians->degrees.
// Expected degree strings are computed via the SAME conversion the writer
// uses (detail::RadiansToDegrees), not hardcoded literals -- radians ->
// degrees is not always bit-exact for arbitrary angles, so hardcoding a
// literal could make this test fragile for reasons unrelated to a real bug.
static void TestF004_SingleArc_CorrectGroupCodesAndDegreeConversion() {
    Document doc;
    constexpr double startRad = 0.5;
    constexpr double endRad = 1.0;
    doc.Add(Arc2d{Point2d{0.0, 0.0}, 3.0, startRad, endRad});
    const foundation::string output = WriteToString(doc);

    std::ostringstream expectedStart;
    expectedStart << std::setprecision(17) << detail::RadiansToDegrees(startRad);
    std::ostringstream expectedEnd;
    expectedEnd << std::setprecision(17) << detail::RadiansToDegrees(endRad);

    OH_CHECK(output.find("0\nARC\n8\n0\n10\n0\n20\n0\n40\n3\n50\n" + expectedStart.str() + "\n51\n" +
                          expectedEnd.str() + "\n") != foundation::string::npos);
}

// F-005: entities on different layers -> each entity's own layer name.
static void TestF005_EntitiesOnDifferentLayers_CorrectLayerReference() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}}, "Walls");
    doc.Add(Line2d{Point2d{2.0, 2.0}, Point2d{3.0, 3.0}}, "Doors");
    const foundation::string output = WriteToString(doc);

    OH_CHECK(output.find("8\nWalls\n") != foundation::string::npos);
    OH_CHECK(output.find("8\nDoors\n") != foundation::string::npos);
}

// F-006: LAYER table has exactly one entry per doc.Layers(), including "0".
static void TestF006_LayerTable_OneEntryPerLayerIncludingDefault() {
    Document doc;
    doc.CreateLayer("Walls");
    doc.CreateLayer("Doors");
    const foundation::string output = WriteToString(doc);

    OH_CHECK(doc.Layers().size() == 3); // "0" + "Walls" + "Doors"
    OH_CHECK(CountOccurrences(output, "0\nLAYER\n2\n") == 3);
    OH_CHECK(output.find("2\n0\n") != foundation::string::npos);
    OH_CHECK(output.find("2\nWalls\n") != foundation::string::npos);
    OH_CHECK(output.find("2\nDoors\n") != foundation::string::npos);
}

// F-007: known color reverse-mapped exactly.
static void TestF007_KnownColor_ReverseMappedToExactAci() {
    Document doc;
    doc.CreateLayer("Colored").SetColor("red");
    const foundation::string output = WriteToString(doc);

    OH_CHECK(output.find("2\nColored\n70\n0\n62\n1\n6\nCONTINUOUS\n") != foundation::string::npos);
}

// F-008: default layer color ("black") maps to ACI 7.
static void TestF008_DefaultBlackColor_MapsToAciSeven() {
    Document doc;
    const foundation::string output = WriteToString(doc);

    OH_CHECK(output.find("2\n0\n70\n0\n62\n7\n6\nCONTINUOUS\n") != foundation::string::npos);
}

// F-009: unknown color -> group code 62 omitted entirely (not a fallback value).
static void TestF009_UnknownColor_GroupCode62Omitted() {
    Document doc;
    doc.CreateLayer("Weird").SetColor("#3a7bd5");
    const foundation::string output = WriteToString(doc);

    // "70\n0\n" must be followed immediately by "6\n" (linetype), with no
    // "62\n" in between -- confirms omission, not a guessed value.
    OH_CHECK(output.find("2\nWeird\n70\n0\n6\nCONTINUOUS\n") != foundation::string::npos);
}

// F-010: every LineType maps to its exact canonical DXF name.
static void TestF010_EveryLineType_MapsToExactName() {
    {
        Document doc;
        doc.CreateLayer("L").SetLineType(LineType::Continuous);
        OH_CHECK(WriteToString(doc).find("6\nCONTINUOUS\n") != foundation::string::npos);
    }
    {
        Document doc;
        doc.CreateLayer("L").SetLineType(LineType::Dashed);
        OH_CHECK(WriteToString(doc).find("6\nDASHED\n") != foundation::string::npos);
    }
    {
        Document doc;
        doc.CreateLayer("L").SetLineType(LineType::Dotted);
        OH_CHECK(WriteToString(doc).find("6\nDOTTED\n") != foundation::string::npos);
    }
    {
        Document doc;
        doc.CreateLayer("L").SetLineType(LineType::DashDot);
        OH_CHECK(WriteToString(doc).find("6\nDASHDOT\n") != foundation::string::npos);
    }
}

// F-011: entity on a hidden layer is still written (DG-002 -- no visibility filter).
static void TestF011_HiddenLayerEntity_StillWritten() {
    Document doc;
    doc.CreateLayer("Hidden").SetVisible(false);
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}}, "Hidden");
    const foundation::string output = WriteToString(doc);

    OH_CHECK(output.find("0\nLINE\n8\nHidden\n") != foundation::string::npos);
}

// F-012: WriteDxfFile to an unwritable path returns false, doesn't throw.
static void TestF012_WriteDxfFile_UnwritablePath_ReturnsFalse() {
    Document doc;
    const bool result = WriteDxfFile(doc, "/nonexistent_directory_xyz/out.dxf");
    OH_CHECK(!result);
}

// F-013: WriteDxfStream/WriteDxfFile succeed on a normal target.
static void TestF013_NormalTarget_ReturnsTrue() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}});

    std::ostringstream oss;
    OH_CHECK(WriteDxfStream(doc, oss));

    const foundation::string tmpPath = "dxf_writer_test_output.dxf";
    OH_CHECK(WriteDxfFile(doc, tmpPath));
    std::remove(tmpPath.c_str());
}

int main() {
    TestF001_EmptyDocument_ProducesValidStructure();
    TestF002_SingleLine_CorrectGroupCodes();
    TestF003_SingleCircle_CorrectGroupCodes();
    TestF004_SingleArc_CorrectGroupCodesAndDegreeConversion();
    TestF005_EntitiesOnDifferentLayers_CorrectLayerReference();
    TestF006_LayerTable_OneEntryPerLayerIncludingDefault();
    TestF007_KnownColor_ReverseMappedToExactAci();
    TestF008_DefaultBlackColor_MapsToAciSeven();
    TestF009_UnknownColor_GroupCode62Omitted();
    TestF010_EveryLineType_MapsToExactName();
    TestF011_HiddenLayerEntity_StillWritten();
    TestF012_WriteDxfFile_UnwritablePath_ReturnsFalse();
    TestF013_NormalTarget_ReturnsTrue();

    std::puts("DxfWriterTests: all tests passed.");
    return 0;
}
