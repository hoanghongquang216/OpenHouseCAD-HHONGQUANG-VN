add_executable(OpenHouseGeometryTests
    GeometryTests.cpp
)

target_link_libraries(OpenHouseGeometryTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Geometry
)

openhouse_enable_warnings(OpenHouseGeometryTests)

add_test(
    NAME OpenHouseGeometryTests
    COMMAND OpenHouseGeometryTests
)

add_executable(OpenHouseLine2Tests
    Line2Tests.cpp
)

target_link_libraries(OpenHouseLine2Tests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Geometry
)

openhouse_enable_warnings(OpenHouseLine2Tests)

add_test(
    NAME OpenHouseLine2Tests
    COMMAND OpenHouseLine2Tests
)

add_executable(OpenHouseCircle2Tests
    Circle2Tests.cpp
)

target_link_libraries(OpenHouseCircle2Tests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Geometry
)

openhouse_enable_warnings(OpenHouseCircle2Tests)

add_test(
    NAME OpenHouseCircle2Tests
    COMMAND OpenHouseCircle2Tests
)

add_executable(OpenHouseArc2Tests
    Arc2Tests.cpp
)

target_link_libraries(OpenHouseArc2Tests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Geometry
)

openhouse_enable_warnings(OpenHouseArc2Tests)

add_test(
    NAME OpenHouseArc2Tests
    COMMAND OpenHouseArc2Tests
)

add_executable(OpenHouseBoundingBox2Tests
    BoundingBox2Tests.cpp
)

target_link_libraries(OpenHouseBoundingBox2Tests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Geometry
)

openhouse_enable_warnings(OpenHouseBoundingBox2Tests)

add_test(
    NAME OpenHouseBoundingBox2Tests
    COMMAND OpenHouseBoundingBox2Tests
)

add_executable(OpenHouseBoundsTests
    BoundsTests.cpp
)

target_link_libraries(OpenHouseBoundsTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Geometry
)

openhouse_enable_warnings(OpenHouseBoundsTests)

add_test(
    NAME OpenHouseBoundsTests
    COMMAND OpenHouseBoundsTests
)

add_executable(OpenHouseHitTestTests
    HitTestTests.cpp
)

target_link_libraries(OpenHouseHitTestTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Geometry
)

openhouse_enable_warnings(OpenHouseHitTestTests)

add_test(
    NAME OpenHouseHitTestTests
    COMMAND OpenHouseHitTestTests
)

add_executable(OpenHouseTransformTests
    TransformTests.cpp
)

target_link_libraries(OpenHouseTransformTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Geometry
)

openhouse_enable_warnings(OpenHouseTransformTests)

add_test(
    NAME OpenHouseTransformTests
    COMMAND OpenHouseTransformTests
)

add_executable(OpenHouseIntersectionTests
    IntersectionTests.cpp
)

target_link_libraries(OpenHouseIntersectionTests
    PRIVATE
        OpenHouse::Testing
        OpenHouse::Geometry
)

openhouse_enable_warnings(OpenHouseIntersectionTests)

add_test(
    NAME OpenHouseIntersectionTests
    COMMAND OpenHouseIntersectionTests
)
