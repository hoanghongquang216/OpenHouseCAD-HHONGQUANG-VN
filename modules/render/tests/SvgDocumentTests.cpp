#include <openhouse/render/SvgDocument.hpp>
#include <openhouse/testing/Check.hpp>

#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

using namespace openhouse::render;
using namespace openhouse::geometry;

namespace {
std::string ReadFile(const std::string& path) {
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}
} // namespace

static void TestEmptyDocumentIsValidSvgShell() {
    const SvgDocument doc(100.0, 100.0);
    const std::string content = doc.ToString();

    OH_CHECK(content.find("<?xml") != std::string::npos);
    OH_CHECK(content.find("<svg") != std::string::npos);
    OH_CHECK(content.find("</svg>") != std::string::npos);
    OH_CHECK(content.find(R"(width="100")") != std::string::npos);
    OH_CHECK(content.find(R"(height="100")") != std::string::npos);
}

static void TestAddPointProducesCircleElement() {
    SvgDocument doc;
    doc.AddPoint(Point2d{10.0, 20.0});
    const std::string content = doc.ToString();

    OH_CHECK(content.find("<circle") != std::string::npos);
    OH_CHECK(content.find(R"(cx="10")") != std::string::npos);
    OH_CHECK(content.find(R"(cy="20")") != std::string::npos);
    OH_CHECK(content.find(R"(fill="black")") != std::string::npos);
}

static void TestAddPointWithCustomRadiusAndColor() {
    SvgDocument doc;
    doc.AddPoint(Point2d{5.0, 5.0}, 7.5, "red");
    const std::string content = doc.ToString();

    OH_CHECK(content.find(R"(r="7.5")") != std::string::npos);
    OH_CHECK(content.find(R"(fill="red")") != std::string::npos);
}

static void TestMultiplePointsAllPresent() {
    SvgDocument doc;
    doc.AddPoint(Point2d{1.0, 1.0});
    doc.AddPoint(Point2d{2.0, 2.0});
    doc.AddPoint(Point2d{3.0, 3.0});
    const std::string content = doc.ToString();

    // All three circle elements must be present (order doesn't matter for
    // this check, just presence).
    OH_CHECK(content.find(R"(cx="1")") != std::string::npos);
    OH_CHECK(content.find(R"(cx="2")") != std::string::npos);
    OH_CHECK(content.find(R"(cx="3")") != std::string::npos);

    // [[maybe_unused]]: `count` is only read inside OH_CHECK(), which
    // compiles away entirely under NDEBUG (Release builds) -- without
    // this, GCC/Clang correctly flag it as "set but not used" in that
    // configuration, which -Werror then turns into a hard build failure.
    // Found via a real Release-mode CI failure, not by inspection.
    std::size_t count = 0;
    std::size_t pos = 0;
    while ((pos = content.find("<circle", pos)) != std::string::npos) {
        ++count;
        pos += 1;
    }
    OH_CHECK(count == 3);
}

static void TestWriteToFileActuallyWritesReadableContent() {
    // This is the actual vertical-slice proof: write a real file to disk,
    // read it back independently (not via the SvgDocument API), and
    // confirm the point is really there -- not just that ToString()
    // returns a plausible-looking string in memory.
    const std::string path = (std::filesystem::temp_directory_path() /
                               "openhousecad_render001_test.svg")
                                  .string();

    SvgDocument doc(50.0, 50.0);
    doc.AddPoint(Point2d{25.0, 25.0}, 4.0, "blue");

    // See TestMultiplePointsAllPresent's comment on count -- same NDEBUG
    // issue: writeOk is only read by OH_CHECK(), which vanishes in Release
    // builds.
    const bool writeOk = doc.WriteToFile(path);
    OH_CHECK(writeOk);
    OH_CHECK(std::filesystem::exists(path));

    const std::string readBack = ReadFile(path);
    OH_CHECK(readBack.find("<svg") != std::string::npos);
    OH_CHECK(readBack.find("<circle") != std::string::npos);
    OH_CHECK(readBack.find(R"(cx="25")") != std::string::npos);
    OH_CHECK(readBack.find(R"(fill="blue")") != std::string::npos);

    // In-memory ToString() and the file that was actually written to disk
    // must match exactly -- guards against any divergence between the
    // two code paths.
    OH_CHECK(readBack == doc.ToString());

    std::filesystem::remove(path);
}

static void TestWriteToFileFailsGracefullyForInvalidPath() {
    SvgDocument doc;
    doc.AddPoint(Point2d{0.0, 0.0});
    // A path in a directory that (almost certainly) doesn't exist.
    // Same NDEBUG issue as elsewhere in this file.
    const bool writeOk = doc.WriteToFile("/nonexistent_dir_12345/out.svg");
    OH_CHECK(!writeOk);
}

static void TestAddLineProducesLineElement() {
    SvgDocument doc;
    doc.AddLine(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 10.0}});
    const std::string content = doc.ToString();

    OH_CHECK(content.find("<line") != std::string::npos);
    OH_CHECK(content.find(R"(x1="0")") != std::string::npos);
    OH_CHECK(content.find(R"(y1="0")") != std::string::npos);
    OH_CHECK(content.find(R"(x2="10")") != std::string::npos);
    OH_CHECK(content.find(R"(y2="10")") != std::string::npos);
    OH_CHECK(content.find(R"(stroke="black")") != std::string::npos);
}

static void TestAddLineWithCustomStrokeWidthAndColor() {
    SvgDocument doc;
    doc.AddLine(Line2d{Point2d{1.0, 1.0}, Point2d{2.0, 2.0}}, 2.5, "green");
    const std::string content = doc.ToString();

    OH_CHECK(content.find(R"(stroke-width="2.5")") != std::string::npos);
    OH_CHECK(content.find(R"(stroke="green")") != std::string::npos);
}

static void TestPointsAndLinesCoexist() {
    SvgDocument doc;
    doc.AddPoint(Point2d{0.0, 0.0});
    doc.AddLine(Line2d{Point2d{0.0, 0.0}, Point2d{5.0, 5.0}});
    doc.AddPoint(Point2d{5.0, 5.0});
    const std::string content = doc.ToString();

    OH_CHECK(content.find("<circle") != std::string::npos);
    OH_CHECK(content.find("<line") != std::string::npos);
}

static void TestWriteLineToFileRoundTrips() {
    const std::string path = (std::filesystem::temp_directory_path() /
                               "openhousecad_render002_test.svg")
                                  .string();

    SvgDocument doc(50.0, 50.0);
    doc.AddLine(Line2d{Point2d{5.0, 5.0}, Point2d{45.0, 45.0}}, 2.0, "blue");

    OH_CHECK(doc.WriteToFile(path));
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    const std::string readBack = ss.str();

    OH_CHECK(readBack.find("<line") != std::string::npos);
    OH_CHECK(readBack.find(R"(x2="45")") != std::string::npos);
    OH_CHECK(readBack == doc.ToString());

    std::filesystem::remove(path);
}

static void TestAddCircleProducesOutlineNotFill() {
    SvgDocument doc;
    doc.AddCircle(Circle2d{Point2d{50.0, 50.0}, 20.0});
    const std::string content = doc.ToString();

    OH_CHECK(content.find("<circle") != std::string::npos);
    OH_CHECK(content.find(R"(cx="50")") != std::string::npos);
    OH_CHECK(content.find(R"(cy="50")") != std::string::npos);
    OH_CHECK(content.find(R"(r="20")") != std::string::npos);
    // The defining difference from AddPoint: fill="none", not a filled dot.
    OH_CHECK(content.find(R"(fill="none")") != std::string::npos);
    OH_CHECK(content.find(R"(stroke="black")") != std::string::npos);
}

static void TestAddCircleWithCustomStyle() {
    SvgDocument doc;
    doc.AddCircle(Circle2d{Point2d{0.0, 0.0}, 5.0}, 3.0, "purple");
    const std::string content = doc.ToString();

    OH_CHECK(content.find(R"(stroke="purple")") != std::string::npos);
    OH_CHECK(content.find(R"(stroke-width="3")") != std::string::npos);
}

static void TestPointAndCircleAreVisuallyDistinguishable() {
    // A point (filled) and a circle (outlined) both use <circle> under
    // the hood but must be distinguishable by their fill attribute.
    SvgDocument doc;
    doc.AddPoint(Point2d{10.0, 10.0}, 3.0, "black");
    doc.AddCircle(Circle2d{Point2d{10.0, 10.0}, 3.0}, 1.0, "black");
    const std::string content = doc.ToString();

    OH_CHECK(content.find(R"(fill="black")") != std::string::npos); // the point
    OH_CHECK(content.find(R"(fill="none")") != std::string::npos);  // the circle
}

static void TestWriteCircleToFileRoundTrips() {
    const std::string path = (std::filesystem::temp_directory_path() /
                               "openhousecad_render003_test.svg")
                                  .string();

    SvgDocument doc(100.0, 100.0);
    doc.AddCircle(Circle2d{Point2d{50.0, 50.0}, 30.0}, 2.0, "orange");

    OH_CHECK(doc.WriteToFile(path));
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    const std::string readBack = ss.str();

    OH_CHECK(readBack.find("<circle") != std::string::npos);
    OH_CHECK(readBack.find(R"(r="30")") != std::string::npos);
    OH_CHECK(readBack == doc.ToString());

    std::filesystem::remove(path);
}

static void TestAddArcQuarterTurnSmallArcCounterClockwise() {
    // Quarter circle (sweep = pi/2 < pi -> large-arc-flag=0; sweep > 0
    // -> sweep-flag=1), from (5,0) to (0,5) around center (0,0), radius 5.
    constexpr double kPi = 3.14159265358979323846;
    SvgDocument doc;
    doc.AddArc(Arc2d{Point2d{0.0, 0.0}, 5.0, 0.0, kPi / 2.0});
    const std::string content = doc.ToString();

    OH_CHECK(content.find("<path") != std::string::npos);
    // Start point (5,0) is exact; don't assert on the end point's exact
    // formatted text -- cos(pi/2) is not exactly 0 in floating point, so
    // its std::format representation is an implementation detail not
    // worth hardcoding/guessing. The "M 5 0" prefix and the flag values
    // are exact and meaningful to check.
    OH_CHECK(content.find(R"(d="M 5 0 A 5 5 0 0 1)") != std::string::npos);
}

static void TestAddArcLargeArcFlagSetWhenSweepExceedsPi() {
    // Sweep = 3*pi/2 > pi -> large-arc-flag must be 1.
    constexpr double kPi = 3.14159265358979323846;
    SvgDocument doc;
    doc.AddArc(Arc2d{Point2d{0.0, 0.0}, 1.0, 0.0, 3.0 * kPi / 2.0});
    const std::string content = doc.ToString();

    // "A rx ry 0 <large-arc-flag> <sweep-flag> ..." -- large-arc-flag is 1.
    OH_CHECK(content.find("A 1 1 0 1 1") != std::string::npos);
}

static void TestAddArcSweepFlagZeroForClockwiseSweep() {
    // endAngle < startAngle -> negative sweep -> sweep-flag=0.
    constexpr double kPi = 3.14159265358979323846;
    SvgDocument doc;
    doc.AddArc(Arc2d{Point2d{0.0, 0.0}, 1.0, kPi / 2.0, 0.0});
    const std::string content = doc.ToString();

    // Sweep magnitude pi/2 < pi -> large-arc-flag 0; sweep negative -> sweep-flag 0.
    OH_CHECK(content.find("A 1 1 0 0 0") != std::string::npos);
}

static void TestAddArcStyling() {
    SvgDocument doc;
    doc.AddArc(Arc2d{Point2d{0.0, 0.0}, 3.0, 0.0, 1.0}, 2.5, "teal");
    const std::string content = doc.ToString();

    OH_CHECK(content.find(R"(stroke="teal")") != std::string::npos);
    OH_CHECK(content.find(R"(stroke-width="2.5")") != std::string::npos);
    OH_CHECK(content.find(R"(fill="none")") != std::string::npos);
}

static void TestAddLineWithDashArray() {
    SvgDocument doc;
    doc.AddLine(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}}, 1.0, "black", "6,4");
    const std::string content = doc.ToString();

    OH_CHECK(content.find(R"(stroke-dasharray="6,4")") != std::string::npos);
}

static void TestAddLineWithoutDashArrayOmitsAttribute() {
    SvgDocument doc;
    doc.AddLine(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}});
    const std::string content = doc.ToString();

    // Default dashArray is empty -- the attribute must not appear at all,
    // not appear as `stroke-dasharray=""`.
    OH_CHECK(content.find("stroke-dasharray") == std::string::npos);
}

static void TestAddCircleWithDashArray() {
    SvgDocument doc;
    doc.AddCircle(Circle2d{Point2d{0.0, 0.0}, 5.0}, 1.0, "black", "1,3");
    const std::string content = doc.ToString();

    OH_CHECK(content.find(R"(stroke-dasharray="1,3")") != std::string::npos);
}

static void TestAddArcWithDashArray() {
    SvgDocument doc;
    doc.AddArc(Arc2d{Point2d{0.0, 0.0}, 5.0, 0.0, 1.0}, 1.0, "black", "6,3,1,3");
    const std::string content = doc.ToString();

    OH_CHECK(content.find(R"(stroke-dasharray="6,3,1,3")") != std::string::npos);
}

int main() {
    TestEmptyDocumentIsValidSvgShell();
    TestAddPointProducesCircleElement();
    TestAddPointWithCustomRadiusAndColor();
    TestMultiplePointsAllPresent();
    TestAddLineProducesLineElement();
    TestAddLineWithCustomStrokeWidthAndColor();
    TestPointsAndLinesCoexist();
    TestAddCircleProducesOutlineNotFill();
    TestAddCircleWithCustomStyle();
    TestPointAndCircleAreVisuallyDistinguishable();
    TestAddArcQuarterTurnSmallArcCounterClockwise();
    TestAddArcLargeArcFlagSetWhenSweepExceedsPi();
    TestAddArcSweepFlagZeroForClockwiseSweep();
    TestAddArcStyling();
    TestAddLineWithDashArray();
    TestAddLineWithoutDashArrayOmitsAttribute();
    TestAddCircleWithDashArray();
    TestAddArcWithDashArray();
    TestWriteCircleToFileRoundTrips();
    TestWriteLineToFileRoundTrips();
    TestWriteToFileActuallyWritesReadableContent();
    TestWriteToFileFailsGracefullyForInvalidPath();

    std::puts("SvgDocumentTests: all tests passed.");
    return 0;
}
