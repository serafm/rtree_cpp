cmake_minimum_required(VERSION 3.28)
project(rtree_cpp_v2)

set(CMAKE_CXX_STANDARD 17)

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
        src/structures/PriorityQueue.cpp
        src/structures/PriorityQueue.h
        src/structures/TIntArrayList.cpp
        src/structures/TIntArrayList.h
        src/structures/TFloatArrayList.cpp
        src/structures/TFloatArrayList.h)
