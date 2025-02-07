cmake_minimum_required(VERSION 3.28)
project(rtree_cpp_v2)

set(CMAKE_CXX_STANDARD 17)

add_executable(rtree_cpp_v2
        src/rtree/core/Node.cpp
        src/rtree/core/Node.h
        src/rtree/core/Rectangle.cpp
        src/rtree/core/Rectangle.h
        src/rtree/core/Point.cpp
        src/rtree/core/Point.h
        src/rtree/query/QueryBuilder.cpp
        src/rtree/query/QueryBuilder.h
        src/rtree/RTreeBulkLoadBuilder.cpp
        src/rtree/RTreeBulkLoadBuilder.h
        src/main/CreateSpatialIndex.cpp
        src/main/CreateSpatialIndex.h
        src/main/Main.cpp
        src/rtree/RTreeBuilder.cpp
        src/rtree/RTreeBuilder.h)