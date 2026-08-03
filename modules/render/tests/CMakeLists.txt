add_executable(OpenHouseSvgDocumentTests
    SvgDocumentTests.cpp
)

target_link_libraries(OpenHouseSvgDocumentTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Render
)

openhouse_enable_warnings(OpenHouseSvgDocumentTests)

add_test(
    NAME OpenHouseSvgDocumentTests
    COMMAND OpenHouseSvgDocumentTests
)

add_executable(OpenHouseRenderDocumentTests
    RenderDocumentTests.cpp
)

target_link_libraries(OpenHouseRenderDocumentTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Render
)

openhouse_enable_warnings(OpenHouseRenderDocumentTests)

add_test(
    NAME OpenHouseRenderDocumentTests
    COMMAND OpenHouseRenderDocumentTests
)
