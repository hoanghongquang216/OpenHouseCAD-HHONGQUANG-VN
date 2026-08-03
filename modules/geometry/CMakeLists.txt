cmake_minimum_required(VERSION 3.25)

add_library(OpenHouseGeometry INTERFACE)

add_library(OpenHouse::Geometry ALIAS OpenHouseGeometry)

target_include_directories(OpenHouseGeometry
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(OpenHouseGeometry
    INTERFACE
        OpenHouse::Foundation
)

target_compile_features(OpenHouseGeometry INTERFACE cxx_std_23)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
