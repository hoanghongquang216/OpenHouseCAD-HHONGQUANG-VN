// Golden file tests for DXF-EXPORT-001, added per Gate Review (before
// Phase 5). Maps to docs/design/DXF-EXPORT-001-Test-Design.md Section 3
// (G-001..003).
//
// G-001/G-002 compare WriteDxfStream's output against a checked-in
// reference file byte-for-byte. IMPORTANT -- bootstrap behavior: if the
// reference file does not exist yet on disk, this test WRITES it (and
// reports that it did so) instead of failing, so the first run creates
// the reference for a human to review and commit. Every subsequent run
// compares against that committed file normally. This avoids hand-
// authoring exact byte content by hand (error-prone for a text format
// with floating-point output) while still giving a real regression
// signal once the reference exists -- the same bootstrap-then-compare
// pattern many serializer test suites use for golden files.
//
// G-003 needs no reference file -- it tests the Determinism guarantee
// (Design.md's addendum) directly, in-memory.

#include <openhouse/document/Document.hpp>
#include <openhouse/dxf/DxfWriter.hpp>
#include <openhouse/foundation/String.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>

#include <openhouse/testing/Check.hpp>
#include <cstdio>
#include <fstream>
#include <optional>
#include <sstream>

using namespace openhouse::document;
using namespace openhouse::geometry;
using namespace openhouse::dxf;

namespace foundation = openhouse::foundation;

#ifndef OPENHOUSE_TEST_DATA_DIR
#error "OPENHOUSE_TEST_DATA_DIR must be defined by CMakeLists.txt (absolute path to this test's testdata/ directory) -- see the CMake target_compile_definitions entry added alongside this test's add_executable."
#endif

namespace {

std::optional<foundation::string> ReadFileIfExists(const foundation::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return std::nullopt;
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

// Builds the fixed Document G-001 writes out.
Document BuildBasicDocument() {
    Document doc;
    doc.CreateLayer("Walls").SetColor("red");
    doc.CreateLayer("Doors").SetLineType(LineType::Dashed);
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}}, "Walls");
    doc.Add(Circle2d{Point2d{5.0, 5.0}, 2.0}, "Doors");
    doc.Add(Arc2d{Point2d{0.0, 0.0}, 3.0, 0.0, 1.5707963267948966}, "0"); // pi/2
    return doc;
}

// Compares `output` against the reference file at `path`. Bootstraps
// (writes and reports) if the reference doesn't exist yet -- see the
// file-level comment above.
void CompareAgainstGolden(const foundation::string& output, const foundation::string& path) {
    const std::optional<foundation::string> reference = ReadFileIfExists(path);
    if (!reference.has_value()) {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        OH_CHECK(static_cast<bool>(out));
        out << output;
        std::printf(
            "GOLDEN FILE BOOTSTRAPPED: %s did not exist -- wrote it from this run's output. "
            "Review its content, then `git add` and commit it. Re-run this test after "
            "committing to get a real comparison.\n",
            path.c_str());
        return; // first run: nothing to compare against yet, not a failure
    }
    OH_CHECK(output == *reference);
}

} // namespace

// G-001: a small, fixed Document -> byte-for-byte match against
// testdata/golden_basic.dxf.
static void TestG001_BasicDocument_MatchesGoldenFile() {
    const Document doc = BuildBasicDocument();
    std::ostringstream out;
    OH_CHECK(WriteDxfStream(doc, out));

    CompareAgainstGolden(out.str(), foundation::string(OPENHOUSE_TEST_DATA_DIR) + "/golden_basic.dxf");
}

// G-002: an empty Document -> byte-for-byte match against
// testdata/golden_empty.dxf.
static void TestG002_EmptyDocument_MatchesGoldenFile() {
    const Document doc;
    std::ostringstream out;
    OH_CHECK(WriteDxfStream(doc, out));

    CompareAgainstGolden(out.str(), foundation::string(OPENHOUSE_TEST_DATA_DIR) + "/golden_empty.dxf");
}

// G-003: writing the same Document twice produces byte-for-byte identical
// output -- the direct test of the Determinism guarantee itself,
// independent of any reference file.
static void TestG003_SameDocumentWrittenTwice_ByteForByteIdentical() {
    const Document doc = BuildBasicDocument();

    std::ostringstream firstOut;
    OH_CHECK(WriteDxfStream(doc, firstOut));
    std::ostringstream secondOut;
    OH_CHECK(WriteDxfStream(doc, secondOut));

    OH_CHECK(firstOut.str() == secondOut.str());
}

int main() {
    TestG001_BasicDocument_MatchesGoldenFile();
    TestG002_EmptyDocument_MatchesGoldenFile();
    TestG003_SameDocumentWrittenTwice_ByteForByteIdentical();

    std::puts("DxfWriterGoldenFileTests: all tests passed.");
    return 0;
}
