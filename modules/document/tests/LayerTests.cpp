#include <openhouse/document/Layer.hpp>

#include <cassert>
#include <cstdio>

using namespace openhouse::document;

static void TestConstructionDefaults() {
    const Layer layer("MyLayer");
    assert(layer.Name() == "MyLayer");
    assert(layer.Color() == "black");
    assert(layer.GetLineType() == LineType::Continuous);
    assert(layer.LineWeight() == 1.0);
    assert(layer.Visible());
    assert(!layer.Locked());
}

static void TestSetColor() {
    Layer layer("Walls");
    layer.SetColor("red");
    assert(layer.Color() == "red");
}

static void TestSetLineType() {
    Layer layer("Dimensions");
    layer.SetLineType(LineType::Dashed);
    assert(layer.GetLineType() == LineType::Dashed);
}

static void TestSetLineWeight() {
    Layer layer("Structural");
    layer.SetLineWeight(2.5);
    assert(layer.LineWeight() == 2.5);
}

static void TestVisibilityToggle() {
    Layer layer("Hidden");
    assert(layer.Visible());
    layer.SetVisible(false);
    assert(!layer.Visible());
    layer.SetVisible(true);
    assert(layer.Visible());
}

static void TestLockedToggle() {
    Layer layer("Locked");
    assert(!layer.Locked());
    layer.SetLocked(true);
    assert(layer.Locked());
}

static void TestNameImmutableAfterConstruction() {
    // No SetName() exists -- this is a compile-time guarantee, not a
    // runtime one. Nothing to assert here beyond construction; this
    // test exists as living documentation that Name() has no setter.
    const Layer layer("Fixed");
    assert(layer.Name() == "Fixed");
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
