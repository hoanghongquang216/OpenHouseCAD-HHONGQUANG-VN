#pragma once

#include <openhouse/document/Command.hpp>
#include <openhouse/document/Trim.hpp>

namespace openhouse::document {

// TRIM-001's own ICommand -- deliberately independent of
// TransformEntityCommandBase (Command.hpp), per the TRIM-001 audit's
// Go decision: no evidence yet that Trim and Translate/Rotate/Scale
// share enough Memento logic to justify a common base.
class TrimCommand final : public ICommand {
public:
    TrimCommand(EntityId targetId, EntityId cutterId, geometry::Point2d clickPoint) noexcept
        : targetId_(targetId), cutterId_(cutterId), clickPoint_(clickPoint) {}

    [[nodiscard]] bool Execute(Document& doc) override {
        const auto trimmed = ComputeTrim(doc, targetId_, cutterId_, clickPoint_);
        if (!trimmed.has_value()) {
            return false;
        }
        Entity* entity = doc.FindEntityMutable(targetId_);
        if (entity == nullptr) {
            return false;
        }
        shapeBefore_ = entity->shape;
        entity->shape = *trimmed;
        shapeAfter_ = entity->shape;
        return true;
    }

    void Undo(Document& doc) override {
        if (Entity* entity = doc.FindEntityMutable(targetId_)) {
            entity->shape = shapeBefore_;
        }
    }

    void Redo(Document& doc) override {
        if (Entity* entity = doc.FindEntityMutable(targetId_)) {
            entity->shape = shapeAfter_;
        }
    }

private:
    EntityId targetId_;
    EntityId cutterId_;
    geometry::Point2d clickPoint_;
    Shape shapeBefore_{};
    Shape shapeAfter_{};
};

}
