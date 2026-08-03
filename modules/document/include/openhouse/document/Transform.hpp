#pragma once

#include <openhouse/document/Document.hpp>
#include <openhouse/document/Selection.hpp>
#include <openhouse/geometry/Transform.hpp>
#include <openhouse/geometry/Vector2.hpp>

#include <type_traits>
#include <variant>

namespace openhouse::document {

// TranslateEntity/RotateEntity/ScaleEntity (Spiral 4, TRF-001/002/003)
// and their *Selection counterparts are the document-layer half of
// Transform.hpp's split (see geometry::Transform.hpp's own comment):
// this is where CAD-specific business rules are enforced -- an entity
// that doesn't exist, or lives on a hidden or locked layer, rejects the
// operation entirely rather than silently doing nothing to the wrong
// thing or partially applying it.
//
// Locked layer semantics (locked in during Spiral 4's design review --
// see docs/ARCHITECTURE_DECISION_RECORDS and Layer.hpp's own comment):
//   Visible=false -> HitTest rejects, Selection is moot, Transform rejects.
//   Locked=true, Visible=true -> HitTest succeeds, Selection succeeds,
//                                 but Transform is REJECTED.
// This is Locked's first real consumer: it was stored since Spiral 2
// with no behavior, explicitly deferred to "whichever Spiral first does
// editing" (see Layer.hpp) -- this is that Spiral.
//
// Returning bool (Entity-level) / std::size_t (Selection-level) rather
// than void is deliberate, per Spiral 4's design review: Spiral 5's
// Command/Undo system needs to know whether an operation actually did
// anything (an Undo entry for a no-op Move is meaningless), and a
// partial-success count for a multi-entity selection (some entities
// locked, some not) is more useful to a caller/UI than an all-or-
// nothing result would be.

// True if `id` resolves to an entity whose layer permits editing
// (exists, visible, not locked). Shared by every *Entity function below
// rather than duplicating the same three checks in each one.
[[nodiscard]] inline bool CanTransform(const Document& doc, EntityId id) noexcept {
    const Entity* entity = doc.FindEntity(id);
    if (entity == nullptr) {
        return false;
    }
    const Layer* layer = doc.FindLayer(entity->layer);
    return layer != nullptr && layer->Visible() && !layer->Locked();
}

[[nodiscard]] inline bool TranslateEntity(Document& doc, EntityId id, geometry::Vector2d delta) {
    if (!CanTransform(doc, id)) {
        return false;
    }
    Entity* entity = doc.FindEntityMutable(id);
    entity->shape = std::visit(
        [delta](const auto& shape) -> Shape { return geometry::Translate(shape, delta); },
        entity->shape);
    return true;
}

// Applies TranslateEntity to every entity in `selection`, and returns
// how many actually moved (not the selection's total size -- an entity
// that no longer exists, or that's on a locked/hidden layer, doesn't
// count). Deliberately loops calling TranslateEntity rather than
// duplicating its logic, so the two never drift apart.
[[nodiscard]] inline std::size_t TranslateSelection(Document& doc, const SelectionSet& selection,
                                                     geometry::Vector2d delta) {
    std::size_t count = 0;
    for (const EntityId id : selection.Ids()) {
        if (TranslateEntity(doc, id, delta)) {
            ++count;
        }
    }
    return count;
}

}
