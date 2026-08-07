cmake_minimum_required(VERSION 3.25)

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

message(STATUS "OpenHouseApp: Qt6 found (${Qt6_VERSION}) -- building "
                "APP-001 toolchain-verification target.")

qt_add_executable(OpenHouseApp
    src/main.cpp
)

target_link_libraries(OpenHouseApp
    PRIVATE
        Qt6::Widgets
)

target_compile_features(OpenHouseApp PRIVATE cxx_std_23)
