add_executable(OpenHouseDocumentTests
    DocumentTests.cpp
)

target_link_libraries(OpenHouseDocumentTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Document
)

openhouse_enable_warnings(OpenHouseDocumentTests)

add_test(
    NAME OpenHouseDocumentTests
    COMMAND OpenHouseDocumentTests
)

add_executable(OpenHouseLayerTests
    LayerTests.cpp
)

target_link_libraries(OpenHouseLayerTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Document
)

openhouse_enable_warnings(OpenHouseLayerTests)

add_test(
    NAME OpenHouseLayerTests
    COMMAND OpenHouseLayerTests
)
