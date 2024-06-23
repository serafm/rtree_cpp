//
// Created by serafm on 7/6/2024.
//

#ifndef INTVECTOR_H
#define INTVECTOR_H
#include <vector>

#include "Procedure.h"

namespace Collections {
    class IntVector {
    private:
        int DEFAULT_CAPACITY = 10;
        std::vector<int> data;
        int position;

    public:
        IntVector();
        explicit IntVector(int capacity);
        bool add(int value);
        int removeAt(int index);
        int get(int index);
        int size();
        int set(int offset, int value);
        void clear();
        void clear(int capacity);
        void reset();
        bool forEach(Procedure procedure) const;
    };
}

#endif //INTVECTOR_H
