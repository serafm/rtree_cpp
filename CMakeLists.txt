cmake_minimum_required(VERSION 3.28)
project(rtree_cpp_v2)

set(CMAKE_CXX_STANDARD 17)

add_executable(rtree_cpp_v2
        src/rtree/Node.cpp
        src/rtree/Node.h
        src/rtree/Rectangle.cpp
        src/rtree/Rectangle.h
        src/rtree/Point.cpp
        src/rtree/Point.h
        src/QueryBuilder.cpp
        src/QueryBuilder.h
        src/rtree/RTreeBuilder.cpp
        src/rtree/RTreeBuilder.h
        src/main/CreateSpatialIndex.cpp
        src/main/CreateSpatialIndex.h
        src/main/Main.cpp)