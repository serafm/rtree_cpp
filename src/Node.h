#ifndef NODE_H
#define NODE_H

#include <limits>
#include <vector>
#include "RTree.h"

namespace SpatialIndex {

    class Node {
    public:
        int nodeId = 0;
        float mbrMinX = std::numeric_limits<float>::max();
        float mbrMinY = std::numeric_limits<float>::max();
        float mbrMaxX = std::numeric_limits<float>::min();
        float mbrMaxY = std::numeric_limits<float>::min();

        std::vector<float> entriesMinX;
        std::vector<float> entriesMinY;
        std::vector<float> entriesMaxX;
        std::vector<float> entriesMaxY;

        std::vector<int> ids;
        int level;
        int entryCount;

        Node(int nodeId, int level, int maxNodeEntries);
        Node() = default;
        ~Node();
        void addEntry(float minX, float minY, float maxX, float maxY, int id);
        int findEntry(float minX, float minY, float maxX, float maxY, int id);
        void deleteEntry(int i);
        void recalculateMBRIfInfluencedBy(float deletedMinX, float deletedMinY, float deletedMaxX, float deletedMaxY);
        void recalculateMBR();
        void reorganize(RTree rtree);
        int getEntryCount();
        int getId(int index);
        bool isLeaf();
        int getLevel();
    };

}
#endif // NODE_H
