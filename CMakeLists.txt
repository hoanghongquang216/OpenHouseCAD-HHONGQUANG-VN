cmake_minimum_required(VERSION 3.20)

project(OpenHouseCAD LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(OPENHOUSECAD_BUILD_TESTS "Build kernel tests" ON)

add_subdirectory(modules/kernel)
