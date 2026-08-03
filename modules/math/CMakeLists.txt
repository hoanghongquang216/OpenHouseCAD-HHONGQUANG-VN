cmake_minimum_required(VERSION 3.25)

add_library(OpenHouseMath INTERFACE)

add_library(OpenHouse::Math ALIAS OpenHouseMath)

target_include_directories(OpenHouseMath
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(OpenHouseMath
    INTERFACE
        OpenHouse::Foundation
        OpenHouse::Geometry
)

target_compile_features(OpenHouseMath INTERFACE cxx_std_23)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
