add_library(openhouse_kernel INTERFACE)

target_include_directories(
    openhouse_kernel
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(OPENHOUSECAD_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
