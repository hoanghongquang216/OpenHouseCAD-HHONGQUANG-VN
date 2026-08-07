# Qt6 Integration Checklist

Status: **in progress**. This document exists so that when Qt6 is
available in a real development environment, integration is a mechanical
checklist rather than a from-scratch investigation. See
`docs/ARCHITECTURE_DECISION_RECORDS/ADR-0003-windowing-gui-stack.md` for
the decision this checklist implements.

Per explicit project decision, no Qt-dependent C++ source code should be
merged until each step below has been verified against a real Qt6
installation -- not assumed correct from documentation alone. Everything
in `modules/app/CMakeLists.txt` as of APP-000 is scaffold that fails
loudly (via `FATAL_ERROR`) if Qt6 isn't found; it does not build anything
yet.

## 1. Install Qt6

- Install Qt6 (Widgets component at minimum) via your platform's package
  manager, the Qt online installer, or a distribution's Qt6 packages.
- Confirm `find_package(Qt6 COMPONENTS Widgets)` can locate it -- either
  it's on the default CMake search path, or you'll need
  `-DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/<platform>` when configuring.

## 2. Verify the scaffold detects Qt6 correctly

```bash
cmake -S . -B build -DOPENHOUSE_BUILD_APP=ON
```

Expected: a `-- OpenHouseApp: Qt6 found (X.Y.Z) ...` status message, no
errors. If Qt6 isn't found, you should get the `FATAL_ERROR` message from
`modules/app/CMakeLists.txt` with guidance, not a cryptic CMake failure.

**Report this result back before writing any Qt C++ code** -- confirms
the detection logic itself is correct in a real environment before
building on top of it.

## 3. APP-001 — Toolchain Verification

Status: ✅ Done

**Evidence**

- Qt6 detected successfully during CMake configure.
- `OpenHouseApp` target builds successfully.
- Minimal Qt application (`QApplication` + `QMainWindow`) runs successfully on a real Qt6 installation.
- This milestone verifies the Qt6 toolchain and basic application bootstrap only.

**Observations (non-blocking)**

- Runtime warnings related to the graphics environment (e.g. WSL2 `libEGL` / Mesa / ZINK) were observed but did not prevent the application from starting or displaying the main window.
- These warnings are treated as environment-specific and are outside the scope of APP-001.

**Out of scope**

APP-001 does **not** validate:

- `QRhiWidget` integration.
- Rendering correctness or performance.
- CAD viewport functionality.
- CAD-scale performance characteristics.

These items remain future implementation work and require separate execution-generated evidence.

## 4. CI

Status: patch committed (`31cf537`), workflow run pending verification.

`.github/workflows/ci.yml` now installs Qt6 and sets
`OPENHOUSE_BUILD_APP=ON` on a single representative leg (gcc/Release),
rather than the full build matrix -- APP-001's scope is toolchain
verification, not compiler-compatibility verification. This should be
revisited once the app layer contains substantial Qt-dependent
implementation. Actual GitHub Actions execution result for this change
not yet confirmed.

## 5. Licensing verification

Before any distribution/packaging work, re-confirm the LGPLv3 dynamic-
linking assumption in ADR-0003's Consequences section still holds for
however OpenHouseCAD ends up being built and distributed (installer,
static binary, etc.).
