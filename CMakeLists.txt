cmake_minimum_required(VERSION 3.28)
project(rtree_cpp_v2)

set(CMAKE_CXX_STANDARD 17)



add_executable(rtree_cpp_v2
        src/Node.h
        src/Rectangle.cpp
        src/Rectangle.h
        src/RTree.cpp
        src/RTree.h
        src/CreateSpatialIndex.cpp
        src/Node.cpp
        src/Point.cpp
        src/Point.h
        src/Main.cpp
        src/CreateSpatialIndex.h
        src/Test/RectangleTest.cpp
        src/Test/RTreeTest.cpp
        src/Test/NodeTest.cpp)

# Enable testing support in CMake
enable_testing()

# Add Google Test
find_package(GTest REQUIRED) # Make sure GTest is installed (via apt, vcpkg, or source)

# Add test executable
add_executable(rtree_tests
        src/Rectangle.cpp
        src/RTree.cpp
        src/Node.cpp
        src/Point.cpp
        src/Test/RectangleTest.cpp
        src/Test/RTreeTest.cpp
        src/Test/NodeTest.cpp)

# Link Google Test to the test executable
target_link_libraries(rtree_tests GTest::gtest GTest::gtest_main)

# Add a test target for CTest
add_test(NAME rtree_tests COMMAND rtree_tests)