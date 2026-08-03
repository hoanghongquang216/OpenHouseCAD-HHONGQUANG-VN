cmake_minimum_required(VERSION 3.25)

add_library(OpenHouseDocument INTERFACE)

add_library(OpenHouse::Document ALIAS OpenHouseDocument)

target_include_directories(OpenHouseDocument
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(OpenHouseDocument
    INTERFACE
        OpenHouse::Foundation
        OpenHouse::Geometry
)

target_compile_features(OpenHouseDocument INTERFACE cxx_std_23)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
