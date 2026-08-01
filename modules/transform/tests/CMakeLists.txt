cmake_minimum_required(VERSION 3.21)

add_executable(OpenHouseTransformTests
    Test_Matrix4.cpp
)

target_link_libraries(OpenHouseTransformTests
PRIVATE
    OpenHouseTransform
)
