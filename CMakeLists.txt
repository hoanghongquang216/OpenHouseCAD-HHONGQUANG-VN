cmake_minimum_required(VERSION 3.25)

project(OpenHouseCAD
    VERSION 0.1.0
    DESCRIPTION "OpenHouseCAD - Ung dung Thiet ke Cong trinh Xay dung"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(OPENHOUSE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(OpenHouseWarnings)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(CTest)
endif()

add_subdirectory(modules/foundation)
add_subdirectory(modules/geometry)
