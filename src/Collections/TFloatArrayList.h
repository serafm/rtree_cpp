//
// Created by serafm on 8/6/2024.
//

#ifndef TFLOATARRAYLIST_H
#define TFLOATARRAYLIST_H
#include <vector>

namespace Collections {
    class TFloatArrayList {
    private:
        int DEFAULT_CAPACITY = 10;
        std::vector<float> data;

    public:
        TFloatArrayList();
        explicit TFloatArrayList(int capacity);
        bool add(float value);
        float removeAt(int offset);
        float get(int offset);
        int size();
        float set(int offset, float value);
        void clear(int capacity);
        void reset();
    };
}

#endif //TFLOATARRAYLIST_H
