#pragma once

#ifndef NODE_H
#define NODE_H

#include <limits>
#include <math.h>
#include <vector>

namespace rtree {

    class QueryBuilder;

    class Node {

        public:

        /**
         * Node entry MBR structure
         */
        struct Entry {
            float minX, minY, maxX, maxY;
            int id;
        };

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
        int level = 0;

        /**
         * Number of entries in the node
         */
        int entryCount = 0;

        /**
         * Queue of entries
         */
        std::vector<float> entriesMinX{};
        std::vector<float> entriesMinY{};
        std::vector<float> entriesMaxX{};
        std::vector<float> entriesMaxY{};

        /**
         * Vector of:
         * Rectangle ids if it's a leaf node / Node ids if it's parent node
         */
        std::vector<int> ids;

        /**
         * Node constructor. Creates a new node with the given properties.
         * @param id Node id
         * @param level Level of the node in the tree.
         */
        Node(int id, int level);
        Node();
        ~Node();

        /**
         * Add new entry to the node.
         * @param minX Min X value
         * @param minY Min Y value
         * @param maxX Max X value
         * @param maxY Max Y value
         * @param id Entry ID
         */
        void addEntry(float minX, float minY, float maxX, float maxY, int id);

        /**
         * Find entry in the node.
         * @param minX Min X value
         * @param minY Min Y value
         * @param maxX Max X value
         * @param maxY Max Y value
         * @param id Entry id
         * @return
         */
        int findEntry(float minX, float minY, float maxX, float maxY, int id);

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
         * Get all node entries.
         */
        void getNodeEntries() const;

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
         *
         * @return
         */
        [[nodiscard]] bool isEmpty() const;

        //void reset(int newLevel);

    };

}

#endif // NODE_H
