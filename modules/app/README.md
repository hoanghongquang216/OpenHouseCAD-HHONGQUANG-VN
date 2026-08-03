# modules/app

**Status: scaffold only. No implementation yet.**

This will become the Qt6-based application layer per
`docs/ARCHITECTURE_DECISION_RECORDS/ADR-0003-windowing-gui-stack.md`.

As of APP-000, `CMakeLists.txt` here does nothing but:
1. Skip entirely unless configured with `-DOPENHOUSE_BUILD_APP=ON`.
2. When enabled, verify Qt6 can be found and fail with a clear message if
   not.

No Qt-dependent C++ source files exist yet. Per explicit project
decision, none will be added until Qt6 build verification has been
confirmed in a real development environment (see
`docs/QT_INTEGRATION_CHECKLIST.md`) -- not authored speculatively and
handed over unbuilt.

This module will depend on `OpenHouse::Foundation`, `OpenHouse::Geometry`,
`OpenHouse::Math`, and `OpenHouse::Render`. The reverse must never be
true: no kernel-layer module may depend on this one or on Qt.
