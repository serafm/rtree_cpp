cmake_minimum_required(VERSION 3.28)
project(rtree_cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(rtree_cpp
        src/rtree/structures/Node.cpp
        src/rtree/structures/Node.h
        src/rtree/structures/Rectangle.cpp
        src/rtree/structures/Rectangle.h
        src/rtree/structures/Point.cpp
        src/rtree/structures/Point.h
        src/rtree/builders/QueryBuilder.cpp
        src/rtree/builders/QueryBuilder.h
        src/rtree/builders/RTreeBulkLoad.cpp
        src/rtree/builders/RTreeBulkLoad.h
        src/Main.cpp
)