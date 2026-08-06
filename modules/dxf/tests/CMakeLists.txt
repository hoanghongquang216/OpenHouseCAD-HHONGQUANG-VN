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
# =============================================================================
# Thêm đoạn này vào CUỐI file modules/dxf/tests/CMakeLists.txt hiện có
# (chỉ append -- giữ nguyên khối OpenHouseDxfReaderTests đã có phía trên).
#
# Link cả OpenHouse::Dxf và OpenHouse::Render, theo đúng tiền lệ đã thấy ở
# modules/dxf/examples/dxf_import_demo.cpp (executable duy nhất trong repo
# hiện đã link cả hai module này cùng lúc).
# =============================================================================

add_executable(OpenHouseSvgPipelineIntegrationTests
    SvgPipelineIntegrationTests.cpp
)

target_link_libraries(OpenHouseSvgPipelineIntegrationTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Dxf
        OpenHouse::Render
)

openhouse_enable_warnings(OpenHouseSvgPipelineIntegrationTests)

add_test(
    NAME OpenHouseSvgPipelineIntegrationTests
    COMMAND OpenHouseSvgPipelineIntegrationTests
)
add_executable(OpenHouseDxfWriterTests
    DxfWriterTests.cpp
)
target_link_libraries(OpenHouseDxfWriterTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Dxf
        OpenHouse::Document
)
openhouse_enable_warnings(OpenHouseDxfWriterTests)
add_test(
    NAME OpenHouseDxfWriterTests
    COMMAND OpenHouseDxfWriterTests
)
add_executable(OpenHouseDxfRoundTripTests
    DxfRoundTripTests.cpp
)
target_link_libraries(OpenHouseDxfRoundTripTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Dxf
        OpenHouse::Document
)
openhouse_enable_warnings(OpenHouseDxfRoundTripTests)
add_test(
    NAME OpenHouseDxfRoundTripTests
    COMMAND OpenHouseDxfRoundTripTests
)

add_executable(OpenHouseDxfWriterGoldenFileTests
    DxfWriterGoldenFileTests.cpp
)
target_link_libraries(OpenHouseDxfWriterGoldenFileTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Dxf
        OpenHouse::Document
)
target_compile_definitions(OpenHouseDxfWriterGoldenFileTests
    PRIVATE
        OPENHOUSE_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testdata"
)
openhouse_enable_warnings(OpenHouseDxfWriterGoldenFileTests)
add_test(
    NAME OpenHouseDxfWriterGoldenFileTests
    COMMAND OpenHouseDxfWriterGoldenFileTests
)
