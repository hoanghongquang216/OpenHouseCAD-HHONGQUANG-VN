cmake_minimum_required(VERSION 3.25)

add_library(OpenHouseFoundation INTERFACE)

add_library(OpenHouse::Foundation ALIAS OpenHouseFoundation)

target_include_directories(OpenHouseFoundation
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(OpenHouseFoundation INTERFACE cxx_std_23)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
