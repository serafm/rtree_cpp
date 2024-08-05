#pragma once

#ifndef FLOATVECTOR_H
#define FLOATVECTOR_H
#include <vector>

namespace Collections {
    class FloatVector {
    private:
        int DEFAULT_CAPACITY = 10;
        std::vector<float> data;
        int position;
        float no_entry_value;

    public:
        FloatVector();
        explicit FloatVector(int capacity);
        bool add(float value);
        float removeAt(int offset);
        float get(int offset);
        int size() const;
        float set(int offset, float value);
        void clear();
        void clear(int capacity);
        void reset();
    };
}

#endif //FLOATVECTOR_H
