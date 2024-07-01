#ifndef NODE_H
#define NODE_H

#include <limits>
#include <optional>
#include <vector>

namespace SpatialIndex {

    class RTree;
    class Node {
    public:
        struct Entry {
            float minX, minY, maxX, maxY;
            int id;
        };

        int nodeId = 0;
        float mbrMinX = std::numeric_limits<float>::max();
        float mbrMinY = std::numeric_limits<float>::max();
        float mbrMaxX = std::numeric_limits<float>::min();
        float mbrMaxY = std::numeric_limits<float>::min();

        std::vector<Entry> entries;

        std::vector<int> ids;
        int level = 0;
        int entryCount = 0;

        Node(int nodeId, int level, int maxNodeEntries);
        Node() = default;
        ~Node();
        void addEntry(float minX, float minY, float maxX, float maxY, int id);
        int findEntry(float minX, float minY, float maxX, float maxY, int id);
        void deleteEntry(int i);
        void recalculateMBRIfInfluencedBy(float deletedMinX, float deletedMinY, float deletedMaxX, float deletedMaxY);
        void recalculateMBR();
        void reorganize(RTree* rtree);
        int getEntryCount() const;
        [[nodiscard]] int getId(int index) const;
        [[nodiscard]] bool isLeaf() const;
        [[nodiscard]] int getLevel() const;
        [[nodiscard]] bool isEmpty() const;
    };

}
#endif // NODE_H
