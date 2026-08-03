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

    std::puts("DxfReaderTests: all tests passed.");
    return 0;
}
