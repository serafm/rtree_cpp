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

        /**
        * Insert a value, append it to the arrays. Then reheapify by promoting it to the correct place.
        * @param value The value to insert.
        * @param priority The priority of the value.
        */
        void insert(int value, float priority);


        int size();
        void clear();
        void reset();
        int getValue();
        float getPriority();

        /**
        * Get the value with the lowest priority. Creates a "hole" at the root of the tree.
        * The algorithm swaps the hole with the appropriate child, until
        * the last entry will fit correctly into the hole (ie is lower priority than its children)
        * @return The value with the lowest priority
        */
        int pop();

        /**
         * Allows the priority queue to switch between different sorting orders
         * (e.g., min-heap or max-heap). When the sort order is changed,
         * the method reheapifies the entire queue to ensure that the heap
         * property is maintained according to the new order.
         * This involves demoting each non-leaf node to its correct position in the heap.
         * @param sortOrder The sort order
         */
        void setSortOrder(bool sortOrder);

    private:
        TIntArrayList values;
        TFloatArrayList priorities;
        bool sortOrder = SORT_ORDER_ASCENDING;
        bool INTERNAL_CONSISTENCY_CHECKING = false;

        /**
        * @param p1
        * @param p2
        * @return true if p1 has an earlier sort order than p2.
        */
        bool sortsEarlierThan(float p1, float p2) const;

        /**
         * To move a node up the heap to its correct position based on its priority.
         * This operation is essential for maintaining the heap property of the priority queue
         * after changes that may affect the ordering.
         * @param index the index of the node
         * @param value the new value
         * @param priority the priority
         */
        void promote(int index, int value, float priority);

        /**
         * Move a node down the heap to its correct position based on its priority.
         * This operation is essential for maintaining the heap property of the
         * priority queue after changes that may affect the ordering.
         * @param index the index of the node
         * @param value the new value
         * @param priority the priority
         */
        void demote(int index, int value, float priority);

        /**
         * Verifies the internal consistency of the priority queue
         * by ensuring that each non-leaf node maintains the heap property
         * with respect to its children.
         */
        void check();

    };
}

#endif //PRIORITYQUEUE_H
