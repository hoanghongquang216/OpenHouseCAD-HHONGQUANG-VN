add_library(openhouse_kernel INTERFACE)

target_include_directories(
    openhouse_kernel
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(OPENHOUSECAD_BUILD_TESTS)
    enable_testing()

    add_executable(kernel_objectstore_test tests/Test_ObjectStore.cpp)
    target_link_libraries(kernel_objectstore_test PRIVATE openhouse_kernel)

    add_test(NAME kernel_objectstore_test COMMAND kernel_objectstore_test)
endif()
