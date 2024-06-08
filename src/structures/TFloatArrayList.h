//
// Created by serafm on 8/6/2024.
//

#ifndef TFLOATARRAYLIST_H
#define TFLOATARRAYLIST_H
#include <vector>

namespace DataStructures {
    class TFloatArrayList {
    private:
        int DEFAULT_CAPACITY = 10;
        std::vector<float> data;

    public:
        TFloatArrayList();
        explicit TFloatArrayList(int capacity);
        bool add(int value);
        float removeAt(int index);
        float get(int index);
        int size();
        float set(int offset, int value);
        void clear(int capacity);
        void reset();
    };
} // DataStructures

#endif //TFLOATARRAYLIST_H
