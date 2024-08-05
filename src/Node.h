#pragma once

#ifndef NODE_H
#define NODE_H

#include <cstdint>
#include <deque>
#include <limits>
#include <memory>
#include <vector>

namespace rtree {

    class RTree;

    class Node {

        public:

        /**
         * Node entry MBR structure
         */
        struct Entry {
            float minX, minY, maxX, maxY;
            uint32_t id;
        };

        /**
         * Node ID
         */
        uint32_t nodeId;

        /**
         * Minimum bounding rectangle (MBR) of the node
         */
        float mbrMinX = std::numeric_limits<float>::max();
        float mbrMinY = std::numeric_limits<float>::max();
        float mbrMaxX = std::numeric_limits<float>::min();
        float mbrMaxY = std::numeric_limits<float>::min();

        /**
         * The level of the node in the tree
         */
        int level = 0;

        /**
         * Number of entries in node
         */
        int entryCount = 0;

        /**
         * Queue of entries
         */
        std::deque<Entry> entries;

        /**
         * Vector of:
         * Rectangle ids if it's a leaf node / Node ids if it's parent node
         */
        std::vector<uint32_t> ids;

        /**
         * Node constructor. Creates a new node with the given properties.
         * @param level Level of the node in the tree.
         */
        Node(uint32_t id, int level);
        ~Node();

        /**
         * Add new entry to the node.
         * @param minX Min X value
         * @param minY Min Y value
         * @param maxX Max X value
         * @param maxY Max Y value
         * @param id Entry ID
         */
        void addEntry(float minX, float minY, float maxX, float maxY, uint32_t id);

        /**
         * Find entry in the node.
         * @param minX Min X value
         * @param minY Min Y value
         * @param maxX Max X value
         * @param maxY Max Y value
         * @return
         */
        int findEntry(float minX, float minY, float maxX, float maxY, uint32_t id);

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
         * @param rtree
         */
        void reorganize(int maxNodeEntries);

        /**
         * Get all node entries.
         */
        void getNodeEntries();

        /**
         * Check if an entry is empty or incomplete.
         * @param entry Entry struct
         * @return boolean value
         */
        static bool isEmptyOrIncomplete(Entry& entry);

        /**
         * Get the number of entries in the node.
         * @return Number of entries
         */
        [[nodiscard]] int getEntryCount() const;

        /**
         * Get the node ID.
         * @return Node ID
         */
        [[nodiscard]] uint32_t getNodeId() const;

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

    };

}

#endif // NODE_H
