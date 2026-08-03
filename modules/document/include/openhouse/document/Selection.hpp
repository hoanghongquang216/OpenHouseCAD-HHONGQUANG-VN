#pragma once

#include <openhouse/document/EntityId.hpp>

#include <unordered_set>

namespace openhouse::document {

// A set of selected EntityIds. Deliberately NOT part of Document: a
// Document is drawing data (what a DXF file, once import exists, would
// load), while a selection is per-session UI/interaction state that has
// no meaning outside of an active editing session -- a Document loaded
// from disk has no opinion about what's selected. Keeping them separate
// also means a Document can be shared/duplicated without a selection
// being implicitly duplicated along with it.
//
// SelectionSet does NOT hold a reference to the Document its IDs came
// from, and does NOT validate that an ID it's given actually exists in
// any particular Document -- that's the caller's responsibility (e.g.
// HitTest, see the geometry module, only ever returns IDs that came
// from a real Document lookup). This keeps SelectionSet a simple,
// Document-independent value type; the alternative (SelectionSet
// checking against a live Document on every Select() call) would couple
// it to a specific Document instance and complicate the common case of
// swapping which Document is being edited.
class SelectionSet {
public:
    // Adds `id` to the selection. Returns true if this call actually
    // changed the selection (the ID wasn't already selected), false if
    // it was a no-op (already selected, or id == kInvalidEntityId).
    // The return value lets callers (Command/Undo, UI dirty-flagging)
    // distinguish "selection state changed" from "nothing to do"
    // without a separate IsSelected() check before calling.
    [[nodiscard]] bool Select(EntityId id) {
        if (id == kInvalidEntityId) {
            return false;
        }
        return ids_.insert(id).second;
    }

    // Removes `id` from the selection. Returns true if it was actually
    // selected (and is now removed), false if it wasn't selected to
    // begin with -- same "did this change anything" convention as
    // Select().
    [[nodiscard]] bool Deselect(EntityId id) { return ids_.erase(id) > 0; }

    // Selects `id` if not currently selected, deselects it if it is.
    // Always returns true for a valid id (some change always happens --
    // either an insertion or a removal), false only for
    // kInvalidEntityId (never a valid change).
    [[nodiscard]] bool Toggle(EntityId id) {
        if (id == kInvalidEntityId) {
            return false;
        }
        if (auto it = ids_.find(id); it != ids_.end()) {
            ids_.erase(it);
        } else {
            ids_.insert(id);
        }
        return true;
    }

    void Clear() noexcept { ids_.clear(); }

    [[nodiscard]] bool IsSelected(EntityId id) const noexcept {
        return ids_.find(id) != ids_.end();
    }

    [[nodiscard]] const std::unordered_set<EntityId>& Ids() const noexcept { return ids_; }

    [[nodiscard]] std::size_t Count() const noexcept { return ids_.size(); }

    [[nodiscard]] bool Empty() const noexcept { return ids_.empty(); }

private:
    std::unordered_set<EntityId> ids_;
};

}
