#pragma once

#ifndef RTREE_H
#define RTREE_H

#include <map>
#include <queue>
#include <stack>
#include <vector>
#include "Node.h"
#include "Point.h"
#include "Rectangle.h"

namespace Collections {
    class IntVector;
    class PriorityQueue;
}

namespace spatialindex {

    class RTree {

        private:

        // List of all nodes in the tree
        std::map<int, std::shared_ptr<Node>> m_nodeMap;
        std::vector<int8_t> m_entryStatus;
        std::vector<int8_t> m_initialEntryStatus;
        std::stack<int> m_parents;
        std::stack<int> m_parentsEntry;
        int m_treeHeight = 1;
        int m_rootNodeId = 1;
        std::shared_ptr<Node> m_root;
        int m_size = 0;
        int m_highestUsedNodeId = m_rootNodeId;
        std::stack<int> m_deletedNodeIds;
        std::priority_queue<std::pair<float, int>> m_distanceQueue;

        void add(float minX, float minY, float maxX, float maxY, int id, int level);
        std::shared_ptr<Node> adjustTree(std::shared_ptr<Node> n, std::shared_ptr<Node> nn);
        void condenseTree(const std::shared_ptr<Node> &l);
        std::shared_ptr<Node> chooseNode(float minX, float minY, float maxX, float maxY, int level);
        static Rectangle& calculateMBR(Node &n);
        void createNearestNDistanceQueue(Point& p, int count, float furthestDistance);
        int getNextNodeId();
        bool intersects(Rectangle &r, Node &n);
        float nearest(Point &p, std::shared_ptr<Node> &n, float furthestDistanceSq, Collections::IntVector &nearestIds);
        static void pickSeeds(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId, const std::shared_ptr<Node>& newNode);
        int pickNext(const std::shared_ptr<Node>& n, const std::shared_ptr<Node>& newNode);
        std::shared_ptr<Node> splitNode(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId);
        static void printSortedQueue(std::priority_queue<std::pair<float, int>>& queue);

        public:

        static constexpr int DEFAULT_MAX_NODE_ENTRIES = 50;
        static constexpr int DEFAULT_MIN_NODE_ENTRIES = 20;
        static constexpr int ENTRY_STATUS_ASSIGNED = 0;
        static constexpr int ENTRY_STATUS_UNASSIGNED = 1;

        int m_maxNodeEntries{};
        int m_minNodeEntries{};
        std::vector<int> m_ids;

        RTree();

        void add(const Rectangle & r, int id);
        void contains(Rectangle& r);
        bool del(Rectangle &r, int id);
        std::shared_ptr<Node> &getNode(int id);
        int getRootNodeId() const;
        void intersects(Rectangle& r);
        int numNodes() const;
        void nearest(Point& p, float furthestDistance);
        void nearestNUnsorted(Point& p, int count, float furthestDistance);
        void nearestN(Point& p, int count, float furthestDistance);
        int treeSize() const;
        Rectangle getBounds();

    };
}
#endif // RTREE_H
