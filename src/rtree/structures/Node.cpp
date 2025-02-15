#include "Node.h"

#include "../builders/RTreeBulkLoad.h"

namespace rtree {

    Node::Node(int id, int level)
    : nodeId(id),
    level(level)
    {
        entriesMinX = std::vector<float>(RTreeBulkLoad::DEFAULT_MAX_NODE_ENTRIES);
        entriesMinY = std::vector<float>(RTreeBulkLoad::DEFAULT_MAX_NODE_ENTRIES);
        entriesMaxX = std::vector<float>(RTreeBulkLoad::DEFAULT_MAX_NODE_ENTRIES);
        entriesMaxY = std::vector<float>(RTreeBulkLoad::DEFAULT_MAX_NODE_ENTRIES);
        ids = std::vector<int>(RTreeBulkLoad::DEFAULT_MAX_NODE_ENTRIES);
    }

    Node::Node() {
        entriesMinX = std::vector<float>(RTreeBulkLoad::DEFAULT_MAX_NODE_ENTRIES);
        entriesMinY = std::vector<float>(RTreeBulkLoad::DEFAULT_MAX_NODE_ENTRIES);
        entriesMaxX = std::vector<float>(RTreeBulkLoad::DEFAULT_MAX_NODE_ENTRIES);
        entriesMaxY = std::vector<float>(RTreeBulkLoad::DEFAULT_MAX_NODE_ENTRIES);
        ids = std::vector<int>(RTreeBulkLoad::DEFAULT_MAX_NODE_ENTRIES);
    }

    Node::~Node() = default;

    void Node::addEntry(float minX, float minY, float maxX, float maxY, int id) {
        ids[entryCount] = id;
        entriesMinX[entryCount] = minX;
        entriesMinY[entryCount] = minY;
        entriesMaxX[entryCount] = maxX;
        entriesMaxY[entryCount] = maxY;

        if (minX < mbrMinX) mbrMinX = minX;
        if (minY < mbrMinY) mbrMinY = minY;
        if (maxX > mbrMaxX) mbrMaxX = maxX;
        if (maxY > mbrMaxY) mbrMaxY = maxY;

        entryCount++;
    }

    int Node::findEntry(float minX, float minY, float maxX, float maxY, int id) const {
        for (int i = 0; i < entryCount; i++) {
            if (id == ids[i] &&
              entriesMinX[i] == minX && entriesMinY[i] == minY &&
              entriesMaxX[i] == maxX && entriesMaxY[i] == maxY) {
                return i;
              }
        }
        return -1;
    }

    void Node::deleteEntry(int i) {
        int lastIndex = entryCount - 1;
        float deletedMinX = entriesMinX[i];
        float deletedMinY = entriesMinY[i];
        float deletedMaxX = entriesMaxX[i];
        float deletedMaxY = entriesMaxY[i];

        if (i != lastIndex) {
            entriesMinX[i] = entriesMinX[lastIndex];
            entriesMinY[i] = entriesMinY[lastIndex];
            entriesMaxX[i] = entriesMaxX[lastIndex];
            entriesMaxY[i] = entriesMaxY[lastIndex];
            ids[i] = ids[lastIndex];
        }
        entryCount--;

        // adjust the MBR
        recalculateMBRIfInfluencedBy(deletedMinX, deletedMinY, deletedMaxX, deletedMaxY);
    }

    void Node::recalculateMBRIfInfluencedBy(float deletedMinX, float deletedMinY, float deletedMaxX, float deletedMaxY) {
        if (mbrMinX == deletedMinX || mbrMinY == deletedMinY || mbrMaxX == deletedMaxX || mbrMaxY == deletedMaxY) {
            recalculateMBR();
        }
    }

    void Node::recalculateMBR() {
        mbrMinX = entriesMinX[0];
        mbrMinY = entriesMinY[0];
        mbrMaxX = entriesMaxX[0];
        mbrMaxY = entriesMaxY[0];

        for (int i = 1; i < entryCount; i++) {
            if (entriesMinX[i] < mbrMinX) mbrMinX = entriesMinX[i];
            if (entriesMinY[i] < mbrMinY) mbrMinY = entriesMinY[i];
            if (entriesMaxX[i] > mbrMaxX) mbrMaxX = entriesMaxX[i];
            if (entriesMaxY[i] > mbrMaxY) mbrMaxY = entriesMaxY[i];
        }
    }

    void Node::reorganize(int maxNodeEntries) {
        int countdownIndex = maxNodeEntries - 1;
        for (int index = 0; index < entryCount; index++) {
            if (ids[index] == -1) {
                while (ids[countdownIndex] == -1 && countdownIndex > index) {
                    countdownIndex--;
                }
                entriesMinX[index] = entriesMinX[countdownIndex];
                entriesMinY[index] = entriesMinY[countdownIndex];
                entriesMaxX[index] = entriesMaxX[countdownIndex];
                entriesMaxY[index] = entriesMaxY[countdownIndex];
                ids[index] = ids[countdownIndex];
                ids[countdownIndex] = -1;
            }
        }
    }

    int Node::getEntryCount() const {
        return entryCount;
    }

    int Node::getNodeId() const {
        return nodeId;
    }

    bool Node::isLeaf() const {
        return level == 1;
    }

    int Node::getLevel() const {
        return level;
    }

    bool Node::isEmpty() const {
        return level == 0;
    }

}
