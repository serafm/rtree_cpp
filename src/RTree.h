#ifndef RTREE_H
#define RTREE_H

#include "Node.h"
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>

#include "Point.h"
#include "Rectangle.h"

namespace SpatialIndex {

    class RTree {
    private:
        static const int DEFAULT_MAX_NODE_ENTRIES = 50;
        static const int DEFAULT_MIN_NODE_ENTRIES = 20;
        std::unordered_map<int, std::string> nodeMap;
        static const bool INTERNAL_CONSISTENCY_CHECKING = false;
        static const int ENTRY_STATUS_ASSIGNED = 0;
        static const int ENTRY_STATUS_UNASSIGNED = 1;
        std::vector<int8_t> entryStatus;
        std::vector<int8_t> initialEntryStatus;
        std::stack<int, std::vector<int>> parents;
        std::stack<int, std::vector<int>> parentsEntry;
        int treeHeight = 1;
        int rootNodeId = 0;
        int size = 0;
        int highestUsedNodeId = rootNodeId;
        std::stack<int, std::vector<int>> deleteNodeIds;

        void add(float minX, float minY, float maxX, float maxY, int id, int level);
        void createNearestNDistanceQueue(Point p, int count, PriorityQueue distanceQueue, float furthestDistance);
        void createNearestNDistanceQueue(Point p, int count, PriorityQueue distanceQueue, float furthestDistance);
        int getNextNodeId();
        Node splitNode(Node n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId);
        void pickSeeds(Node n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId, Node newNode);
        int pickNext(Node n, Node newNode);
        float nearest(Point p, Node n, float furthestDistanceSq, TIntArrayList nearestIds);
        bool intersects(Rectangle r, TIntProcedure v, Node n);
        void condenseTree(Node l);
        Node chooseNode(float minX, float minY, float maxX, float maxY, int level);
        Node adjustTree(Node n, Node nn);
        bool checkConsistency(int nodeId, int expectedLevel, Rectangle expectedMBR);
        Rectangle calculateMBR(Node n);

    public:
        int maxNodeEntries;
        int minNodeEntries;

        RTree();
        void init();
        void add(Rectangle r, int id);
        bool del(Rectangle r, int id);
        void nearest(Point p, TIntProcedure v, float furthestDistance);
        void nearestNUnsorted(Point p, TIntProcedure v, int count, float furthestDistance);
        void nearestN(Point p, TIntProcedure v, int count, float furthestDistance);
        void intersects(Rectangle r, TIntProcedure v);
        void contains(Rectangle r, TIntProcedure v);
        int treeSize();
        Rectangle getBounds();
        Node getNode(int id);
        int getRootNodeId();
        bool checkConsistency();

    };
}
#endif // RTREE_H
