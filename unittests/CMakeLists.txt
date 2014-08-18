include_directories("${PROJECT_SOURCE_DIR}")

add_executable(unittest
    TestLex.cpp
    TestParser.cpp
    TestSemantic.cpp
    TestString.cpp
    TestTable.cpp
    UnitTest.cpp
    )
target_link_libraries(unittest
    luna
    )

add_executable(gctest
    GCTest.cpp
    )
target_link_libraries(gctest
    luna
    )
