//
// Created by serafm on 7/6/2024.
//

#include "PriorityQueue.h"

namespace DataStructures {

    PriorityQueue::PriorityQueue(bool sortOrder, int initialCapacity) {
        this->sortOrder = sortOrder;
        values = TIntArrayList(initialCapacity);
        priorities = TFloatArrayList(initialCapacity);
    }

    bool PriorityQueue::sortsEarlierThan(float p1, float p2) {
        if (sortOrder == SORT_ORDER_ASCENDING) {
            return p1 < p2;
        }
        return p1 > p2;
    }

    void PriorityQueue::insert(int value, float priority) {
        values.push_back(value);
        // priorities.push_back(priority);

        promote((int)values.size()-1, value, priority);
    }

    void PriorityQueue::promote(int index, int value, float priority) {
        while (index > 0) {
            int parentIndex = (index -1) / 2;
            float parentPriority = priorities.get(parentIndex);

            if (sortsEarlierThan(parentPriority, priority)) {
                break;
            }

            values.re
        }
    }
}