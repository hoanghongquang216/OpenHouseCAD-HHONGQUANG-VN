#include <openhouse/document/Document.hpp>

#include <cassert>
#include <cstdio>
#include <type_traits>
#include <variant>

using namespace openhouse::document;
using namespace openhouse::geometry;

static void TestEmptyDocument() {
    const Document doc;
    assert(doc.Empty());
    assert(doc.Count() == 0);
}

static void TestAddLine() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{10.0, 10.0}});
    assert(!doc.Empty());
    assert(doc.Count() == 1);
}

static void TestAddMultipleShapeTypes() {
    Document doc;
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}});
    doc.Add(Circle2d{Point2d{5.0, 5.0}, 3.0});
    doc.Add(Arc2d{Point2d{0.0, 0.0}, 2.0, 0.0, 1.0});
    assert(doc.Count() == 3);
}

static void TestEntitiesAccessibleAndCorrectType() {
    Document doc;
    doc.Add(Circle2d{Point2d{1.0, 2.0}, 3.0});

    const auto& entities = doc.Entities();
    assert(entities.size() == 1);

    const auto* circle = std::get_if<Circle2d>(&entities[0].shape);
    assert(circle != nullptr);
    assert(circle->radius == 3.0);
    assert(circle->center.x == 1.0);
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
    assert(lineCount == 1);
    assert(circleCount == 1);
    assert(arcCount == 1);
}

static void TestClear() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    assert(doc.Count() == 1);
    doc.Clear();
    assert(doc.Empty());
    assert(doc.Count() == 0);
}

static void TestBoundsOfEmptyDocumentIsNullopt() {
    const Document doc;
    assert(!doc.Bounds().has_value());
}

static void TestBoundsOfSingleShape() {
    Document doc;
    doc.Add(Circle2d{Point2d{10.0, 10.0}, 5.0});
    const auto box = doc.Bounds();
    assert(box.has_value());
    assert(box->min.x == 5.0 && box->min.y == 5.0);
    assert(box->max.x == 15.0 && box->max.y == 15.0);
}

static void TestBoundsAggregatesMultipleShapes() {
    Document doc;
    doc.Add(Line2d{Point2d{-10.0, 0.0}, Point2d{0.0, 0.0}});
    doc.Add(Circle2d{Point2d{20.0, 20.0}, 2.0});

    const auto box = doc.Bounds();
    assert(box.has_value());
    // Union of line's bbox [-10,0]-[0,0] and circle's bbox [18,18]-[22,22].
    assert(box->min.x == -10.0);
    assert(box->min.y == 0.0);
    assert(box->max.x == 22.0);
    assert(box->max.y == 22.0);
}

// --- Layer-related tests (Spiral 2 / DOC-003 Milestone 2.2) ---------------

static void TestNewDocumentHasDefaultLayerZero() {
    const Document doc;
    assert(doc.Layers().size() == 1);
    const Layer* zero = doc.FindLayer("0");
    assert(zero != nullptr);
    assert(zero->Name() == "0");
}

static void TestAddWithNoLayerGoesToDefaultLayer() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0});
    assert(doc.Layers().size() == 1); // still just "0" -- no new layer created
    assert(doc.Entities().front().layer == "0");
}

static void TestAddWithNewLayerNameAutoCreatesIt() {
    Document doc;
    assert(doc.FindLayer("Walls") == nullptr);
    doc.Add(Line2d{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}}, "Walls");
    assert(doc.FindLayer("Walls") != nullptr);
    assert(doc.Layers().size() == 2); // "0" (from construction) + "Walls"
    assert(doc.Entities().front().layer == "Walls");
}

static void TestAddingToSameLayerTwiceDoesNotDuplicateIt() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Walls");
    doc.Add(Circle2d{Point2d{5.0, 5.0}, 2.0}, "Walls");
    assert(doc.Layers().size() == 2); // "0" + "Walls", not 3
    assert(doc.Count() == 2);
}

static void TestCreateLayerDirectlyIsIdempotent() {
    Document doc;
    Layer& first = doc.CreateLayer("Dimensions");
    first.SetColor("blue");
    Layer& second = doc.CreateLayer("Dimensions"); // should return the SAME layer
    assert(&first == &second);
    assert(second.Color() == "blue"); // not reset to default
    assert(doc.Layers().size() == 2); // "0" + "Dimensions", called twice but created once
}

static void TestBoundsSkipsHiddenLayer() {
    Document doc;
    doc.Add(Circle2d{Point2d{0.0, 0.0}, 1.0}, "Visible");
    doc.Add(Circle2d{Point2d{100.0, 100.0}, 1.0}, "Hidden");
    doc.FindLayer("Hidden")->SetVisible(false);

    const auto box = doc.Bounds();
    assert(box.has_value());
    // Only the "Visible" layer's circle (bbox [-1,-1]-[1,1]) should count;
    // if the hidden layer's far-away circle leaked in, max would be ~101.
    assert(box->max.x < 2.0);
    assert(box->max.y < 2.0);
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
