#pragma once

#ifndef RTREE_H
#define RTREE_H

#include <boost/container/vector.hpp>
#include <map>
#include <memory>
#include <queue>
#include <set>
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

        // Adds a new rectangle to the R-tree
        void add(float minX, float minY, float maxX, float maxY, int id, int level);

        // Adjusts the tree after an insertion or a split
        std::shared_ptr<Node> adjustTree(std::shared_ptr<Node> n, std::shared_ptr<Node> nn);

        // Selects the most appropriate leaf node for inserting a new rectangle
        std::shared_ptr<Node> chooseNode(float minX, float minY, float maxX, float maxY, int level);

        // Calculates and updates the Minimum Bounding Rectangle (MBR) of a node n based on the MBRs of its child nodes or entries
        static Rectangle& calculateMBR(Node &n);

        // Builds a priority queue containing the nearest N rectangles to a given point p
        void createNearestNDistanceQueue(const Point & p, int count, float furthestDistance);

        // Generates and returns an id for a new node in the tree
        int getNextNodeId();

        // Finds all rectangles in the subtree rooted at node n that intersect with a given rectangle r
        std::set<int> intersects(Rectangle &r, std::shared_ptr<Node>& n);

        // Recursively searches for the nearest rectangle to a point p within the subtree rooted at node n
        float nearest(Point &p, std::shared_ptr<Node> &n, float furthestDistanceSq, boost::container::vector<int> &nearestIds);

        // Selects two initial entries (seeds) to start the node splitting process when a node overflows
        static void pickSeeds(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId, const std::shared_ptr<Node>& newNode);

        // During node splitting, this function selects the next entry to assign to one of the two groups (original node n or new node newNode).
        // It chooses the entry that causes the greatest difference in area enlargement between the two groups
        int pickNext(const std::shared_ptr<Node>& n, const std::shared_ptr<Node>& newNode);

        // Splits a full node n into two nodes after inserting a new rectangle defined by the provided coordinates and newId.
        // It redistributes the entries between the original node and a new node, and returns the new node created
        std::shared_ptr<Node> splitNode(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId);

        // Prints the contents of a priority queue containing pairs of distances and rectangle IDs.
        static void printSortedQueue(std::priority_queue<std::pair<float, int>>& queue);

        // Retrieves a reference to the node with the specified id
        std::shared_ptr<Node> &getNode(int id);

        // Returns the root node ID of the tree
        int getRootNodeId() const;

        // Prints the rectangle ids contained by a range query
        static void printContainedRectangles(const std::vector<int>& ids);

        // Prints the rectangle ids intersecting with a given query
        static void printIntersectedRectangles(const std::set<int>& ids);

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

        // Find all rectangles in the tree that are contained by the passed rectangle
        void contains(Rectangle& r);

        void intersects(Rectangle& r);

        // Retrieve the nearest rectangle to a point
        void nearest(Point &p, float furthestDistance);
        static void printNearest(Point& p, boost::container::vector<int>& nearestIds, float distance);

        // Retrieve the N nearest rectangles to a point without sorting the results
        void nearestNUnsorted(Point& p, int count, float furthestDistance);

        // Retrieve the N nearest rectangles to a point with sorted results
        void nearestN(Point& p, int count, float furthestDistance);

        // Return R-tree size
        int treeSize() const;

        // Return R-tree number of nodes
        int numNodes() const;

    };
}
#endif // RTREE_H
