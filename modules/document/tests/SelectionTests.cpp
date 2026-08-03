#include <openhouse/document/Selection.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>

using namespace openhouse::document;

static void TestEmptySelectionByDefault() {
    const SelectionSet sel;
    OH_CHECK(sel.Empty());
    OH_CHECK(sel.Count() == 0);
}

static void TestSelectAddsIdAndReturnsTrue() {
    SelectionSet sel;
    OH_CHECK(sel.Select(1)); // first time: actually changed something
    OH_CHECK(sel.IsSelected(1));
    OH_CHECK(sel.Count() == 1);
    OH_CHECK(!sel.Empty());
}

static void TestSelectingTwiceReturnsFalseOnSecondCall() {
    SelectionSet sel;
    OH_CHECK(sel.Select(5));  // first: changed -> true
    OH_CHECK(!sel.Select(5)); // second: no-op, already selected -> false
    OH_CHECK(sel.Count() == 1);
}

static void TestDeselectRemovesIdAndReturnsTrue() {
    SelectionSet sel;
    OH_CHECK(sel.Select(1));
    OH_CHECK(sel.Select(2));
    OH_CHECK(sel.Deselect(1)); // was selected -> removed -> true
    OH_CHECK(!sel.IsSelected(1));
    OH_CHECK(sel.IsSelected(2));
    OH_CHECK(sel.Count() == 1);
}

static void TestDeselectingUnselectedIdIsNoOpAndReturnsFalse() {
    SelectionSet sel;
    OH_CHECK(sel.Select(1));
    OH_CHECK(!sel.Deselect(999)); // never selected -> nothing changed -> false
    OH_CHECK(sel.Count() == 1);
    OH_CHECK(sel.IsSelected(1));
}

static void TestToggleSelectsWhenNotSelected() {
    SelectionSet sel;
    OH_CHECK(sel.Toggle(1)); // valid id, a change always happens -> true
    OH_CHECK(sel.IsSelected(1));
}

static void TestToggleDeselectsWhenSelected() {
    SelectionSet sel;
    OH_CHECK(sel.Select(1));
    OH_CHECK(sel.Toggle(1)); // still true: a change happened (removal)
    OH_CHECK(!sel.IsSelected(1));
    OH_CHECK(sel.Empty());
}

static void TestClearRemovesAllSelections() {
    SelectionSet sel;
    OH_CHECK(sel.Select(1));
    OH_CHECK(sel.Select(2));
    OH_CHECK(sel.Select(3));
    sel.Clear();
    OH_CHECK(sel.Empty());
    OH_CHECK(sel.Count() == 0);
    OH_CHECK(!sel.IsSelected(1));
}

static void TestMultipleIdsCoexist() {
    SelectionSet sel;
    OH_CHECK(sel.Select(1));
    OH_CHECK(sel.Select(2));
    OH_CHECK(sel.Select(3));
    OH_CHECK(sel.Count() == 3);
    OH_CHECK(sel.IsSelected(1));
    OH_CHECK(sel.IsSelected(2));
    OH_CHECK(sel.IsSelected(3));
    OH_CHECK(!sel.IsSelected(4));
}

static void TestSelectingInvalidEntityIdIsIgnoredAndReturnsFalse() {
    SelectionSet sel;
    OH_CHECK(!sel.Select(kInvalidEntityId)); // never a real change -> false
    OH_CHECK(sel.Empty());
    OH_CHECK(!sel.IsSelected(kInvalidEntityId));
}

static void TestTogglingInvalidEntityIdIsIgnoredAndReturnsFalse() {
    SelectionSet sel;
    OH_CHECK(!sel.Toggle(kInvalidEntityId)); // the one Toggle() case that's false
    OH_CHECK(sel.Empty());
}

static void TestDeselectingInvalidEntityIdReturnsFalse() {
    SelectionSet sel;
    OH_CHECK(!sel.Deselect(kInvalidEntityId));
}

static void TestIdsAccessorReflectsCurrentSelection() {
    SelectionSet sel;
    OH_CHECK(sel.Select(10));
    OH_CHECK(sel.Select(20));
    const auto& ids = sel.Ids();
    OH_CHECK(ids.size() == 2);
    OH_CHECK(ids.find(10) != ids.end());
    OH_CHECK(ids.find(20) != ids.end());
}

int main() {
    TestEmptySelectionByDefault();
    TestSelectAddsIdAndReturnsTrue();
    TestSelectingTwiceReturnsFalseOnSecondCall();
    TestDeselectRemovesIdAndReturnsTrue();
    TestDeselectingUnselectedIdIsNoOpAndReturnsFalse();
    TestToggleSelectsWhenNotSelected();
    TestToggleDeselectsWhenSelected();
    TestClearRemovesAllSelections();
    TestMultipleIdsCoexist();
    TestSelectingInvalidEntityIdIsIgnoredAndReturnsFalse();
    TestTogglingInvalidEntityIdIsIgnoredAndReturnsFalse();
    TestDeselectingInvalidEntityIdReturnsFalse();
    TestIdsAccessorReflectsCurrentSelection();

    std::puts("SelectionTests: all tests passed.");
    return 0;
}
