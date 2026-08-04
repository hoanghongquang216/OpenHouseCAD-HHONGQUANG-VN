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
