#include <openhouse/render/RenderDocument.hpp>

#include <openhouse/testing/Check.hpp>
#include <cstdio>
#include <string>

using namespace openhouse::document;
using namespace openhouse::geometry;
using namespace openhouse::render;

static void TestEmptyDocumentProducesEmptySvgBody() {
    const Document doc;
    SvgDocument svg;
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<circle") == std::string::npos);
    OH_CHECK(content.find("<line") == std::string::npos);
    OH_CHECK(content.find("<path") == std::string::npos);
    OH_CHECK(content.find("<svg") != std::string::npos); // shell still present
}

static void TestSingleLineRenders() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 10.0}});

    SvgDocument svg;
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<line") != std::string::npos);
    OH_CHECK(content.find(R"(x2="10")") != std::string::npos);
}

static void TestSingleCircleRenders() {
    Document doc;
    doc.Add(Circle2d{Point2d{5.0, 5.0}, 3.0});

    SvgDocument svg;
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<circle") != std::string::npos);
    OH_CHECK(content.find(R"(r="3")") != std::string::npos);
    OH_CHECK(content.find(R"(fill="none")") != std::string::npos); // outline, not point marker
}

static void TestSingleArcRenders() {
    Document doc;
    doc.Add(Arc2d{Point2d{0.0, 0.0}, 5.0, 0.0, 1.0});

    SvgDocument svg;
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<path") != std::string::npos);
}

static void TestMixedDocumentAllShapesRender() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}});
    doc.Add(Circle2d{Point2d{2.0, 2.0}, 1.0});
    doc.Add(Arc2d{Point2d{3.0, 3.0}, 1.0, 0.0, 1.0});
    doc.Add(Circle2d{Point2d{4.0, 4.0}, 2.0});

    SvgDocument svg;
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    // Count each element type to confirm nothing is dropped or duplicated.
    auto countOccurrences = [&](const std::string& needle) {
        std::size_t count = 0, pos = 0;
        while ((pos = content.find(needle, pos)) != std::string::npos) {
            ++count;
            pos += needle.size();
        }
        return count;
    };

    OH_CHECK(countOccurrences("<line") == 1);
    OH_CHECK(countOccurrences("<circle") == 2);
    OH_CHECK(countOccurrences("<path") == 1);
}

static void TestOrderIsPreserved() {
    // Shapes should render in the same order they were added -- matters
    // for correct visual stacking (later shapes drawn on top).
    Document doc;
    doc.Add(Circle2d{Point2d{1.0, 1.0}, 1.0});
    doc.Add(Circle2d{Point2d{2.0, 2.0}, 1.0});

    SvgDocument svg;
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    const auto firstPos = content.find(R"(cx="1")");
    const auto secondPos = content.find(R"(cx="2")");
    OH_CHECK(firstPos != std::string::npos);
    OH_CHECK(secondPos != std::string::npos);
    OH_CHECK(firstPos < secondPos);
}

// --- Layer consumer tests (Spiral 2 / DOC-003 Milestone 2.3) --------------

static void TestLayerColorAppliedToStroke() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}}, "Walls");
    doc.FindLayer("Walls")->SetColor("red");

    SvgDocument svg;
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find(R"(stroke="red")") != std::string::npos);
}

static void TestLayerLineWeightAppliedToStrokeWidth() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0}, "Structural");
    doc.FindLayer("Structural")->SetLineWeight(3.5);

    SvgDocument svg;
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find(R"(stroke-width="3.5")") != std::string::npos);
}

static void TestDashedLineTypeProducesDashArray() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}}, "Center");
    doc.FindLayer("Center")->SetLineType(LineType::Dashed);

    SvgDocument svg;
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("stroke-dasharray=") != std::string::npos);
}

static void TestContinuousLineTypeHasNoDashArray() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}}, "Walls");
    // LineType::Continuous is the default -- no explicit SetLineType call.

    SvgDocument svg;
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("stroke-dasharray=") == std::string::npos);
}

static void TestHiddenLayerEntityIsNotRendered() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 5.0}, "Hidden");
    doc.FindLayer("Hidden")->SetVisible(false);

    SvgDocument svg;
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    OH_CHECK(content.find("<circle") == std::string::npos);
}

// The exact scenario from Spiral 2's Milestone 2.4 demo requirement:
// Walls (black, solid), Center (gray, dashed), Hidden (invisible) --
// verifies all three behaviors together, matching what the demo
// executable is expected to produce.
static void TestThreeLayerScenarioMatchesSpiral2Spec() {
    Document doc;

    Layer& walls = doc.CreateLayer("Walls");
    walls.SetColor("black");

    Layer& center = doc.CreateLayer("Center");
    center.SetColor("gray");
    center.SetLineType(LineType::Dashed);

    Layer& hidden = doc.CreateLayer("Hidden");
    hidden.SetVisible(false);

    doc.Add(Line2d{Point2d{10.0, 10.0}, Point2d{90.0, 10.0}}, "Walls");
    doc.Add(Line2d{Point2d{10.0, 50.0}, Point2d{90.0, 50.0}}, "Center");
    doc.Add(Circle2d{Point2d{50.0, 80.0}, 10.0}, "Hidden");

    SvgDocument svg(100.0, 100.0);
    RenderToSvg(doc, svg);
    const std::string content = svg.ToString();

    // Walls: appears, black, no dash.
    OH_CHECK(content.find(R"(stroke="black")") != std::string::npos);
    // Center: appears, gray, dashed.
    OH_CHECK(content.find(R"(stroke="gray")") != std::string::npos);
    OH_CHECK(content.find("stroke-dasharray=") != std::string::npos);
    // Hidden: does not appear at all (its circle would be the only <circle>).
    OH_CHECK(content.find("<circle") == std::string::npos);
    // Exactly two <line> elements (Walls + Center), not three renderable
    // shapes -- Hidden's circle must be excluded, not just invisible-styled.
    std::size_t lineCount = 0, pos = 0;
    while ((pos = content.find("<line", pos)) != std::string::npos) {
        ++lineCount;
        pos += 5;
    }
    OH_CHECK(lineCount == 2);
}

// --- RenderOptions / selection highlight (SEL-003) -------------------------

static void TestEmptySelectionProducesIdenticalOutputToNoSelectionArgument() {
    // Regression test explicitly requested during SEL-003's design
    // review: adding RenderOptions must not change the output of the
    // pre-existing two-argument RenderToSvg(doc, svg) call at all.
    Document doc;
    doc.Add(Line2d{Point2d{10.0, 10.0}, Point2d{90.0, 10.0}}, "Walls");
    doc.Add(Circle2d{Point2d{50.0, 50.0}, 20.0}, "Fixtures");
    doc.Add(Arc2d{Point2d{0.0, 0.0}, 15.0, 0.0, 1.5}, "Doors");

    SvgDocument oldStyle(200.0, 200.0);
    RenderToSvg(doc, oldStyle); // pre-SEL-003 call site, unchanged

    SvgDocument newStyleEmpty(200.0, 200.0);
    RenderToSvg(doc, newStyleEmpty, RenderOptions{}); // explicit empty options

    OH_CHECK(oldStyle.ToString() == newStyleEmpty.ToString());
}

static void TestSelectionWithUnknownIdDoesNotCrashAndRendersNormally() {
    // Regression test explicitly requested during SEL-003's design
    // review: a stale/unrelated EntityId in the SelectionSet (e.g. left
    // over after Document::Clear()) must not crash RenderToSvg, and
    // must produce output identical to having no selection at all,
    // since it never matches any real entity.
    Document doc;
    doc.Add(Line2d{Point2d{10.0, 10.0}, Point2d{90.0, 10.0}}, "Walls");
    doc.Add(Circle2d{Point2d{50.0, 50.0}, 20.0}, "Fixtures");

    SvgDocument baseline(200.0, 200.0);
    RenderToSvg(doc, baseline);

    SelectionSet staleSelection;
    OH_CHECK(staleSelection.Select(999999)); // no entity in `doc` has this ID
    RenderOptions options;
    options.selection = &staleSelection;

    SvgDocument withStaleSelection(200.0, 200.0);
    RenderToSvg(doc, withStaleSelection, options);

    OH_CHECK(baseline.ToString() == withStaleSelection.ToString());
}

static void TestSelectedEntityGetsHighlightColorAndWiderStroke() {
    Document doc;
    doc.FindLayer("0")->SetColor("black");
    doc.FindLayer("0")->SetLineWeight(2.0);
    const EntityId lineId = doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}});

    SelectionSet sel;
    OH_CHECK(sel.Select(lineId));
    RenderOptions options;
    options.selection = &sel;

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg, options);
    const std::string content = svg.ToString();

    OH_CHECK(content.find(R"(stroke="#ff0000")") != std::string::npos);
    OH_CHECK(content.find(R"(stroke-width="3.5")") != std::string::npos); // 2.0 + 1.5
    OH_CHECK(content.find(R"(stroke="black")") == std::string::npos); // fully overridden
}

static void TestUnselectedEntityKeepsOriginalLayerStyle() {
    Document doc;
    doc.FindLayer("0")->SetColor("black");
    doc.FindLayer("0")->SetLineWeight(2.0);
    doc.Add(Circle2d{Point2d{50.0, 50.0}, 20.0}); // never selected

    const SelectionSet emptySel; // valid pointer, but selects nothing
    RenderOptions options;
    options.selection = &emptySel;

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg, options);
    const std::string content = svg.ToString();

    OH_CHECK(content.find(R"(stroke="black")") != std::string::npos);
    OH_CHECK(content.find(R"(stroke-width="2")") != std::string::npos);
    OH_CHECK(content.find("#ff0000") == std::string::npos);
}

static void TestSelectedEntityRendersAfterUnselectedRegardlessOfAddOrder() {
    // The line is added FIRST (so it would naturally draw first / lower
    // in Z-order under simple document-order rendering), but it's the
    // SELECTED one -- it must still end up appearing AFTER (on top of)
    // the unselected circle in the SVG output.
    Document doc;
    const EntityId lineId = doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}});
    doc.Add(Circle2d{Point2d{50.0, 50.0}, 20.0});

    SelectionSet sel;
    OH_CHECK(sel.Select(lineId));
    RenderOptions options;
    options.selection = &sel;

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg, options);
    const std::string content = svg.ToString();

    const auto circlePos = content.find("<circle");
    const auto linePos = content.find("<line");
    OH_CHECK(circlePos != std::string::npos);
    OH_CHECK(linePos != std::string::npos);
    OH_CHECK(circlePos < linePos); // unselected circle drawn before selected line
}

static void TestSelectedEntityKeepsItsOwnDashArray() {
    // Per SEL-003's design review: selection overrides color and
    // stroke-width, but NOT the dash pattern -- a selected dashed
    // centerline should still visually read as "dashed" while
    // highlighted, not snap to solid.
    Document doc;
    Layer& center = doc.CreateLayer("Center");
    center.SetLineType(LineType::Dashed);
    const EntityId id = doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}}, "Center");

    SelectionSet sel;
    OH_CHECK(sel.Select(id));
    RenderOptions options;
    options.selection = &sel;

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg, options);
    const std::string content = svg.ToString();

    OH_CHECK(content.find(R"(stroke="#ff0000")") != std::string::npos);
    OH_CHECK(content.find("stroke-dasharray=") != std::string::npos); // still dashed
}

static void TestNullSelectionPointerBehavesLikeNoSelection() {
    Document doc;
    doc.Add(Circle2d{Point2d{50.0, 50.0}, 20.0});

    RenderOptions options; // options.selection stays nullptr (the default)

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg, options); // must not dereference a null selection
    OH_CHECK(svg.ToString().find("#ff0000") == std::string::npos);
}

int main() {
    TestEmptyDocumentProducesEmptySvgBody();
    TestSingleLineRenders();
    TestSingleCircleRenders();
    TestSingleArcRenders();
    TestMixedDocumentAllShapesRender();
    TestOrderIsPreserved();

    TestLayerColorAppliedToStroke();
    TestLayerLineWeightAppliedToStrokeWidth();
    TestDashedLineTypeProducesDashArray();
    TestContinuousLineTypeHasNoDashArray();
    TestHiddenLayerEntityIsNotRendered();
    TestThreeLayerScenarioMatchesSpiral2Spec();

    TestEmptySelectionProducesIdenticalOutputToNoSelectionArgument();
    TestSelectionWithUnknownIdDoesNotCrashAndRendersNormally();
    TestSelectedEntityGetsHighlightColorAndWiderStroke();
    TestUnselectedEntityKeepsOriginalLayerStyle();
    TestSelectedEntityRendersAfterUnselectedRegardlessOfAddOrder();
    TestSelectedEntityKeepsItsOwnDashArray();
    TestNullSelectionPointerBehavesLikeNoSelection();

    std::puts("RenderDocumentTests: all tests passed.");
    return 0;
}
