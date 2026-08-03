cmake_minimum_required(VERSION 3.25)

project(OpenHouseCAD
    VERSION 0.1.0
    DESCRIPTION "OpenHouseCAD - Ung dung Thiet ke Cong trinh Xay dung"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# docs/CODING_STANDARD.md: "Treat warnings as errors in CI." Off by default
# for local development convenience; CI should configure with
# -DOPENHOUSE_WARNINGS_AS_ERRORS=ON.
option(OPENHOUSE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(OpenHouseWarnings)

# Only enable testing (and therefore each module's `tests/` subdirectory,
# which is guarded by `if(BUILD_TESTING)`) when this is the top-level
# project — avoids pulling in tests when OpenHouseCAD is consumed as a
# subdirectory/dependency of another project.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(CTest)
endif()

# Off by default: the app module (Qt6-based, per ADR-0003) is scaffold-
# only so far -- no Qt source code has been written or built yet. See
# docs/QT_INTEGRATION_CHECKLIST.md. Enabling this without Qt6 installed
# will fail with a clear error message (see modules/app/CMakeLists.txt),
# not a build of anything real yet.
option(OPENHOUSE_BUILD_APP "Build the Qt6-based application (requires Qt6; not yet implemented)" OFF)

add_subdirectory(modules/testing)
add_subdirectory(modules/foundation)
add_subdirectory(modules/geometry)
add_subdirectory(modules/math)
add_subdirectory(modules/document)
add_subdirectory(modules/render)
add_subdirectory(modules/dxf)
add_subdirectory(modules/app)
