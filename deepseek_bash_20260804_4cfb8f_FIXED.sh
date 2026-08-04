#!/bin/bash

# =============================================================================
# OPENHOUSECAD SPRINT PACKAGE GENERATOR
# =============================================================================
# Tạo toàn bộ cấu trúc thư mục và file cho:
#   - DS-004: DXF Test Assets
#   - DS-005: SVG Regression Test Suite
#   - Tài liệu: ENGINEERING_PRINCIPLES.md (cập nhật)
#   - Tài liệu hướng dẫn: Handover, Review Checklist, README
# =============================================================================

set -e

echo "========================================================================"
echo "  OPENHOUSECAD SPRINT PACKAGE GENERATOR"
echo "========================================================================"
echo ""

# =============================================================================
# 1. Tạo cấu trúc thư mục
# =============================================================================

echo "📁 Creating directory structure..."

mkdir -p tests/data/dxf/malformed
mkdir -p tests/svg
mkdir -p tests/expected/svg
mkdir -p docs/dxf
mkdir -p docs/svg

echo "✅ Directory structure created."
echo ""

# =============================================================================
# 2. DS-004: DXF Test Assets
# =============================================================================

echo "📄 Generating DXF test assets (DS-004)..."

# 2.1 empty.dxf
cat > tests/data/dxf/empty.dxf << 'DXF_END'
0
SECTION
2
HEADER
0
ENDSEC
0
SECTION
2
TABLES
0
ENDSEC
0
SECTION
2
ENTITIES
0
ENDSEC
0
EOF
DXF_END

# 2.2 line.dxf
cat > tests/data/dxf/line.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
LINE
8
0
10
0.0
20
0.0
11
5.0
21
0.0
0
ENDSEC
0
EOF
DXF_END

# 2.3 circle.dxf
cat > tests/data/dxf/circle.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
CIRCLE
8
0
10
0.0
20
0.0
40
2.0
0
ENDSEC
0
EOF
DXF_END

# 2.4 arc.dxf
cat > tests/data/dxf/arc.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
ARC
8
0
10
0.0
20
0.0
40
2.0
50
0.0
51
90.0
0
ENDSEC
0
EOF
DXF_END

# 2.5 lwpolyline_open.dxf
cat > tests/data/dxf/lwpolyline_open.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
LWPOLYLINE
8
0
90
4
70
0
10
0.0
20
0.0
10
3.0
20
0.0
10
3.0
20
3.0
10
0.0
20
3.0
0
ENDSEC
0
EOF
DXF_END

# 2.6 lwpolyline_closed.dxf
cat > tests/data/dxf/lwpolyline_closed.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
LWPOLYLINE
8
0
90
4
70
1
10
0.0
20
0.0
10
3.0
20
0.0
10
3.0
20
3.0
10
0.0
20
3.0
0
ENDSEC
0
EOF
DXF_END

# 2.7 lwpolyline_bulge.dxf
cat > tests/data/dxf/lwpolyline_bulge.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
LWPOLYLINE
8
0
90
4
70
1
10
0.0
20
0.0
10
3.0
20
0.0
10
3.0
20
3.0
10
0.0
20
3.0
42
1.0
0
ENDSEC
0
EOF
DXF_END

# 2.8 multiple_entities.dxf
cat > tests/data/dxf/multiple_entities.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
LINE
8
0
10
0.0
20
0.0
11
5.0
21
0.0
0
CIRCLE
8
0
10
2.0
20
2.0
40
1.0
0
ARC
8
0
10
4.0
20
4.0
40
2.0
50
0.0
51
90.0
0
LWPOLYLINE
8
0
90
3
70
0
10
0.0
20
0.0
10
3.0
20
0.0
10
3.0
20
3.0
0
ENDSEC
0
EOF
DXF_END

# 2.9 unsupported_entities.dxf
cat > tests/data/dxf/unsupported_entities.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
TEXT
8
0
10
0.0
20
0.0
40
0.5
1
HELLO
0
ELLIPSE
8
0
10
0.0
20
0.0
11
1.0
21
0.0
40
0.5
0
SPLINE
8
0
0
IMAGE
8
0
0
HATCH
8
0
0
LINE
8
0
10
0.0
20
0.0
11
5.0
21
0.0
0
ENDSEC
0
EOF
DXF_END

# 2.10 layer_test.dxf
cat > tests/data/dxf/layer_test.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
LINE
8
WALL
10
0.0
20
0.0
11
5.0
21
0.0
0
CIRCLE
8
COLUMN
10
2.0
20
2.0
40
1.0
0
ARC
8
DOOR
10
4.0
20
4.0
40
2.0
50
0.0
51
90.0
0
LWPOLYLINE
8
FLOOR
90
3
70
0
10
0.0
20
0.0
10
3.0
20
0.0
10
3.0
20
3.0
0
ENDSEC
0
EOF
DXF_END

# 2.11 house_floor_01.dxf
cat > tests/data/dxf/house_floor_01.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
LWPOLYLINE
8
WALL
90
6
70
1
10
0.0
20
0.0
10
10.0
20
0.0
10
10.0
20
6.0
42
0.5
10
7.0
20
6.0
10
7.0
20
4.0
10
0.0
20
4.0
0
LWPOLYLINE
8
DOOR
90
4
70
0
10
3.0
20
0.0
10
3.5
20
0.0
10
3.5
20
1.0
10
3.0
20
1.0
0
CIRCLE
8
COLUMN
10
5.0
20
3.0
40
0.5
0
LWPOLYLINE
8
WINDOW
90
4
70
1
10
1.0
20
4.0
10
2.0
20
4.0
10
2.0
20
4.5
10
1.0
20
4.5
0
ARC
8
FURNITURE
10
8.0
20
2.0
40
0.8
50
0.0
51
180.0
0
LINE
8
FURNITURE
10
8.0
20
2.0
11
8.0
21
2.8
0
LINE
8
FURNITURE
10
8.0
20
2.8
11
8.8
21
2.8
0
CIRCLE
8
FURNITURE
10
6.0
20
2.0
40
0.6
0
0
ENDSEC
0
EOF
DXF_END

# 2.12 room_simple.dxf
cat > tests/data/dxf/room_simple.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
LWPOLYLINE
8
0
90
4
70
1
10
0.0
20
0.0
10
10.0
20
0.0
10
10.0
20
8.0
10
0.0
20
8.0
0
ENDSEC
0
EOF
DXF_END

# 2.13 room_with_arcs.dxf
cat > tests/data/dxf/room_with_arcs.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
LWPOLYLINE
8
0
90
4
70
1
10
0.0
20
0.0
10
10.0
20
0.0
42
0.5
10
10.0
20
8.0
42
0.5
10
0.0
20
8.0
0
ENDSEC
0
EOF
DXF_END

# 2.14 malformed/missing_layer.dxf
cat > tests/data/dxf/malformed/missing_layer.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
LINE
10
0.0
20
0.0
11
5.0
21
0.0
0
ENDSEC
0
EOF
DXF_END

# 2.15 malformed/unknown_entity.dxf
cat > tests/data/dxf/malformed/unknown_entity.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
UNKNOWN_ENTITY
8
0
10
0.0
20
0.0
0
LINE
8
0
10
0.0
20
0.0
11
5.0
21
0.0
0
ENDSEC
0
EOF
DXF_END

# 2.16 malformed/invalid_bulge.dxf
cat > tests/data/dxf/malformed/invalid_bulge.dxf << 'DXF_END'
0
SECTION
2
ENTITIES
0
LWPOLYLINE
8
0
90
2
70
1
10
0.0
20
0.0
42
1.0
0
ENDSEC
0
EOF
DXF_END

echo "✅ DXF test assets created (16 files)."
echo ""

# =============================================================================
# 3. DS-004: Documentation
# =============================================================================

echo "📄 Generating documentation (DS-004)..."

cat > docs/dxf/TEST_CASES.md << 'DOC_END'
# DXF Test Cases

This document describes the test assets in `tests/data/dxf/` and their expected behavior when parsed by OpenHouseCAD's DXF importer.

---

## Test Files

| File | Purpose | Expected Result | Used By |
|------|---------|-----------------|---------|
| `empty.dxf` | File without entities | Empty Document, no error | `DxfReaderTests.cpp` |
| `line.dxf` | Basic LINE entity | 1 Line2d | `DxfReaderTests.cpp` |
| `circle.dxf` | Basic CIRCLE entity | 1 Circle2d | `DxfReaderTests.cpp` |
| `arc.dxf` | Basic ARC entity (0° → 90°) | 1 Arc2d | `DxfReaderTests.cpp` |
| `lwpolyline_open.dxf` | Open LWPOLYLINE (4 vertices) | 3 Line2d (no closing segment) | `DxfReaderTests.cpp` |
| `lwpolyline_closed.dxf` | Closed LWPOLYLINE (rectangle) | 4 Line2d | `DxfReaderTests.cpp` |
| `lwpolyline_bulge.dxf` | Closed LWPOLYLINE with bulge=1 | 3 Line2d + 1 Arc2d (90°) | `DxfReaderTests.cpp` |
| `multiple_entities.dxf` | Mixed LINE, CIRCLE, ARC, LWPOLYLINE | 4 entities | `DxfReaderTests.cpp`, Integration |
| `unsupported_entities.dxf` | TEXT, ELLIPSE, SPLINE, IMAGE, HATCH | 1 LINE, skip others | `DxfReaderTests.cpp` |
| `layer_test.dxf` | Multiple layers (WALL, COLUMN, DOOR, FLOOR) | 4 entities with correct layers | `DxfReaderTests.cpp`, Regression |
| `house_floor_01.dxf` | Realistic floor plan (8 entities) | 8 entities with correct layers | Regression, Manual Demo |
| `room_simple.dxf` | Simple rectangular room | 4 Line2d | `DxfReaderTests.cpp` |
| `room_with_arcs.dxf` | Room with 2 rounded corners (bulge=0.5) | 2 Line2d + 2 Arc2d | `DxfReaderTests.cpp` |

---

## Malformed / Edge Cases

| File | Purpose | Expected Result | Used By |
|------|---------|-----------------|---------|
| `malformed/missing_layer.dxf` | Entity without layer (group 8) | Skip entity, continue parsing | `DxfReaderTests.cpp` |
| `malformed/unknown_entity.dxf` | Unknown entity type | Skip unknown entity, continue parsing | `DxfReaderTests.cpp` |
| `malformed/invalid_bulge.dxf` | Missing vertex data (incomplete LWPOLYLINE) | Skip invalid entity, continue parsing | `DxfReaderTests.cpp` |

---

## Usage

These files can be used for:
- Unit tests in `tests/dxf/DxfReaderTests.cpp`
- Integration tests (CI)
- Manual testing with `dxf_import_demo`
- Regression testing when adding new entity types

---

## Notes

- Files are created by hand, not generated from CAD software.
- All files follow DXF ASCII format (AutoCAD 2010 compatible).
- Coordinates are in 2D (X, Y). Z is omitted.
- For `arc.dxf`: angles are in degrees (group 50 and 51).
- `empty.dxf` validates that the parser handles files without ENTITIES section.
DOC_END

echo "✅ Documentation created: docs/dxf/TEST_CASES.md"
echo ""

# =============================================================================
# 4. DS-005: SVG Regression Test Suite
# =============================================================================

echo "📄 Generating SVG regression test suite (DS-005)..."

# 4.1 SvgRegressionTests.cpp
cat > tests/svg/SvgRegressionTests.cpp << 'CPP_END'
#include <openhouse/document/Document.hpp>
#include <openhouse/dxf/DxfReader.hpp>
#include <openhouse/render/SvgExporter.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>

namespace openhouse::tests {

static std::string ReadFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return {};
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

static std::string NormalizeSvg(const std::string& content) {
    // Placeholder: future normalization (timestamps, comments, etc.)
    return content;
}

class SvgRegressionTest : public ::testing::Test {
protected:
    void RunTest(const std::string& dxfFileName, const std::string& svgFileName) {
        const std::string dxfPath = "tests/data/dxf/" + dxfFileName;
        const std::string expectedPath = "tests/expected/svg/" + svgFileName;

        auto result = dxf::ParseDxfFile(dxfPath);
        ASSERT_TRUE(result.has_value()) << "Failed to parse DXF: " << result.error();
        const auto& doc = result.value();

        std::stringstream svgOutput;
        bool exportSuccess = render::ExportSvg(svgOutput, doc);
        ASSERT_TRUE(exportSuccess) << "Failed to export SVG";

        const std::string expected = ReadFile(expectedPath);
        ASSERT_FALSE(expected.empty()) << "Golden SVG not found: " << expectedPath;

        const std::string actual = NormalizeSvg(svgOutput.str());
        const std::string normalizedExpected = NormalizeSvg(expected);
        EXPECT_EQ(actual, normalizedExpected) << "SVG mismatch for: " + dxfFileName;
    }
};

TEST_F(SvgRegressionTest, Line) { RunTest("line.dxf", "line.svg"); }
TEST_F(SvgRegressionTest, Circle) { RunTest("circle.dxf", "circle.svg"); }
TEST_F(SvgRegressionTest, Arc) { RunTest("arc.dxf", "arc.svg"); }
TEST_F(SvgRegressionTest, LwpolylineOpen) { RunTest("lwpolyline_open.dxf", "lwpolyline_open.svg"); }
TEST_F(SvgRegressionTest, LwpolylineClosed) { RunTest("lwpolyline_closed.dxf", "lwpolyline_closed.svg"); }
TEST_F(SvgRegressionTest, LwpolylineBulge) { RunTest("lwpolyline_bulge.dxf", "lwpolyline_bulge.svg"); }
TEST_F(SvgRegressionTest, MultipleEntities) { RunTest("multiple_entities.dxf", "multiple_entities.svg"); }
TEST_F(SvgRegressionTest, UnsupportedEntities) { RunTest("unsupported_entities.dxf", "unsupported_entities.svg"); }
TEST_F(SvgRegressionTest, LayerTest) { RunTest("layer_test.dxf", "layer_test.svg"); }
TEST_F(SvgRegressionTest, HouseFloor01) { RunTest("house_floor_01.dxf", "house_floor_01.svg"); }
TEST_F(SvgRegressionTest, RoomSimple) { RunTest("room_simple.dxf", "room_simple.svg"); }
TEST_F(SvgRegressionTest, RoomWithArcs) { RunTest("room_with_arcs.dxf", "room_with_arcs.svg"); }

} // namespace openhouse::tests
CPP_END

# 4.2 SvgGoldenTests.cpp
cat > tests/svg/SvgGoldenTests.cpp << 'CPP_END'
#include <openhouse/document/Document.hpp>
#include <openhouse/dxf/DxfReader.hpp>
#include <openhouse/render/SvgExporter.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>

namespace openhouse::tests {

class SvgGoldenTest : public ::testing::Test {
protected:
    void RunTest(const std::string& dxfFileName) {
        const std::string dxfPath = "tests/data/dxf/" + dxfFileName;

        auto result = dxf::ParseDxfFile(dxfPath);
        ASSERT_TRUE(result.has_value()) << "Failed to parse DXF: " << result.error();
        const auto& doc = result.value();

        std::stringstream svgOutput;
        bool exportSuccess = render::ExportSvg(svgOutput, doc);
        ASSERT_TRUE(exportSuccess) << "Failed to export SVG";

        const std::string svg = svgOutput.str();
        EXPECT_TRUE(svg.find("<svg") != std::string::npos) << "Missing <svg> tag";
        EXPECT_TRUE(svg.find("</svg>") != std::string::npos) << "Missing </svg> tag";
    }
};

TEST_F(SvgGoldenTest, Line) { RunTest("line.dxf"); }
TEST_F(SvgGoldenTest, Circle) { RunTest("circle.dxf"); }
TEST_F(SvgGoldenTest, Arc) { RunTest("arc.dxf"); }
TEST_F(SvgGoldenTest, LwpolylineBulge) { RunTest("lwpolyline_bulge.dxf"); }
TEST_F(SvgGoldenTest, HouseFloor01) { RunTest("house_floor_01.dxf"); }
TEST_F(SvgGoldenTest, RoomWithArcs) { RunTest("room_with_arcs.dxf"); }

} // namespace openhouse::tests
CPP_END

echo "✅ SVG test sources created."
echo ""

# =============================================================================
# 5. DS-005: Golden SVG Files
# =============================================================================

echo "📄 Generating golden SVG files (DS-005)..."

# 5.1 line.svg
cat > tests/expected/svg/line.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-1 -1 7 2">
  <line x1="0" y1="0" x2="5" y2="0" stroke="black" stroke-width="1"/>
</svg>
SVG_END

# 5.2 circle.svg
cat > tests/expected/svg/circle.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-3 -3 6 6">
  <circle cx="0" cy="0" r="2" stroke="black" stroke-width="1" fill="none"/>
</svg>
SVG_END

# 5.3 arc.svg
cat > tests/expected/svg/arc.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-3 -3 6 6">
  <path d="M 2 0 A 2 2 0 0 1 0 2" stroke="black" stroke-width="1" fill="none"/>
</svg>
SVG_END

# 5.4 lwpolyline_open.svg
cat > tests/expected/svg/lwpolyline_open.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-1 -1 5 5">
  <polyline points="0,0 3,0 3,3 0,3" stroke="black" stroke-width="1" fill="none"/>
</svg>
SVG_END

# 5.5 lwpolyline_closed.svg
cat > tests/expected/svg/lwpolyline_closed.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-1 -1 5 5">
  <polygon points="0,0 3,0 3,3 0,3" stroke="black" stroke-width="1" fill="none"/>
</svg>
SVG_END

# 5.6 lwpolyline_bulge.svg
cat > tests/expected/svg/lwpolyline_bulge.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-1 -1 5 5">
  <path d="M 0 0 L 3 0 L 3 3 A 3 3 0 0 1 0 3 Z" stroke="black" stroke-width="1" fill="none"/>
</svg>
SVG_END

# 5.7 multiple_entities.svg
cat > tests/expected/svg/multiple_entities.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-2 -2 8 6">
  <line x1="0" y1="0" x2="5" y2="0" stroke="black" stroke-width="1"/>
  <circle cx="2" cy="2" r="1" stroke="black" stroke-width="1" fill="none"/>
  <path d="M 4 4 A 2 2 0 0 1 4 2" stroke="black" stroke-width="1" fill="none"/>
  <polyline points="0,0 3,0 3,3" stroke="black" stroke-width="1" fill="none"/>
</svg>
SVG_END

# 5.8 unsupported_entities.svg
cat > tests/expected/svg/unsupported_entities.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-1 -1 7 2">
  <line x1="0" y1="0" x2="5" y2="0" stroke="black" stroke-width="1"/>
</svg>
SVG_END

# 5.9 layer_test.svg
cat > tests/expected/svg/layer_test.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-1 -1 7 5">
  <line x1="0" y1="0" x2="5" y2="0" stroke="black" stroke-width="1"/>
  <circle cx="2" cy="2" r="1" stroke="black" stroke-width="1" fill="none"/>
  <path d="M 4 4 A 2 2 0 0 1 4 2" stroke="black" stroke-width="1" fill="none"/>
  <polyline points="0,0 3,0 3,3" stroke="black" stroke-width="1" fill="none"/>
</svg>
SVG_END

# 5.10 house_floor_01.svg
cat > tests/expected/svg/house_floor_01.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-1 -1 12 8">
  <path d="M 0 0 L 10 0 A 1 1 0 0 1 11 1 L 7 4 L 0 4 Z" stroke="black" stroke-width="1" fill="none"/>
  <polyline points="3,0 3.5,0 3.5,1 3,1" stroke="black" stroke-width="1" fill="none"/>
  <circle cx="5" cy="3" r="0.5" stroke="black" stroke-width="1" fill="none"/>
  <rect x="1" y="4" width="1" height="0.5" stroke="black" stroke-width="1" fill="none"/>
  <path d="M 8 2 A 0.8 0.8 0 0 1 8 2.8" stroke="black" stroke-width="1" fill="none"/>
  <line x1="8" y1="2" x2="8" y2="2.8" stroke="black" stroke-width="1"/>
  <line x1="8" y1="2.8" x2="8.8" y2="2.8" stroke="black" stroke-width="1"/>
  <circle cx="6" cy="2" r="0.6" stroke="black" stroke-width="1" fill="none"/>
</svg>
SVG_END

# 5.11 room_simple.svg
cat > tests/expected/svg/room_simple.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-1 -1 12 10">
  <polygon points="0,0 10,0 10,8 0,8" stroke="black" stroke-width="1" fill="none"/>
</svg>
SVG_END

# 5.12 room_with_arcs.svg
cat > tests/expected/svg/room_with_arcs.svg << 'SVG_END'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-1 -1 12 10">
  <path d="M 0 0 L 10 0 A 2 2 0 0 1 12 2 L 10 8 A 2 2 0 0 1 8 10 L 0 8 Z" stroke="black" stroke-width="1" fill="none"/>
</svg>
SVG_END

echo "✅ Golden SVG files created (12 files)."
echo ""

# =============================================================================
# 6. DS-005: Documentation (SVG Guide) - ĐÃ SỬA: ĐÓNG HEREDOC ĐÚNG
# =============================================================================

echo "📄 Generating SVG test documentation (DS-005)..."

cat > docs/svg/SVG_TEST_GUIDE.md << 'GUIDE_END'
# SVG Test Guide

This document describes how to use the SVG test suite for OpenHouseCAD.

---

## Overview

The SVG test suite verifies that the SVG exporter produces correct output for various DXF test files. The workflow is:

1. Parse a DXF file from `tests/data/dxf/` with `dxf::ParseDxfFile`.
2. Export the resulting document to SVG with `render::ExportSvg`.
3. Compare the output against a golden file in `tests/expected/svg/` (regression tests), or perform a structural sanity check (golden tests).

---

## Directory Structure

```
tests/
├── data/dxf/              # Input DXF fixtures (see docs/dxf/TEST_CASES.md)
│   └── malformed/         # Edge-case / invalid DXF files
├── expected/svg/          # Golden SVG output, one per DXF fixture
└── svg/
    ├── SvgRegressionTests.cpp   # Byte-for-byte comparison against golden files
    └── SvgGoldenTests.cpp       # Structural sanity checks (valid <svg>...</svg>)
```

---

## Test Suites

### SvgRegressionTests.cpp

Each `TEST_F(SvgRegressionTest, ...)` case parses a DXF fixture, exports it to SVG, and does an exact string comparison against the matching file in `tests/expected/svg/`. Use this suite to catch any unintended change in exporter output.

Covers: `line`, `circle`, `arc`, `lwpolyline_open`, `lwpolyline_closed`, `lwpolyline_bulge`, `multiple_entities`, `unsupported_entities`, `layer_test`, `house_floor_01`, `room_simple`, `room_with_arcs`.

### SvgGoldenTests.cpp

A lighter-weight suite that only checks the export succeeds and produces a well-formed `<svg>...</svg>` document. Use this for fixtures where exact byte-for-byte output isn't required, or as a fast smoke test.

---

## Running the Tests

```bash
# From the build directory
ctest -R "SvgRegressionTest|SvgGoldenTest" --output-on-failure
```

or, if invoking the test binary directly:

```bash
./svg_tests --gtest_filter="SvgRegressionTest.*:SvgGoldenTest.*"
```

---

## Updating Golden Files

If an exporter change intentionally alters SVG output:

1. Run the regression suite and confirm the diff is expected.
2. Regenerate the affected file(s) in `tests/expected/svg/` from the new exporter output.
3. Re-run the suite to confirm it passes.
4. Note the change and its reason in the PR description — golden file updates should never be silent.

---

## Adding a New Test Case

1. Add the DXF fixture to `tests/data/dxf/` (or `malformed/` for edge cases) and document it in `docs/dxf/TEST_CASES.md`.
2. Add the matching golden SVG to `tests/expected/svg/`.
3. Add a `TEST_F` case to `SvgRegressionTests.cpp` (and optionally `SvgGoldenTests.cpp`).
4. Run the suite locally before opening a PR.

---

## Notes

- `NormalizeSvg()` in `SvgRegressionTests.cpp` is currently a passthrough; extend it if the exporter starts emitting non-deterministic output (timestamps, generated IDs, etc.) that shouldn't fail a regression test.
- Golden files are hand-written to match the current exporter's expected geometry, not captured from a live run — treat any mismatch as something to investigate, not something to auto-accept.
GUIDE_END

echo "✅ SVG test guide created: docs/svg/SVG_TEST_GUIDE.md"
echo ""

# =============================================================================
# 7. ENGINEERING_PRINCIPLES.md (cập nhật)
# =============================================================================

echo "📄 Updating ENGINEERING_PRINCIPLES.md..."

cat > docs/ENGINEERING_PRINCIPLES.md << 'PRINCIPLES_END'
# OpenHouseCAD — Engineering Principles

This document records the working principles for OpenHouseCAD engineering, updated with the DS-004 / DS-005 sprint work.

---

## Testing Philosophy

- **Fixtures over mocks for file-format code.** DXF parsing and SVG export are tested against real, hand-crafted DXF files (`tests/data/dxf/`) rather than mocked structures, so tests exercise the actual parsing/exporting code paths.
- **Malformed input is a first-class test case.** Every parser must have explicit coverage for missing fields, unknown entity types, and incomplete geometry (`tests/data/dxf/malformed/`). The expected behavior is always: skip the invalid entity/section and continue, never crash.
- **Two tiers of SVG testing:**
  - *Golden/regression tests* (`SvgRegressionTests.cpp`) catch any change in exact output — use for exporter code that must stay byte-stable.
  - *Structural/smoke tests* (`SvgGoldenTests.cpp`) only check the output is well-formed — use where exact output isn't the contract, just validity.
- **Golden files are updated deliberately, never silently.** Any diff in golden SVG output must be reviewed and explained in the PR, not auto-regenerated and merged.

## DXF Handling

- Unsupported entity types (`TEXT`, `ELLIPSE`, `SPLINE`, `IMAGE`, `HATCH`, unknown types) must be skipped without aborting the parse of the rest of the file.
- Entities missing required fields (e.g., no layer/group code 8) are skipped, not defaulted silently.
- Layers are preserved end-to-end from DXF import through to SVG export (see `layer_test.dxf` / `house_floor_01.dxf`).

## Documentation

- Every test fixture directory (`tests/data/dxf/`, `tests/expected/svg/`) has a companion Markdown doc explaining what each file is for and what result it should produce (`docs/dxf/TEST_CASES.md`, `docs/svg/SVG_TEST_GUIDE.md`). A fixture without documentation is considered incomplete.
- Sprint deliverables that generate files (like this sprint's package generator script) should be idempotent and safe to re-run.
PRINCIPLES_END

echo "✅ ENGINEERING_PRINCIPLES.md updated."
echo ""

# =============================================================================
# 8. Handover Document
# =============================================================================

echo "📄 Generating HANDOVER.md..."

cat > docs/HANDOVER.md << 'HANDOVER_END'
# Sprint Handover — DS-004 / DS-005

## Scope Delivered

- **DS-004 — DXF Test Assets**: 16 DXF fixture files under `tests/data/dxf/` (13 valid cases + 3 malformed/edge cases), documented in `docs/dxf/TEST_CASES.md`.
- **DS-005 — SVG Regression Test Suite**: `tests/svg/SvgRegressionTests.cpp` (12 exact-match regression tests) and `tests/svg/SvgGoldenTests.cpp` (6 structural smoke tests), backed by 12 golden SVG files in `tests/expected/svg/`, documented in `docs/svg/SVG_TEST_GUIDE.md`.
- **Documentation**: `docs/ENGINEERING_PRINCIPLES.md` updated with testing philosophy and DXF handling rules established this sprint.

## How to Verify Locally

```bash
bash deepseek_bash_20260804_4cfb8f.sh   # generates the full tree below the repo root
ctest -R "SvgRegressionTest|SvgGoldenTest" --output-on-failure
```

## File Map

| Path | Contents |
|---|---|
| `tests/data/dxf/*.dxf` | 13 valid DXF fixtures |
| `tests/data/dxf/malformed/*.dxf` | 3 malformed/edge-case DXF fixtures |
| `docs/dxf/TEST_CASES.md` | Table of every DXF fixture, purpose, expected result |
| `tests/svg/SvgRegressionTests.cpp` | 12 exact-match SVG regression tests |
| `tests/svg/SvgGoldenTests.cpp` | 6 structural SVG smoke tests |
| `tests/expected/svg/*.svg` | 12 golden SVG files |
| `docs/svg/SVG_TEST_GUIDE.md` | How to run/extend the SVG test suite |
| `docs/ENGINEERING_PRINCIPLES.md` | Updated testing philosophy |

## Known Limitations / Follow-ups

- `NormalizeSvg()` is currently a passthrough — no handling yet for non-deterministic exporter output, should the exporter later add timestamps or generated IDs.
- Golden SVG files were hand-written against the intended exporter behavior; they have not been cross-checked against a live exporter run in this sprint and should be verified against the actual `render::ExportSvg` implementation before being treated as ground truth.
- No CMake/build-system wiring is included in this script — the new `.cpp` test files need to be added to the relevant `CMakeLists.txt` test target for `ctest` to pick them up.

## Owner Handoff

Next engineer should start by wiring the two new `.cpp` files into the build, running the suite once against the real exporter, and reconciling any mismatches between golden files and actual output before merging.
HANDOVER_END

echo "✅ HANDOVER.md created."
echo ""

# =============================================================================
# 9. Review Checklist
# =============================================================================

echo "📄 Generating REVIEW_CHECKLIST.md..."

cat > docs/REVIEW_CHECKLIST.md << 'CHECKLIST_END'
# Review Checklist — DS-004 / DS-005

## DXF Fixtures (DS-004)

- [ ] Every fixture file listed in `docs/dxf/TEST_CASES.md` matches an actual file under `tests/data/dxf/`.
- [ ] Each malformed fixture triggers "skip and continue" behavior, not a crash or thrown exception.
- [ ] Coordinate/geometry values in each fixture are sane (no negative radii, no degenerate zero-length lines used as "valid" cases).
- [ ] Layer names used (`WALL`, `COLUMN`, `DOOR`, `FLOOR`, `WINDOW`, `FURNITURE`) are consistent across fixtures.

## SVG Regression Suite (DS-005)

- [ ] `SvgRegressionTests.cpp` and `SvgGoldenTests.cpp` compile against the current `openhouse` headers (`Document.hpp`, `DxfReader.hpp`, `SvgExporter.hpp`).
- [ ] Every DXF fixture referenced by a `TEST_F` case actually exists in `tests/data/dxf/`.
- [ ] Every golden SVG referenced exists in `tests/expected/svg/` and is valid, well-formed SVG.
- [ ] New/added `.cpp` test files are registered in the build system (`CMakeLists.txt` or equivalent) so `ctest` discovers them.
- [ ] Golden SVG files were generated from (or reconciled against) actual exporter output — not just hand-written to "look right."

## Documentation

- [ ] `docs/dxf/TEST_CASES.md` and `docs/svg/SVG_TEST_GUIDE.md` are up to date with the fixtures/tests actually present.
- [ ] `docs/ENGINEERING_PRINCIPLES.md` changes reflect real decisions made this sprint, not aspirational statements.
- [ ] `docs/HANDOVER.md` known-limitations section is still accurate at merge time.

## Script Hygiene

- [ ] `bash -n <script>` passes with no warnings (no unclosed heredocs).
- [ ] Running the script twice on a clean checkout produces identical output both times (idempotent).
- [ ] `set -e` behavior verified — a failure partway through does not leave a silently half-generated tree.
CHECKLIST_END

echo "✅ REVIEW_CHECKLIST.md created."
echo ""

# =============================================================================
# 10. README
# =============================================================================

echo "📄 Generating README.md..."

cat > README.md << 'README_END'
# OpenHouseCAD — DS-004 / DS-005 Sprint Package

This package contains DXF test assets, an SVG regression test suite, and supporting documentation generated for the DS-004 and DS-005 sprint tickets.

## What's Included

- `tests/data/dxf/` — 16 DXF fixture files (13 valid + 3 malformed), see `docs/dxf/TEST_CASES.md`.
- `tests/svg/` — SVG regression and golden test suites (`SvgRegressionTests.cpp`, `SvgGoldenTests.cpp`).
- `tests/expected/svg/` — 12 golden SVG output files.
- `docs/dxf/TEST_CASES.md` — DXF fixture reference.
- `docs/svg/SVG_TEST_GUIDE.md` — how to run and extend the SVG test suite.
- `docs/ENGINEERING_PRINCIPLES.md` — updated testing philosophy.
- `docs/HANDOVER.md` — sprint handover notes.
- `docs/REVIEW_CHECKLIST.md` — checklist for reviewing this package.

## Quick Start

```bash
bash deepseek_bash_20260804_4cfb8f.sh
```

This generates the full `tests/` and `docs/` tree shown above under the current directory. Re-running the script is safe — it overwrites the same files with the same content.

## Next Steps

See `docs/HANDOVER.md` → *Known Limitations / Follow-ups* for what still needs doing before this suite is considered fully wired into CI (build-system registration, reconciling golden files against the real exporter).
README_END

echo "✅ README.md created."
echo ""

echo "========================================================================"
echo "  DONE. Sprint package generated successfully."
echo "========================================================================"
