//
// Created by serafm on 7/6/2024.
//

#include "PriorityQueue.h"

#include <iostream>
#include <ostream>

namespace Collections {

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
        values.add(value);
        priorities.add(priority);

        promote((int)values.size()-1, value, priority);
    }

    void PriorityQueue::promote(int index, int value, float priority) {
        while (index > 0) {
            int parentIndex = (index -1) / 2;
            float parentPriority = priorities.get(parentIndex);

            if (sortsEarlierThan(parentPriority, priority)) {
                break;
            }

            values.set(index, value);
            priorities.set(index, priority);

            if (INTERNAL_CONSISTENCY_CHECKING) {
                check();
            }
        }
    }

    int PriorityQueue::size() {
        return values.size();
    }

    int PriorityQueue::getValue() {
        return values.get(0);
    }

    float PriorityQueue::getPriority() {
        return priorities.get(0);
    }

    void PriorityQueue::demote(int index, int value, float priority) {
        int childIndex = (index * 2) + 1; // left child

        while (childIndex < values.size()) {
            float childPriority = priorities.get(childIndex);

            if (childIndex + 1 < values.size()) {
                float rightPriority = priorities.get(childIndex + 1);
                if (sortsEarlierThan(rightPriority, childPriority)) {
                    childPriority = rightPriority;
                    childIndex++; // right child
                }
            }

            if (sortsEarlierThan(childPriority, priority)) {
                priorities.set(index, childPriority);
                values.set(index, values.get(childIndex));
                index = childIndex;
                childIndex = (index * 2) + 1;
            } else {
                break;
            }
        }

        values.set(index, value);
        priorities.set(index, priority);
    }

    int PriorityQueue::pop() {
        int ret = values.get(0);

        // record the value/priority of the last entry
        int lastIndex = values.size() - 1;
        int tempValue = values.get(lastIndex);
        float tempPriority = priorities.get(lastIndex);

        values.removeAt(lastIndex);
        priorities.removeAt(lastIndex);

        if (lastIndex > 0) {
            demote(0, tempValue, tempPriority);
        }

        if (INTERNAL_CONSISTENCY_CHECKING) {
            check();
        }

        return ret;
    }

    /**
     * 
     * @param sortOrder 
     */
    void PriorityQueue::setSortOrder(bool sortOrder) {
        if (this->sortOrder != sortOrder) {
            this->sortOrder = sortOrder;
            // reheapify the arrays
            for (int i = (values.size() / 2) - 1; i >= 0; i--) {
                demote(i, values.get(i), priorities.get(i));
            }
        }
        if (INTERNAL_CONSISTENCY_CHECKING) {
            check();
        }
    }

    /**
     * For each entry, check that the child entries have a lower or equal priority
    **/
    void PriorityQueue::check() {
        int lastIndex = values.size() - 1;

        for (int i = 0; i < values.size() / 2; i++) {
            float currentPriority = priorities.get(i);

            int leftIndex = (i * 2) + 1;
            if (leftIndex <= lastIndex) {
                float leftPriority = priorities.get(leftIndex);
                if (sortsEarlierThan(leftPriority, currentPriority)) {
                    std::cerr << "Internal error in PriorityQueue" << std::endl;
                }
            }

            int rightIndex = (i * 2) + 2;
            if (rightIndex <= lastIndex) {
                float rightPriority = priorities.get(rightIndex);
                if (sortsEarlierThan(rightPriority, currentPriority)) {
                    std::cerr << "Internal error in PriorityQueue" << std::endl;
                }
            }
        }
    }
}
