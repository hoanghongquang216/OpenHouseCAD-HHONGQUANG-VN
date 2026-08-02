cmake_minimum_required(VERSION 3.20)

project(OpenHouseCAD LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(OPENHOUSECAD_BUILD_TESTS "Build OpenHouseCAD tests" ON)

add_subdirectory(modules/kernel)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/modules/model)
    add_subdirectory(modules/model)
endif()

if(OPENHOUSECAD_BUILD_TESTS)
    enable_testing()
endif()
