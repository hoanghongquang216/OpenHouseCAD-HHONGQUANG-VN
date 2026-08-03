cmake_minimum_required(VERSION 3.25)

add_library(OpenHouseDxf INTERFACE)

add_library(OpenHouse::Dxf ALIAS OpenHouseDxf)

target_include_directories(OpenHouseDxf
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(OpenHouseDxf
    INTERFACE
        OpenHouse::Foundation
        OpenHouse::Geometry
        OpenHouse::Math
        OpenHouse::Document
)

target_compile_features(OpenHouseDxf INTERFACE cxx_std_23)

# Both demo executables below take a single optional argument (the
# output .svg path, matching scripts/demo.sh's calling convention) and
# read this bundled sample file as their input, embedded at compile
# time -- see each demo's own comment on why an input-path *argument*
# would silently break under the automated demo sweep.
set(OPENHOUSE_SAMPLE_HOUSE_DXF "${CMAKE_CURRENT_SOURCE_DIR}/examples/sample_house.dxf")

add_executable(OpenHouseDxfImportDemo
    examples/dxf_import_demo.cpp
)

target_link_libraries(OpenHouseDxfImportDemo
    PRIVATE
        OpenHouse::Dxf
        OpenHouse::Render
)

target_compile_definitions(OpenHouseDxfImportDemo
    PRIVATE
        OPENHOUSE_SAMPLE_HOUSE_DXF="${OPENHOUSE_SAMPLE_HOUSE_DXF}"
)

openhouse_enable_warnings(OpenHouseDxfImportDemo)

add_executable(OpenHouseSelectionDemo
    examples/selection_demo.cpp
)

target_link_libraries(OpenHouseSelectionDemo
    PRIVATE
        OpenHouse::Dxf
        OpenHouse::Document
        OpenHouse::Render
)

target_compile_definitions(OpenHouseSelectionDemo
    PRIVATE
        OPENHOUSE_SAMPLE_HOUSE_DXF="${OPENHOUSE_SAMPLE_HOUSE_DXF}"
)

openhouse_enable_warnings(OpenHouseSelectionDemo)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
