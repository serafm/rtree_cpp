#include "PriorityQueue.h"
#include <iostream>
#include <ostream>

namespace Collections {
    const bool SORT_ORDER_ASCENDING = true; // Introducing constant

    PriorityQueue::PriorityQueue(bool sortOrder) : PriorityQueue(sortOrder, 10) {} // Change Signature

    PriorityQueue::PriorityQueue(bool sortOrder, int initialCapacity)
        : sortOrder(sortOrder), values(initialCapacity), priorities(initialCapacity) {}

    bool PriorityQueue::sortsEarlierThan(float priority1, float priority2) const {
        return sortOrder == SORT_ORDER_ASCENDING ? priority1 < priority2 : priority1 > priority2;
    }

    void PriorityQueue::insert(uint32_t value, float priority) {
        values.add(value);
        priorities.add(priority);
        promote((int)values.size() - 1, value, priority);
    }

    void PriorityQueue::promote(int index, int value, float priority) {
        while (index > 0) {
            int parentIndex = (index - 1) / 2;
            float parentPriority = priorities.get(parentIndex);
            if (sortsEarlierThan(parentPriority, priority)) {
                break;
            }
            setNode(index, value, priority);
            index = parentIndex;
            if (INTERNAL_CONSISTENCY_CHECKING) {
                check();
            }
        }
    }

    void PriorityQueue::setNode(int index, int value, float priority) { // Extract Function
        values.set(index, value);
        priorities.set(index, priority);
    }

    int PriorityQueue::size() {
        return values.size();
    }

    void PriorityQueue::clear() {
        values.clear();
        priorities.clear();
    }

    int PriorityQueue::getValue() {
        return values.get(0);
    }

    float PriorityQueue::getPriority() {
        return priorities.get(0);
    }

    void PriorityQueue::demote(int index, int value, float priority) {
        int childIndex = (index * 2) + 1;
        while (childIndex < values.size()) {
            float childPriority = priorities.get(childIndex);
            if (childIndex + 1 < values.size() && sortsEarlierThan(priorities.get(childIndex + 1), childPriority)) {
                childPriority = priorities.get(childIndex + 1);
                childIndex++;
            }
            if (sortsEarlierThan(childPriority, priority)) {
                setNode(index, values.get(childIndex), childPriority);
                index = childIndex;
                childIndex = (index * 2) + 1;
            } else {
                break;
            }
        }
        setNode(index, value, priority);
    }

    int PriorityQueue::pop() {
        int returnValue = values.get(0);
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
        return returnValue;
    }

    void PriorityQueue::setSortOrder(bool sortOrder) {
        if (this->sortOrder != sortOrder) {
            this->sortOrder = sortOrder;
            for (int i = (values.size() / 2) - 1; i >= 0; i--) {
                demote(i, values.get(i), priorities.get(i));
            }
        }
        if (INTERNAL_CONSISTENCY_CHECKING) {
            check();
        }
    }

    void PriorityQueue::check() {
        int lastIndex = values.size() - 1;
        for (int i = 0; i < values.size() / 2; i++) {
            float currentPriority = priorities.get(i);
            int leftIndex = (i * 2) + 1;
            if (leftIndex <= lastIndex && sortsEarlierThan(priorities.get(leftIndex), currentPriority)) {
                std::cerr << "Internal error in PriorityQueue" << std::endl;
            }
            int rightIndex = (i * 2) + 2;
            if (rightIndex <= lastIndex && sortsEarlierThan(priorities.get(rightIndex), currentPriority)) {
                std::cerr << "Internal error in PriorityQueue" << std::endl;
            }
        }
    }
}