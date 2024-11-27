cmake_minimum_required(VERSION 3.28)
project(rtree_cpp_v2)

set(CMAKE_CXX_STANDARD 17)

add_executable(rtree_cpp_v2
        src/rtree/Node.h
        src/rtree/Rectangle.cpp
        src/rtree/Rectangle.h
        src/QueryBuilder.cpp
        src/QueryBuilder.h
        src/main/CreateSpatialIndex.cpp
        src/rtree/Node.cpp
        src/rtree/Point.cpp
        src/rtree/Point.h
        src/main/Main.cpp
        src/main/CreateSpatialIndex.h
        src/rtree/RTreeBuilder.cpp
        src/rtree/RTreeBuilder.h)