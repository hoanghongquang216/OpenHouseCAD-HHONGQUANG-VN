add_executable(OpenHouseFoundationTests
    FoundationTests.cpp
)

target_link_libraries(OpenHouseFoundationTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Foundation
)

openhouse_enable_warnings(OpenHouseFoundationTests)

add_test(
    NAME OpenHouseFoundationTests
    COMMAND OpenHouseFoundationTests
)
