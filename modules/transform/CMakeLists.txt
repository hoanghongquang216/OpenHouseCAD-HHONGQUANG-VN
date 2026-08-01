cmake_minimum_required(VERSION 3.21)

add_library(OpenHouseTransform INTERFACE)

target_include_directories(OpenHouseTransform
INTERFACE
    include
)

target_compile_features(OpenHouseTransform
INTERFACE
    cxx_std_23
)
