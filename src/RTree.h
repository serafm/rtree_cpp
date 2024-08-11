#pragma once

#ifndef RTREE_H
#define RTREE_H

#include <stack>
#include <unordered_map>
#include <vector>
#include "Node.h"
#include "Point.h"
#include "Rectangle.h"

namespace Collections {
    class IntVector;
    class PriorityQueue;
}

namespace rtree {

    class RTree {

        private:

        static constexpr int DEFAULT_MAX_NODE_ENTRIES = 50;
        static constexpr int DEFAULT_MIN_NODE_ENTRIES = 20;

        // List of all nodes in the tree
        std::unordered_map<uint32_t, Node&> nodeMap;

        static constexpr int ENTRY_STATUS_ASSIGNED = 0;
        static constexpr int ENTRY_STATUS_UNASSIGNED = 1;
        std::vector<int8_t> entryStatus;
        std::vector<int8_t> initialEntryStatus;
        std::stack<uint32_t> parents;
        std::stack<uint32_t> parentsEntry;
        int treeHeight = 1;
        uint32_t rootNodeId = 1;
        Node root;
        int size = 0;
        uint32_t highestUsedNodeId = rootNodeId;
        std::stack<uint32_t> deletedNodeIds;

        void add(float minX, float minY, float maxX, float maxY, uint32_t id, int level);
        Node& adjustTree(Node& n, Node& nn);
        void condenseTree(Node& l);
        Node& chooseNode(float minX, float minY, float maxX, float maxY, int level);
        static Rectangle& calculateMBR(Node& n);
        void createNearestNDistanceQueue(Point& p, int count, Collections::PriorityQueue& distanceQueue, float furthestDistance);
        uint32_t getNextNodeId();
        bool intersects(Rectangle& r, Node& n);
        float nearest(Point& p, Node& n, float furthestDistanceSq, Collections::IntVector& nearestIds);
        void pickSeeds(Node& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, uint32_t newId, Node& newNode);
        int pickNext(Node& n, Node& newNode);
        Node splitNode(Node& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, uint32_t newId);

        public:

        int maxNodeEntries{};
        int minNodeEntries{};
        std::vector<uint32_t> ids;

        RTree();

        void add(Rectangle& r, uint32_t id);
        void contains(Rectangle& r);
        bool del(Rectangle& r, uint32_t id);
        Node& getNode(uint32_t id);
        uint32_t getRootNodeId() const;
        void intersects(Rectangle& r);
        uint32_t numNodes() const;
        void nearest(Point& p, float furthestDistance);
        void nearestNUnsorted(Point& p, int count, float furthestDistance);
        void nearestN(Point& p, int count, float furthestDistance);
        int treeSize() const;
        //Rectangle getBounds();

    };
}
#endif // RTREE_H
