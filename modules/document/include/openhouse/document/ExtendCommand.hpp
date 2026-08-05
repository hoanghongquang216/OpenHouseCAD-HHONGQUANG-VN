#pragma once

#include <openhouse/document/Command.hpp>
#include <openhouse/document/Extend.hpp>

namespace openhouse::document {

// EXTEND-001's own ICommand -- deliberately independent of
// TransformEntityCommandBase (Command.hpp) and of TrimCommand
// (TrimCommand.hpp), matching TRIM-001's own Go decision: no evidence
// yet that a shared Memento base pays for itself across Transform/
// Trim/Extend. Revisit only once further Editing commands repeat the
// same duplication.
class ExtendCommand final : public ICommand {
public:
    ExtendCommand(EntityId targetId, EntityId boundaryId, geometry::Point2d clickPoint) noexcept
        : targetId_(targetId), boundaryId_(boundaryId), clickPoint_(clickPoint) {}

    [[nodiscard]] bool Execute(Document& doc) override {
        const auto extended = ComputeExtend(doc, targetId_, boundaryId_, clickPoint_);
        if (!extended.has_value()) {
            return false;
        }
        Entity* entity = doc.FindEntityMutable(targetId_);
        if (entity == nullptr) {
            return false;
        }
        shapeBefore_ = entity->shape;
        entity->shape = *extended;
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
    EntityId boundaryId_;
    geometry::Point2d clickPoint_;
    Shape shapeBefore_{};
    Shape shapeAfter_{};
};

}
