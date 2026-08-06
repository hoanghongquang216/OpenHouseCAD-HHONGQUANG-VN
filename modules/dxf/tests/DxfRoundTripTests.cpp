// Round-trip tests for DXF-EXPORT-001: Document -> WriteDxfStream ->
// ParseDxfStream -> Document, comparing the re-imported result against
// the original. Maps to docs/design/DXF-EXPORT-001-Test-Design.md
// Section 2 (R-001..006).

#include <openhouse/document/Document.hpp>
#include <openhouse/dxf/DxfReader.hpp>
#include <openhouse/dxf/DxfWriter.hpp>
#include <openhouse/foundation/String.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>

#include <openhouse/testing/Check.hpp>
#include <cmath>
#include <cstdio>
#include <sstream>

using namespace openhouse::document;
using namespace openhouse::geometry;
using namespace openhouse::dxf;

namespace foundation = openhouse::foundation;

namespace {

// Round-trips `doc` through WriteDxfStream -> ParseDxfStream, returning
// the re-imported Document. Aborts the test (via OH_CHECK) if either
// step fails -- a round-trip test's whole premise is that both steps
// succeed, so a failure here is itself the test failing, not a
// condition to branch on.
Document RoundTrip(const Document& doc) {
    std::ostringstream out;
    OH_CHECK(WriteDxfStream(doc, out));

    std::istringstream in(out.str());
    auto result = ParseDxfStream(in);
    OH_CHECK(result.has_value());
    return *result;
}

bool NearlyEqual(double a, double b, double epsilon = 1e-9) {
    return std::abs(a - b) <= epsilon;
}

} // namespace

// R-001: one of each supported shape -> round-trip -> geometry matches.
static void TestR001_OneOfEachShape_GeometryPreserved() {
    Document doc;
    doc.Add(Line2d{Point2d{1.5, 2.5}, Point2d{3.5, 4.5}});
    doc.Add(Circle2d{Point2d{10.0, 10.0}, 4.25});
    doc.Add(Arc2d{Point2d{-5.0, 5.0}, 2.0, 0.3, 1.7});

    const Document reimported = RoundTrip(doc);

    OH_CHECK(reimported.Count() == 3);

    bool foundLine = false, foundCircle = false, foundArc = false;
    for (const auto& entity : reimported.Entities()) {
        if (const auto* line = std::get_if<Line2d>(&entity.shape)) {
            foundLine = true;
            OH_CHECK(NearlyEqual(line->start.x, 1.5) && NearlyEqual(line->start.y, 2.5));
            OH_CHECK(NearlyEqual(line->end.x, 3.5) && NearlyEqual(line->end.y, 4.5));
        } else if (const auto* circle = std::get_if<Circle2d>(&entity.shape)) {
            foundCircle = true;
            OH_CHECK(NearlyEqual(circle->center.x, 10.0) && NearlyEqual(circle->center.y, 10.0));
            OH_CHECK(NearlyEqual(circle->radius, 4.25));
        } else if (const auto* arc = std::get_if<Arc2d>(&entity.shape)) {
            foundArc = true;
            OH_CHECK(NearlyEqual(arc->center.x, -5.0) && NearlyEqual(arc->center.y, 5.0));
            OH_CHECK(NearlyEqual(arc->radius, 2.0));
            OH_CHECK(NearlyEqual(arc->startAngle, 0.3, 1e-6)); // degree-conversion round-trip
            OH_CHECK(NearlyEqual(arc->endAngle, 1.7, 1e-6));
        }
    }
    OH_CHECK(foundLine && foundCircle && foundArc);
}

// R-002: layer names survive the round-trip.
static void TestR002_LayerNames_Preserved() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}}, "Walls");
    doc.Add(Line2d{Point2d{2.0, 2.0}, Point2d{3.0, 3.0}}, "Doors");

    const Document reimported = RoundTrip(doc);

    OH_CHECK(reimported.Count() == 2);
    bool foundWalls = false, foundDoors = false;
    for (const auto& entity : reimported.Entities()) {
        if (entity.layer == "Walls") foundWalls = true;
        if (entity.layer == "Doors") foundDoors = true;
    }
    OH_CHECK(foundWalls && foundDoors);
}

// R-003: a known-mappable color is a true bijection through the round-trip.
static void TestR003_KnownColor_ExactBijectionThroughRoundTrip() {
    Document doc;
    doc.CreateLayer("Colored").SetColor("#00ff00"); // ACI 3, per AciToSvgColor

    const Document reimported = RoundTrip(doc);

    const Layer* layer = reimported.FindLayer("Colored");
    OH_CHECK(layer != nullptr);
    OH_CHECK(layer->Color() == "#00ff00");
}

// R-004: an unknown color is the DOCUMENTED, ACCEPTED lossy edge (DG-003)
// -- it reverts to Layer's own default, not a bug.
static void TestR004_UnknownColor_RevertsToDefaultNotABug() {
    Document doc;
    doc.CreateLayer("Weird").SetColor("#3a7bd5");

    const Document reimported = RoundTrip(doc);

    const Layer* layer = reimported.FindLayer("Weird");
    OH_CHECK(layer != nullptr);
    OH_CHECK(layer->Color() == "black"); // Layer's own default, per DG-003
}

// R-005: every LineType round-trips losslessly (closed bijection, unlike color).
static void TestR005_EveryLineType_LosslessRoundTrip() {
    const LineType types[] = {LineType::Continuous, LineType::Dashed, LineType::Dotted,
                               LineType::DashDot};
    for (const LineType type : types) {
        Document doc;
        doc.CreateLayer("L").SetLineType(type);

        const Document reimported = RoundTrip(doc);

        const Layer* layer = reimported.FindLayer("L");
        OH_CHECK(layer != nullptr);
        OH_CHECK(layer->GetLineType() == type);
    }
}

// R-006: a negative (clockwise) Arc sweep survives the round-trip, sign included.
static void TestR006_NegativeArcSweep_SignPreserved() {
    Document doc;
    // endAngle < startAngle -> negative sweep (clockwise), per Arc2.hpp's
    // own Sweep() convention.
    doc.Add(Arc2d{Point2d{0.0, 0.0}, 1.0, 1.0, 0.0});

    const Document reimported = RoundTrip(doc);

    OH_CHECK(reimported.Count() == 1);
    const auto* arc = std::get_if<Arc2d>(&reimported.Entities().front().shape);
    OH_CHECK(arc != nullptr);
    OH_CHECK(NearlyEqual(arc->startAngle, 1.0, 1e-6));
    OH_CHECK(NearlyEqual(arc->endAngle, 0.0, 1e-6));
    OH_CHECK(Sweep(*arc) < 0.0); // sign preserved, not just magnitude
}

int main() {
    TestR001_OneOfEachShape_GeometryPreserved();
    TestR002_LayerNames_Preserved();
    TestR003_KnownColor_ExactBijectionThroughRoundTrip();
    TestR004_UnknownColor_RevertsToDefaultNotABug();
    TestR005_EveryLineType_LosslessRoundTrip();
    TestR006_NegativeArcSweep_SignPreserved();

    std::puts("DxfRoundTripTests: all tests passed.");
    return 0;
}
