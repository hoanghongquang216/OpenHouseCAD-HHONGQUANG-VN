cmake_minimum_required(VERSION 3.25)

add_library(OpenHouseRender INTERFACE)

add_library(OpenHouse::Render ALIAS OpenHouseRender)

target_include_directories(OpenHouseRender
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(OpenHouseRender
    INTERFACE
        OpenHouse::Foundation
        OpenHouse::Geometry
        OpenHouse::Document
)

target_compile_features(OpenHouseRender INTERFACE cxx_std_23)

add_executable(OpenHouseSpiral1Demo
    examples/spiral1_demo.cpp
)

target_link_libraries(OpenHouseSpiral1Demo
    PRIVATE
        OpenHouse::Render
)

openhouse_enable_warnings(OpenHouseSpiral1Demo)

add_executable(OpenHouseSpiral2Demo
    examples/spiral2_demo.cpp
)

target_link_libraries(OpenHouseSpiral2Demo
    PRIVATE
        OpenHouse::Render
)

openhouse_enable_warnings(OpenHouseSpiral2Demo)

add_executable(OpenHousePrimitivesDemo
    examples/primitives_demo.cpp
)

target_link_libraries(OpenHousePrimitivesDemo
    PRIVATE
        OpenHouse::Render
)

openhouse_enable_warnings(OpenHousePrimitivesDemo)

add_executable(OpenHouseArcDemo
    examples/arc_demo.cpp
)

target_link_libraries(OpenHouseArcDemo
    PRIVATE
        OpenHouse::Render
)

openhouse_enable_warnings(OpenHouseArcDemo)

add_executable(OpenHouseTransformDemo
    examples/transform_demo.cpp
)

target_link_libraries(OpenHouseTransformDemo
    PRIVATE
        OpenHouse::Render
        OpenHouse::Math
)

openhouse_enable_warnings(OpenHouseTransformDemo)

add_executable(OpenHouseDocumentDemo
    examples/document_demo.cpp
)

target_link_libraries(OpenHouseDocumentDemo
    PRIVATE
        OpenHouse::Render
)

openhouse_enable_warnings(OpenHouseDocumentDemo)

add_executable(OpenHouseLayersDemo
    examples/layers_demo.cpp
)

target_link_libraries(OpenHouseLayersDemo
    PRIVATE
        OpenHouse::Render
)

openhouse_enable_warnings(OpenHouseLayersDemo)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
