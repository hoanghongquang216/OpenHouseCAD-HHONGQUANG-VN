cmake_minimum_required(VERSION 3.25)

# ============================================================================
# App module -- SCAFFOLD ONLY.
#
# Per ADR-0003 (docs/ARCHITECTURE_DECISION_RECORDS/ADR-0003-windowing-gui-stack.md),
# this module will eventually build a Qt6-based application. No Qt source
# code has been written yet -- per explicit project decision, no Qt code
# is merged until it has been built and verified in a real environment
# with Qt6 actually installed (the sandbox this scaffold was authored in
# does not have Qt6 available, so nothing here could have been verified
# beyond "the CMake logic is internally consistent").
#
# This file currently does nothing but:
#   1. Declare the OPENHOUSE_BUILD_APP option (default OFF), so the
#      existing zero-dependency kernel build (Foundation/Geometry/Math/
#      Render) is completely unaffected for anyone without Qt6 installed.
#   2. When explicitly enabled, attempt to locate Qt6 and fail with a
#      clear message if not found -- rather than a cryptic CMake error
#      deep in target configuration.
#
# See docs/QT_INTEGRATION_CHECKLIST.md for the steps to take when Qt6 is
# available and actual implementation (APP-001 onward) begins.
# ============================================================================

if(NOT OPENHOUSE_BUILD_APP)
    message(STATUS "OpenHouseApp: skipped (OPENHOUSE_BUILD_APP is OFF)")
    return()
endif()

find_package(Qt6 COMPONENTS Widgets QUIET)

if(NOT Qt6_FOUND)
    message(FATAL_ERROR
        "OPENHOUSE_BUILD_APP is ON but Qt6 (Widgets component) was not "
        "found. Install Qt6 and/or set CMAKE_PREFIX_PATH to your Qt6 "
        "installation, e.g.:\n"
        "  cmake -S . -B build -DOPENHOUSE_BUILD_APP=ON "
        "-DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64\n"
        "See docs/QT_INTEGRATION_CHECKLIST.md for details."
    )
endif()

message(STATUS "OpenHouseApp: Qt6 found (${Qt6_VERSION}) -- app module "
                "targets are not yet implemented (see APP-001, not started).")

# Intentionally no add_executable / add_library here yet. The first real
# target (a Qt6 QMainWindow that opens an empty window -- APP-001) will be
# added once Qt6 build verification is confirmed possible in the actual
# development environment, not this authoring sandbox.
