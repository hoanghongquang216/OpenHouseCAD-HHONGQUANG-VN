#pragma once

#include <openhouse/document/Command.hpp>
#include <openhouse/document/Extend.hpp>

namespace openhouse::document {

// EXTEND-001's ICommand -- REFACTOR-001: shares EntityShapeCommandBase's
// Memento logic (Command.hpp) with TranslateCommand/RotateCommand/
// ScaleCommand and TrimCommand, instead of duplicating it.
class ExtendCommand final : public EntityShapeCommandBase {
public:
    ExtendCommand(EntityId targetId, EntityId boundaryId, geometry::Point2d clickPoint) noexcept
        : EntityShapeCommandBase(targetId), boundaryId_(boundaryId), clickPoint_(clickPoint) {}

protected:
    [[nodiscard]] bool DoOperation(Document& doc, EntityId id) override {
        return ExtendEntity(doc, id, boundaryId_, clickPoint_);
    }

private:
    EntityId boundaryId_;
    geometry::Point2d clickPoint_;
};

}
