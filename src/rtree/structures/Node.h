#pragma once

#ifndef NODE_H
#define NODE_H

#include <cmath>
#include <vector>
#include <algorithm>

#include "Rectangle.h"

namespace rtree {

    /**
     * Node class representing a single node within an R-tree.
     * Nodes can either contain child nodes (internal nodes) or leaf rectangles (leaf nodes).
     */
    class Node {

    public:

        /**
         * Child nodes (only used if this is an internal node).
         */
        std::vector<Node*> children;

        /**
         * Leaf rectangles (only used if this is a leaf node).
         */
        std::vector<Rectangle> leafs;

        /**
         * IDs corresponding to the leaf rectangles (only used if this is a leaf node).
         */
        std::vector<int> ids;

        /**
         * Unique identifier for the node.
         */
        int nodeId{};

        /**
         * Minimum bounding rectangle (MBR) of this node.
         */
        float mbrMinX = MAXFLOAT;
        float mbrMinY = MAXFLOAT;
        float mbrMaxX = -MAXFLOAT;
        float mbrMaxY = -MAXFLOAT;

        /**
         * The level of the node in the tree.
         * Level 1 nodes are leaf nodes, while higher levels are internal nodes.
         */
        int level{};

        /**
         * Number of entries (either children or leaf rectangles) in the node.
         */
        int entryCount{};

        /**
         * Constructor for internal nodes.
         * @param id Node identifier.
         * @param level Level of the node within the tree.
         * @param capacity Maximum number of entries this node can hold.
         */
        Node(int id, int level, int capacity);

        /**
         * Constructor for leaf nodes or empty nodes.
         * @param capacity Maximum number of entries this node can hold.
         */
        Node(int capacity);

        /**
         * Destructor - cleans up child pointers if necessary.
         */
        ~Node();

        /**
         * Add a child node to this node (used for internal nodes).
         * Updates the MBR to include the new child node.
         * @param n Pointer to the child node being added.
         */
        void addChildEntry(Node* n);

        /**
         * Add a leaf rectangle to this node (used for leaf nodes).
         * Updates the MBR to include the new rectangle.
         * @param rect The rectangle being added.
         */
        void addLeafEntry(const Rectangle& rect);

        /**
         * Sort the child nodes by their minimum X coordinate.
         * This is useful for certain spatial queries and tree rebalancing.
         */
        void sortChildrenByMinX();

        /**
         * Sort the leaf rectangles by their minimum X coordinate.
         * This is useful for certain spatial queries and tree rebalancing.
         */
        void sortLeafsByMinX();

        /**
         * Delete an entry by index.
         * @param index Index of the entry to remove.
         */
        void deleteEntry(int index);

        /**
         * Recalculate the minimum bounding rectangle (MBR) for the node.
         * Iterates over all current entries to compute the MBR.
         */
        void recalculateMBR();

        /**
         * Get the current number of entries in this node.
         * @return The number of entries (either children or leaf rectangles).
         */
        [[nodiscard]] int getEntryCount() const;

        /**
         * Get the unique identifier of this node.
         * @return Node ID.
         */
        [[nodiscard]] int getNodeId() const;

        /**
         * Check if this node is a leaf node.
         * Leaf nodes contain rectangles; internal nodes contain child nodes.
         * @return True if this is a leaf node, false if internal.
         */
        [[nodiscard]] bool isLeaf() const;

        /**
         * Get the level of this node within the tree.
         * @return Level (0 for leaf nodes, higher values for internal nodes).
         */
        [[nodiscard]] int getLevel() const;

        /**
         * Check if the node is empty (contains no entries).
         * @return True if empty, false otherwise.
         */
        [[nodiscard]] bool isEmpty() const;
    };

}

#endif // NODE_H
