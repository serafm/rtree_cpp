//
// Created by serafm on 7/6/2024.
//

#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H
#include <vector>

#include "TFloatArrayList.h"
#include "TIntArrayList.h"

namespace Collections {
    class PriorityQueue {
    public:
        PriorityQueue(bool sortOrder);
        PriorityQueue(bool sortOrder, int initialCapacity);
        bool SORT_ORDER_ASCENDING = true;
        bool SORT_ORDER_DESCENDING = false;
        void insert(int value, float priority);
        int size();
        void clear();
        void reset();
        int getValue();
        float getPriority();
        int pop();
        void setSortOrder(bool sortOrder);

    private:
        TIntArrayList values;
        TFloatArrayList priorities;
        bool sortOrder = SORT_ORDER_ASCENDING;
        bool INTERNAL_CONSISTENCY_CHECKING = false;
        bool sortsEarlierThan(float p1, float p2) const;
        void promote(int index, int value, float priority);
        void demote(int index, int value, float priority);
        void check();

    };
}

#endif //PRIORITYQUEUE_H
