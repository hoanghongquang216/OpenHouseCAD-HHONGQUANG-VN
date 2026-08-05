#pragma once

#include <openhouse/document/Command.hpp>
#include <openhouse/document/Trim.hpp>

namespace openhouse::document {

// TRIM-001's ICommand -- REFACTOR-001: shares EntityShapeCommandBase's
// Memento logic (Command.hpp) with TranslateCommand/RotateCommand/
// ScaleCommand and ExtendCommand, instead of duplicating it. Only
// supplies the operation-specific inputs (cutterId_, clickPoint_) and
// the DoOperation hook.
class TrimCommand final : public EntityShapeCommandBase {
public:
    TrimCommand(EntityId targetId, EntityId cutterId, geometry::Point2d clickPoint) noexcept
        : EntityShapeCommandBase(targetId), cutterId_(cutterId), clickPoint_(clickPoint) {}

protected:
    [[nodiscard]] bool DoOperation(Document& doc, EntityId id) override {
        return TrimEntity(doc, id, cutterId_, clickPoint_);
    }

private:
    EntityId cutterId_;
    geometry::Point2d clickPoint_;
};

}
