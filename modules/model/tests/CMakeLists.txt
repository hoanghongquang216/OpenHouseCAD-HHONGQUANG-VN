add_executable(openhouse_model_tests
    Test_EntityIdentity.cpp
    Test_EntityRegistry.cpp
    Test_ModelStore.cpp
    Test_DocumentModel.cpp
)

target_link_libraries(openhouse_model_tests
    PRIVATE
        OpenHouseCAD::Model
)

add_test(
    NAME OpenHouseCAD.Model.Tests
    COMMAND openhouse_model_tests
)
