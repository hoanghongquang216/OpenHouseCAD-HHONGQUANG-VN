# Qt6 Integration Checklist

Status: **not started**. This document exists so that when Qt6 is
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

## 3. APP-001: first real target (not yet written)

Once step 2 is confirmed working, the first actual implementation task
is a minimal `QMainWindow` that opens an empty window and runs the Qt
event loop -- the smallest possible thing that proves the toolchain
(Qt6 + CMake's `qt_add_executable`/AUTOMOC + the actual compiler) works
end to end. This should be written and iterated on with real
`cmake --build` feedback available, not authored blind and handed over
untested.

Rough shape (**illustrative only, not final, not verified**):
- `modules/app/CMakeLists.txt` gains a `qt_add_executable(OpenHouseApp ...)`
  target once Qt6 is confirmed found.
- `modules/app/src/main.cpp` with a `QApplication` + empty `QMainWindow`.
- Link against `Qt6::Widgets` and (once relevant) `OpenHouse::Render` /
  `OpenHouse::Geometry` for the eventual viewport.
- `target_compile_features(... cxx_std_23)`, consistent with every other
  target in this project.

## 4. CI

`.github/workflows/ci.yml` currently builds without Qt6 and does not set
`OPENHOUSE_BUILD_APP=ON`, so the app module is skipped in CI as it is
locally by default. Once APP-001 exists and builds locally, CI will need
a Qt6 installation step (e.g. via `jurplel/install-qt-action` or the
platform's package manager) before the app target can be exercised in
CI. This is explicitly a follow-up task, not bundled into APP-000/001.

## 5. Licensing verification

Before any distribution/packaging work, re-confirm the LGPLv3 dynamic-
linking assumption in ADR-0003's Consequences section still holds for
however OpenHouseCAD ends up being built and distributed (installer,
static binary, etc.).
