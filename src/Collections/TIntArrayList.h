//
// Created by serafm on 7/6/2024.
//

#ifndef TINTARRAYLIST_H
#define TINTARRAYLIST_H
#include <vector>

#include "Procedure.h"

namespace Collections {
    class TIntArrayList {
    private:
        int DEFAULT_CAPACITY = 10;
        std::vector<int> data;
        int position;

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
        bool forEach(Procedure procedure) const;
    };
}

#endif //TINTARRAYLIST_H
