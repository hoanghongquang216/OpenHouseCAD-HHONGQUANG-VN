add_executable(OpenHouseDxfReaderTests
    DxfReaderTests.cpp
)

target_link_libraries(OpenHouseDxfReaderTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Dxf
)

openhouse_enable_warnings(OpenHouseDxfReaderTests)

add_test(
    NAME OpenHouseDxfReaderTests
    COMMAND OpenHouseDxfReaderTests
)
