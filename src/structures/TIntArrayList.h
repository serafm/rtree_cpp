//
// Created by serafm on 7/6/2024.
//

#ifndef TINTARRAYLIST_H
#define TINTARRAYLIST_H
#include <vector>

namespace DataStructures {
    class TIntArrayList {
    private:
        int DEFAULT_CAPACITY = 10;
        std::vector<int> data;

    public:
        TIntArrayList();
        explicit TIntArrayList(int capacity);
        bool add(int value);
        int removeAt(int index);
        int get(int index);
        int size();
        int set(int offset, int value);
        void clear(int capacity);
        void reset();
    };
}

#endif //TINTARRAYLIST_H
