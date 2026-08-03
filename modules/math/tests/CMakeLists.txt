add_executable(OpenHouseMathTests
    MathTests.cpp
)

target_link_libraries(OpenHouseMathTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Math
)

openhouse_enable_warnings(OpenHouseMathTests)

add_test(
    NAME OpenHouseMathTests
    COMMAND OpenHouseMathTests
)

add_executable(OpenHouseNumericTraitsTests
    NumericTraitsTests.cpp
)

target_link_libraries(OpenHouseNumericTraitsTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Math
)

openhouse_enable_warnings(OpenHouseNumericTraitsTests)

add_test(
    NAME OpenHouseNumericTraitsTests
    COMMAND OpenHouseNumericTraitsTests
)

add_executable(OpenHouseToleranceTests
    ToleranceTests.cpp
)

target_link_libraries(OpenHouseToleranceTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Math
)

openhouse_enable_warnings(OpenHouseToleranceTests)

add_test(
    NAME OpenHouseToleranceTests
    COMMAND OpenHouseToleranceTests
)

add_executable(OpenHouseMatrix4Tests
    Matrix4Tests.cpp
)

target_link_libraries(OpenHouseMatrix4Tests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Math
)

openhouse_enable_warnings(OpenHouseMatrix4Tests)

add_test(
    NAME OpenHouseMatrix4Tests
    COMMAND OpenHouseMatrix4Tests
)

add_executable(OpenHouseMatrix3Tests
    Matrix3Tests.cpp
)

target_link_libraries(OpenHouseMatrix3Tests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Math
)

openhouse_enable_warnings(OpenHouseMatrix3Tests)

add_test(
    NAME OpenHouseMatrix3Tests
    COMMAND OpenHouseMatrix3Tests
)
