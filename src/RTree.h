#ifndef RTREE_H
#define RTREE_H

#include "Node.h"
#include <vector>
#include <stack>
#include <unordered_map>
#include "Point.h"
#include "Rectangle.h"
#include "Collections/Procedure.h"

namespace Collections {
    class IntVector;
    class PriorityQueue;
}

namespace SpatialIndex {

    class RTree {
    private:
        static constexpr int DEFAULT_MAX_NODE_ENTRIES = 50;
        static constexpr int DEFAULT_MIN_NODE_ENTRIES = 20;
        std::unordered_map<int, Node> nodeMap;
        static constexpr bool INTERNAL_CONSISTENCY_CHECKING = false;
        static constexpr int ENTRY_STATUS_ASSIGNED = 0;
        static constexpr int ENTRY_STATUS_UNASSIGNED = 1;
        std::vector<int8_t> entryStatus;
        std::vector<int8_t> initialEntryStatus;
        std::stack<int> parents;
        std::stack<int> parentsEntry;
        int treeHeight = 1;
        int rootNodeId = 0;
        int size = 0;
        int highestUsedNodeId = rootNodeId;
        std::stack<int> deletedNodeIds;

        void add(float minX, float minY, float maxX, float maxY, int id, int level);
        void createNearestNDistanceQueue(Point p, int count, Collections::PriorityQueue distanceQueue, float furthestDistance);
        int getNextNodeId();
        Node splitNode(Node n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId);
        void pickSeeds(Node n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId, Node newNode);
        int pickNext(Node n, Node newNode);
        float nearest(Point p, Node n, float furthestDistanceSq, Collections::IntVector nearestIds);
        bool intersects(Rectangle r, Collections::Procedure v, Node n);
        void condenseTree(const Node& l);
        Node chooseNode(float minX, float minY, float maxX, float maxY, int level);
        Node adjustTree(Node n, Node nn);
        bool checkConsistency(int nodeId, int expectedLevel, Rectangle expectedMBR);
        static Rectangle calculateMBR(Node n);

    public:
        int maxNodeEntries;
        int minNodeEntries;

        RTree() = default;
        void init();
        void add(Rectangle r, int id);
        bool del(Rectangle r, int id);
        void nearest(Point p, Collections::Procedure v, float furthestDistance);
        void nearestNUnsorted(Point p, Collections::Procedure v, int count, float furthestDistance);
        void nearestN(Point p, Collections::Procedure v, int count, float furthestDistance);
        void intersects(Rectangle r, Collections::Procedure v);
        void contains(Rectangle r, Collections::Procedure v);
        int treeSize() const;
        Rectangle getBounds();
        Node getNode(int id);
        int getRootNodeId() const;
        bool checkConsistency();

    };
}
#endif // RTREE_H
