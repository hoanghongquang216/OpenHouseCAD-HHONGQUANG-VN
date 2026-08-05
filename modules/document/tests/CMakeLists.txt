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

add_executable(OpenHouseCommandTests
    CommandTests.cpp
)

target_link_libraries(OpenHouseCommandTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Document
)

openhouse_enable_warnings(OpenHouseCommandTests)

add_test(
    NAME OpenHouseCommandTests
    COMMAND OpenHouseCommandTests
)

add_executable(OpenHouseMacroCommandTests
    MacroCommandTests.cpp
)

target_link_libraries(OpenHouseMacroCommandTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Document
)

openhouse_enable_warnings(OpenHouseMacroCommandTests)

add_test(
    NAME OpenHouseMacroCommandTests
    COMMAND OpenHouseMacroCommandTests
)

add_executable(OpenHouseCommandHistoryTests
    CommandHistoryTests.cpp
)

target_link_libraries(OpenHouseCommandHistoryTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Document
)

openhouse_enable_warnings(OpenHouseCommandHistoryTests)

add_test(
    NAME OpenHouseCommandHistoryTests
    COMMAND OpenHouseCommandHistoryTests
)

add_executable(OpenHouseSnapTests
    SnapTests.cpp
)

target_link_libraries(OpenHouseSnapTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Document
)

openhouse_enable_warnings(OpenHouseSnapTests)

add_test(
    NAME OpenHouseSnapTests
    COMMAND OpenHouseSnapTests
)
add_executable(OpenHouseTrimTests
    TrimTests.cpp
)
target_link_libraries(OpenHouseTrimTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Document
)
openhouse_enable_warnings(OpenHouseTrimTests)
add_test(
    NAME OpenHouseTrimTests
    COMMAND OpenHouseTrimTests
)
add_executable(OpenHouseExtendTests
    ExtendTests.cpp
)
target_link_libraries(OpenHouseExtendTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Document
)
openhouse_enable_warnings(OpenHouseExtendTests)
add_test(
    NAME OpenHouseExtendTests
    COMMAND OpenHouseExtendTests
)
