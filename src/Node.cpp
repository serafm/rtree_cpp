#include "Node.h"

namespace SpatialIndex {

    Node::Node(int nodeId, int level, int maxNodeEntries) {
        this->nodeId = nodeId;
        this->level = level;
        std::vector<float> entriesMinX(maxNodeEntries, 0);
        std::vector<float> entriesMinY(maxNodeEntries, 0);
        std::vector<float> entriesMaxX(maxNodeEntries, 0);
        std::vector<float> entriesMaxY(maxNodeEntries, 0);
        std::vector<int> ids(maxNodeEntries, 0);
    }

    Node::~Node() {
    }

    void Node::addEntry(float minX, float minY, float maxX, float maxY, int id) {
        ids.at(entryCount) = id;
        entriesMinX.at(entryCount) = minX;
        entriesMinY.at(entryCount) = minY;
        entriesMaxX.at(entryCount) = maxX;
        entriesMaxY.at(entryCount) = maxY;

        if (minX < mbrMinX) {
            mbrMinX = minX;
        }
        if (minY < mbrMinY) {
            mbrMinY = minY;
        }
        if (maxX > mbrMaxX) {
            mbrMaxX = maxX;
        }
        if (maxY > mbrMaxY) {
            mbrMaxY = maxY;
        }

        entryCount++;
    }

    int Node::findEntry(float minX, float minY, float maxX, float maxY, int id) {
        for (int i = 0; i < entryCount; i++) {
            if (id == ids.at(i) && entriesMinX.at(i) == minX && entriesMinY.at(i) == minY && entriesMaxX.at(i) == maxX && entriesMaxY.at(i) == maxY) {
                return i;
            }
        }
        return -1;
    }

    void Node::deleteEntry(int i) {
        int lastIndex = entryCount - 1;
        float deletedMinX = entriesMinX.at(i);
        float deletedMinY = entriesMinY.at(i);
        float deletedMaxX = entriesMaxX.at(i);
        float deletedMaxY = entriesMaxY.at(i);

        if (i != lastIndex) {
            entriesMinX.at(i) = entriesMinX.at(lastIndex);
            entriesMinY.at(i) = entriesMinY.at(lastIndex);
            entriesMaxX.at(i) = entriesMaxX.at(lastIndex);
            entriesMaxY.at(i) = entriesMaxY.at(lastIndex);
            ids.at(i) = ids.at(lastIndex);
        }

        entryCount--;

        recalculateMBRIfInfluencedBy(deletedMinX, deletedMinY, deletedMaxX, deletedMaxY);
    }

    void Node::recalculateMBRIfInfluencedBy(float deletedMinX, float deletedMinY, float deletedMaxX, float deletedMaxY) {
        if (mbrMinX == deletedMinX || mbrMinY == deletedMinY || mbrMaxX == deletedMaxX || mbrMaxY == deletedMaxY) {
            recalculateMBR();
        }
    }

    void Node::recalculateMBR() {
        mbrMinX = entriesMinX.at(0);
        mbrMinY = entriesMinY.at(0);
        mbrMaxX = entriesMaxX.at(0);
        mbrMaxY = entriesMaxY.at(0);

        for (int i = 1; i < entryCount; i++) {
            if (entriesMinX.at(i) < mbrMinX) {
                mbrMinX = entriesMinX.at(i);
            }
            if (entriesMinY.at(i) < mbrMinY) {
                mbrMinY = entriesMinY.at(i);
            }
            if (entriesMaxX.at(i) > mbrMaxX) {
                mbrMaxX = entriesMaxX.at(i);
            }
            if (entriesMaxY.at(i) > mbrMaxY) {
                mbrMaxY = entriesMaxY.at(i);
            }
        }
    }

    void Node::reorganize(RTree rtree) {
        int countdownIndex = rtree.maxNodeEntries - 1;
        for (int index = 0; index < entryCount; index++) {
            if (ids.at(index) == -1) {
                while (ids.at(countdownIndex) == -1 && countdownIndex > index) {
                    countdownIndex--;
                }
                entriesMinX.at(index) = entriesMinX.at(countdownIndex);
                entriesMinY.at(index) = entriesMinY.at(countdownIndex);
                entriesMaxX.at(index) = entriesMaxX.at(countdownIndex);
                entriesMaxY.at(index) = entriesMaxY.at(countdownIndex);
                ids.at(index) = ids.at(countdownIndex);
                ids.at(countdownIndex) = -1;
            }
        }
    }

    int Node::getEntryCount() {
        return entryCount;
    }

    int Node::getId(int index) {
        if (index < entryCount) {
            return ids.at(index);
        }
        return -1;
    }

    bool Node::isLeaf() {
        return level == 1;
    }

    int Node::getLevel() {
        return level;
    }
}
