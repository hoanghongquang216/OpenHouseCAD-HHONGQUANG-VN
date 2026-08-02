add_library(openhouse_model INTERFACE)

add_library(OpenHouseCAD::Model ALIAS openhouse_model)

set_target_properties(openhouse_model PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(openhouse_model
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(OPENHOUSECAD_BUILD_TESTS)
    enable_testing()
endif()
