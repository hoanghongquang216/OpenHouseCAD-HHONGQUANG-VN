cmake_minimum_required(VERSION 3.25)

add_library(OpenHouseTesting INTERFACE)

add_library(OpenHouse::Testing ALIAS OpenHouseTesting)

target_include_directories(OpenHouseTesting
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(OpenHouseTesting INTERFACE cxx_std_23)
