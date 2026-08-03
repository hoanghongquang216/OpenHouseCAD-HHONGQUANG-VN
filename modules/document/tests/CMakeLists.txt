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

add_executable(OpenHouseSelectionTests
    SelectionTests.cpp
)

target_link_libraries(OpenHouseSelectionTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Document
)

openhouse_enable_warnings(OpenHouseSelectionTests)

add_test(
    NAME OpenHouseSelectionTests
    COMMAND OpenHouseSelectionTests
)

add_executable(OpenHouseDocumentHitTestTests
    DocumentHitTestTests.cpp
)

target_link_libraries(OpenHouseDocumentHitTestTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Document
)

openhouse_enable_warnings(OpenHouseDocumentHitTestTests)

add_test(
    NAME OpenHouseDocumentHitTestTests
    COMMAND OpenHouseDocumentHitTestTests
)

add_executable(OpenHouseDocumentTransformTests
    DocumentTransformTests.cpp
)

target_link_libraries(OpenHouseDocumentTransformTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Document
)

openhouse_enable_warnings(OpenHouseDocumentTransformTests)

add_test(
    NAME OpenHouseDocumentTransformTests
    COMMAND OpenHouseDocumentTransformTests
)
