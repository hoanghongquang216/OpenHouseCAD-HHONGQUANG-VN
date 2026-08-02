cmake_minimum_required(VERSION 3.20)

project(OpenHouseGeometry LANGUAGES CXX)

add_library(openhouse_geometry INTERFACE)

target_include_directories(openhouse_geometry
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(openhouse_geometry INTERFACE cxx_std_20)

option(OPENHOUSE_GEOMETRY_BUILD_TESTS "Build geometry tests" ON)

if(OPENHOUSE_GEOMETRY_BUILD_TESTS)
    enable_testing()

    add_executable(test_vector3 tests/Test_Vector3.cpp)
    target_link_libraries(test_vector3 PRIVATE openhouse_geometry)
    add_test(NAME Vector3Test COMMAND test_vector3)

    add_executable(test_point3 tests/Test_Point3.cpp)
    target_link_libraries(test_point3 PRIVATE openhouse_geometry)
    add_test(NAME Point3Test COMMAND test_point3)

    add_executable(test_matrix4 tests/Test_Matrix4.cpp)
    target_link_libraries(test_matrix4 PRIVATE openhouse_geometry)
    add_test(NAME Matrix4Test COMMAND test_matrix4)

    add_executable(test_transform tests/Test_Transform.cpp)
    target_link_libraries(test_transform PRIVATE openhouse_geometry)
    add_test(NAME TransformTest COMMAND test_transform)
endif()
