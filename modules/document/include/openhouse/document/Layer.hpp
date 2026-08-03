#pragma once

#include <openhouse/foundation/String.hpp>
#include <openhouse/foundation/Utility.hpp>

namespace openhouse::document {

// Matches DXF/AutoCAD's line-type vocabulary at a minimal level -- this
// is deliberately a small, closed set (not a string, not extensible)
// since only these need visual representation in SvgDocument for now.
// Extend when a real need appears (e.g. DXF import encounters a type not
// listed here), not speculatively.
enum class LineType {
    Continuous,
    Dashed,
    Dotted,
    DashDot,
};

// A drawing layer: name, color, line type/weight, visibility, and a
// lock flag. `name` is the layer's identity (matches DXF/AutoCAD
// convention, where layer name IS the key -- no separate numeric ID),
// and is immutable after construction: renaming a layer would require
// updating every Entity that references it by name, which is a real
// operation this Spiral does not implement yet. Every other field is
// mutable via setters.
//
// TODO(Spiral4):
// Replace layer name references with stable LayerId if DXF import/export
// or layer rename requires it. Using the name as the key is deliberate
// for now (it matches DXF's own model and keeps Entity trivially
// copyable/serializable), but it makes rename an O(entities) operation
// that must touch every Entity -- a stable ID would decouple identity
// from display name.
//
// `color` is stored as a plain string compatible with SvgDocument's
// existing color parameter (e.g. "black", "#ff0000"), not a structured
// RGB/ACI type -- SVG is the only output target that exists right now
// (see docs/ROADMAP_EXECUTION.md).
//
// TODO(Spiral4):
// Introduce a Color abstraction when multiple render backends or DXF ACI
// (AutoCAD Color Index) support becomes necessary. A raw SVG color
// string cannot represent ACI indices, and each render backend would
// otherwise need its own string-parsing/translation logic.
//
// `locked` is stored but has NO behavior yet -- there is no
// Selection/Editing system to respect it. This is an intentional,
// forward-declared field (Spiral 2's explicit scope per the roadmap
// includes it), not unused-code drift: Selection (a later Spiral) is
// the concrete, already-planned consumer.
class Layer {
public:
    explicit Layer(foundation::string name) : name_(foundation::move(name)) {}

    [[nodiscard]] const foundation::string& Name() const noexcept { return name_; }

    [[nodiscard]] const foundation::string& Color() const noexcept { return color_; }
    void SetColor(foundation::string color) { color_ = foundation::move(color); }

    [[nodiscard]] LineType GetLineType() const noexcept { return lineType_; }
    void SetLineType(LineType lineType) noexcept { lineType_ = lineType; }

    [[nodiscard]] double LineWeight() const noexcept { return lineWeight_; }
    void SetLineWeight(double lineWeight) noexcept { lineWeight_ = lineWeight; }

    [[nodiscard]] bool Visible() const noexcept { return visible_; }
    void SetVisible(bool visible) noexcept { visible_ = visible; }

    [[nodiscard]] bool Locked() const noexcept { return locked_; }
    void SetLocked(bool locked) noexcept { locked_ = locked; }

private:
    foundation::string name_;
    foundation::string color_{"black"};
    LineType lineType_ = LineType::Continuous;
    double lineWeight_ = 1.0;
    bool visible_ = true;
    bool locked_ = false;
};

}
