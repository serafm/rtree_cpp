#pragma once

#ifndef NODE_H
#define NODE_H

#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <vector>

#include "Rectangle.h"

namespace rtree {

    class QueryBuilder;

    class Node {

        public:

        std::vector<Node*> children;
        std::vector<Rectangle> leafs;
        std::vector<int> ids;

        /**
         * Node ID
         */
        int nodeId{};

        /**
         * Minimum bounding rectangle (MBR) of the node
         */
        float mbrMinX = MAXFLOAT;
        float mbrMinY = MAXFLOAT;
        float mbrMaxX = -MAXFLOAT;
        float mbrMaxY = -MAXFLOAT;

        /**
         * The level of the node in the tree
         */
        int level{};

        /**
         * Number of entries in the node
         */
        int entryCount{};

        /**
         * Vector of entries coordinates
         */
        std::vector<float> entriesMinX{};
        std::vector<float> entriesMinY{};
        std::vector<float> entriesMaxX{};
        std::vector<float> entriesMaxY{};

        /**
         * Node constructor. Creates a new node with the given properties.
         * @param id Node id
         * @param level Level of the node in the tree.
         */
        Node(int id, int level, int capacity);
        Node(int capacity);
        ~Node();

        /**
         * Add new entry to the node.
         * @param minX Min X value
         * @param minY Min Y value
         * @param maxX Max X value
         * @param maxY Max Y value
         * @param id Entry ID
         */
        void addChildEntry(Node* n);

        void addLeafEntry(const Rectangle& rect);

        void sortChildrenByMinX();
        void sortLeafsByMinX();

        /**
         * Find entry in the node.
         * @param minX Min X value
         * @param minY Min Y value
         * @param maxX Max X value
         * @param maxY Max Y value
         * @param id Entry id
         * @return
         */
        int findEntry(float minX, float minY, float maxX, float maxY, int id) const;

        /**
         * Delete entry by index.
         * @param index Entry index
         */
        void deleteEntry(int index);

        /**
         * Recalculate the MBR of the node if necessary.
         * @param deletedMinX Min X value of deleted entry
         * @param deletedMinY Min Y value of deleted entry
         * @param deletedMaxX Max X value of deleted entry
         * @param deletedMaxY Max Y value of deleted entry
         */
        void recalculateMBRIfInfluencedBy(float deletedMinX, float deletedMinY, float deletedMaxX, float deletedMaxY);

        /**
         * Recalculate node's MBR.
         */
        void recalculateMBR();

        /**
         * Reorganize the tree.
         */
        void reorganize(int maxNodeEntries);

        /**
         * Get the number of entries in the node.
         * @return Number of entries
         */
        [[nodiscard]] int getEntryCount() const;

        /**
         * Get the node ID.
         * @return Node ID
         */
        [[nodiscard]] int getNodeId() const;

        /**
         * Check if node is leaf node.
         * @return boolean value
         */
        [[nodiscard]] bool isLeaf() const;

        /**
         * Get the level of the node.
         * @return Level value
         */
        [[nodiscard]] int getLevel() const;

        /**
         * Check if the node is empty
         * @return boolean value
         */
        [[nodiscard]] bool isEmpty() const;
    };

}

#endif // NODE_H
