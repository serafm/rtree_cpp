cmake_minimum_required(VERSION 3.28)
project(rtree_cpp_v2)

set(CMAKE_CXX_STANDARD 17)

# Add Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(rtree_cpp_v2
        src/Node.h
        src/Rectangle.cpp
        src/Rectangle.h
        src/RTree.cpp
        src/RTree.h
        src/Main.cpp
        src/Node.cpp
        src/Point.cpp
        src/Point.h
        src/Collections/PriorityQueue.cpp
        src/Collections/PriorityQueue.h
        src/Collections/IntVector.cpp
        src/Collections/IntVector.h
        src/Collections/FloatVector.cpp
        src/Collections/FloatVector.h)