#include <openhouse/document/Layer.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>

using namespace openhouse::document;

static void TestConstructionDefaults() {
    const Layer layer("MyLayer");
    OH_CHECK(layer.Name() == "MyLayer");
    OH_CHECK(layer.Color() == "black");
    OH_CHECK(layer.GetLineType() == LineType::Continuous);
    OH_CHECK(layer.LineWeight() == 1.0);
    OH_CHECK(layer.Visible());
    OH_CHECK(!layer.Locked());
}

static void TestSetColor() {
    Layer layer("Walls");
    layer.SetColor("red");
    OH_CHECK(layer.Color() == "red");
}

static void TestSetLineType() {
    Layer layer("Dimensions");
    layer.SetLineType(LineType::Dashed);
    OH_CHECK(layer.GetLineType() == LineType::Dashed);
}

static void TestSetLineWeight() {
    Layer layer("Structural");
    layer.SetLineWeight(2.5);
    OH_CHECK(layer.LineWeight() == 2.5);
}

static void TestVisibilityToggle() {
    Layer layer("Hidden");
    OH_CHECK(layer.Visible());
    layer.SetVisible(false);
    OH_CHECK(!layer.Visible());
    layer.SetVisible(true);
    OH_CHECK(layer.Visible());
}

static void TestLockedToggle() {
    Layer layer("Locked");
    OH_CHECK(!layer.Locked());
    layer.SetLocked(true);
    OH_CHECK(layer.Locked());
}

static void TestNameImmutableAfterConstruction() {
    // No SetName() exists -- this is a compile-time guarantee, not a
    // runtime one. Nothing to assert here beyond construction; this
    // test exists as living documentation that Name() has no setter.
    const Layer layer("Fixed");
    OH_CHECK(layer.Name() == "Fixed");
}

int main() {
    TestConstructionDefaults();
    TestSetColor();
    TestSetLineType();
    TestSetLineWeight();
    TestVisibilityToggle();
    TestLockedToggle();
    TestNameImmutableAfterConstruction();

    std::puts("LayerTests: all tests passed.");
    return 0;
}
