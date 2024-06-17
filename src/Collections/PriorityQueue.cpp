//
// Created by serafm on 7/6/2024.
//

#include "PriorityQueue.h"

#include <iostream>
#include <ostream>

namespace Collections {


    PriorityQueue::PriorityQueue(bool sortOrder) {
        this->PriorityQueue::PriorityQueue(sortOrder, 10);
    }

    PriorityQueue::PriorityQueue(bool sortOrder, int initialCapacity) {
        this->sortOrder = sortOrder;
        values = TIntArrayList(initialCapacity);
        priorities = TFloatArrayList(initialCapacity);
    }

    bool PriorityQueue::sortsEarlierThan(float p1, float p2) const {
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
        // Loop until the node is at the root or no further promotion is needed
        while (index > 0) {
            // Calculate the index of the parent node
            int parentIndex = (index -1) / 2;

            // Get the priority of the parent node
            float parentPriority = priorities.get(parentIndex);

            // Check if the current node's priority should not be promoted further
            if (sortsEarlierThan(parentPriority, priority)) {
                break;
            }

            // Move the current value and priority up in the heap
            values.set(index, value);
            priorities.set(index, priority);

            // Optional internal consistency check
            if (INTERNAL_CONSISTENCY_CHECKING) {
                check();
            }
        }
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
        int childIndex = (index * 2) + 1; // left child

        // Loop as long as the current node has at least one child
        while (childIndex < values.size()) {
            // Get the priority of the left child
            float childPriority = priorities.get(childIndex);

            // Check if the right child exists
            if (childIndex + 1 < values.size()) {
                // Get the priority of the right child
                float rightPriority = priorities.get(childIndex + 1);
                // Determine if the right child has a higher priority than the left child
                if (sortsEarlierThan(rightPriority, childPriority)) {
                    childPriority = rightPriority;
                    childIndex++; // Move to the right child
                }
            }

            // Compare the priority of the current node with the highest priority child
            if (sortsEarlierThan(childPriority, priority)) {
                // If the child's priority is higher, move the child up
                priorities.set(index, childPriority);
                values.set(index, values.get(childIndex));
                // Move down to the child index
                index = childIndex;
                // Update to the left child of the new index
                childIndex = (index * 2) + 1;
            } else {
                break; // The current node is in the correct position
            }
        }

        // Place the demoted value in its final position
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

    void PriorityQueue::setSortOrder(bool sortOrder) {
        // Check if the sort order needs to be changed
        if (this->sortOrder != sortOrder) {
            // Update the sort order
            this->sortOrder = sortOrder;

            // Reheapify the arrays to maintain the heap property with the new sort order
            for (int i = (values.size() / 2) - 1; i >= 0; i--) {
                demote(i, values.get(i), priorities.get(i));
            }
        }

        if (INTERNAL_CONSISTENCY_CHECKING) {
            check();
        }
    }

    void PriorityQueue::check() {
        // Get the index of the last element
        int lastIndex = values.size() - 1;

        // Loop over all non-leaf nodes to check the heap property
        for (int i = 0; i < values.size() / 2; i++) {
            // Get the priority of the current node
            float currentPriority = priorities.get(i);

            // Check the left child
            int leftIndex = (i * 2) + 1;
            if (leftIndex <= lastIndex) {
                // Get the priority of the left child
                float leftPriority = priorities.get(leftIndex);
                if (sortsEarlierThan(leftPriority, currentPriority)) {
                    std::cerr << "Internal error in PriorityQueue" << std::endl;
                }
            }

            // Check the right child
            int rightIndex = (i * 2) + 2;
            if (rightIndex <= lastIndex) {
                // Get the priority of the right child
                float rightPriority = priorities.get(rightIndex);
                if (sortsEarlierThan(rightPriority, currentPriority)) {
                    std::cerr << "Internal error in PriorityQueue" << std::endl;
                }
            }
        }
    }
}
