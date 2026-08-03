#include <openhouse/document/Document.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>
#include <type_traits>
#include <variant>

using namespace openhouse::document;
using namespace openhouse::geometry;

static void TestEmptyDocument() {
    const Document doc;
    OH_CHECK(doc.Empty());
    OH_CHECK(doc.Count() == 0);
}

static void TestAddLine() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 10.0}});
    OH_CHECK(!doc.Empty());
    OH_CHECK(doc.Count() == 1);
}

static void TestAddMultipleShapeTypes() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}});
    doc.Add(Circle2d{Point2d{5.0, 5.0}, 3.0});
    doc.Add(Arc2d{Point2d{0.0, 0.0}, 2.0, 0.0, 1.0});
    OH_CHECK(doc.Count() == 3);
}

static void TestEntitiesAccessibleAndCorrectType() {
    Document doc;
    doc.Add(Circle2d{Point2d{1.0, 2.0}, 3.0});

    const auto& entities = doc.Entities();
    OH_CHECK(entities.size() == 1);

    const auto* circle = std::get_if<Circle2d>(&entities[0].shape);
    OH_CHECK(circle != nullptr);
    OH_CHECK(circle->radius == 3.0);
    OH_CHECK(circle->center.x == 1.0);
}

static void TestVisitAllShapes() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 0.0}});
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    doc.Add(Arc2d{Point2d{0.0, 0.0}, 1.0, 0.0, 1.0});

    int lineCount = 0, circleCount = 0, arcCount = 0;
    for (const auto& entity : doc.Entities()) {
        std::visit(
            [&](const auto& s) {
                using T = std::decay_t<decltype(s)>;
                if constexpr (std::is_same_v<T, Line2d>) {
                    ++lineCount;
                } else if constexpr (std::is_same_v<T, Circle2d>) {
                    ++circleCount;
                } else if constexpr (std::is_same_v<T, Arc2d>) {
                    ++arcCount;
                }
            },
            entity.shape);
    }
    OH_CHECK(lineCount == 1);
    OH_CHECK(circleCount == 1);
    OH_CHECK(arcCount == 1);
}

static void TestClear() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    OH_CHECK(doc.Count() == 1);
    doc.Clear();
    OH_CHECK(doc.Empty());
    OH_CHECK(doc.Count() == 0);
}

static void TestBoundsOfEmptyDocumentIsNullopt() {
    const Document doc;
    OH_CHECK(!doc.Bounds().has_value());
}

static void TestBoundsOfSingleShape() {
    Document doc;
    doc.Add(Circle2d{Point2d{10.0, 10.0}, 5.0});
    const auto box = doc.Bounds();
    OH_CHECK(box.has_value());
    OH_CHECK(box->min.x == 5.0 && box->min.y == 5.0);
    OH_CHECK(box->max.x == 15.0 && box->max.y == 15.0);
}

static void TestBoundsAggregatesMultipleShapes() {
    Document doc;
    doc.Add(Line2d{Point2d{-10.0, 0.0}, Point2d{0.0, 0.0}});
    doc.Add(Circle2d{Point2d{20.0, 20.0}, 2.0});

    const auto box = doc.Bounds();
    OH_CHECK(box.has_value());
    // Union of line's bbox [-10,0]-[0,0] and circle's bbox [18,18]-[22,22].
    OH_CHECK(box->min.x == -10.0);
    OH_CHECK(box->min.y == 0.0);
    OH_CHECK(box->max.x == 22.0);
    OH_CHECK(box->max.y == 22.0);
}

// --- Layer-related tests (Spiral 2 / DOC-003 Milestone 2.2) ---------------

static void TestNewDocumentHasDefaultLayerZero() {
    const Document doc;
    OH_CHECK(doc.Layers().size() == 1);
    const Layer* zero = doc.FindLayer("0");
    OH_CHECK(zero != nullptr);
    OH_CHECK(zero->Name() == "0");
}

static void TestAddWithNoLayerGoesToDefaultLayer() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    OH_CHECK(doc.Layers().size() == 1); // still just "0" -- no new layer created
    OH_CHECK(doc.Entities().front().layer == "0");
}

static void TestAddWithNewLayerNameAutoCreatesIt() {
    Document doc;
    OH_CHECK(doc.FindLayer("Walls") == nullptr);
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}}, "Walls");
    OH_CHECK(doc.FindLayer("Walls") != nullptr);
    OH_CHECK(doc.Layers().size() == 2); // "0" (from construction) + "Walls"
    OH_CHECK(doc.Entities().front().layer == "Walls");
}

static void TestAddingToSameLayerTwiceDoesNotDuplicateIt() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Walls");
    doc.Add(Circle2d{Point2d{5.0, 5.0}, 2.0}, "Walls");
    OH_CHECK(doc.Layers().size() == 2); // "0" + "Walls", not 3
    OH_CHECK(doc.Count() == 2);
}

static void TestCreateLayerDirectlyIsIdempotent() {
    Document doc;
    Layer& first = doc.CreateLayer("Dimensions");
    first.SetColor("blue");
    Layer& second = doc.CreateLayer("Dimensions"); // should return the SAME layer
    OH_CHECK(&first == &second);
    OH_CHECK(second.Color() == "blue"); // not reset to default
    OH_CHECK(doc.Layers().size() == 2); // "0" + "Dimensions", called twice but created once
}

static void TestBoundsSkipsHiddenLayer() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Visible");
    doc.Add(Circle2d{Point2d{100.0, 100.0}, 1.0}, "Hidden");
    doc.FindLayer("Hidden")->SetVisible(false);

    const auto box = doc.Bounds();
    OH_CHECK(box.has_value());
    // Only the "Visible" layer's circle (bbox [-1,-1]-[1,1]) should count;
    // if the hidden layer's far-away circle leaked in, max would be ~101.
    OH_CHECK(box->max.x < 2.0);
    OH_CHECK(box->max.y < 2.0);
}

int main() {
    TestEmptyDocument();
    TestAddLine();
    TestAddMultipleShapeTypes();
    TestEntitiesAccessibleAndCorrectType();
    TestVisitAllShapes();
    TestClear();
    TestBoundsOfEmptyDocumentIsNullopt();
    TestBoundsOfSingleShape();
    TestBoundsAggregatesMultipleShapes();

    TestNewDocumentHasDefaultLayerZero();
    TestAddWithNoLayerGoesToDefaultLayer();
    TestAddWithNewLayerNameAutoCreatesIt();
    TestAddingToSameLayerTwiceDoesNotDuplicateIt();
    TestCreateLayerDirectlyIsIdempotent();
    TestBoundsSkipsHiddenLayer();

    std::puts("DocumentTests: all tests passed.");
    return 0;
}
