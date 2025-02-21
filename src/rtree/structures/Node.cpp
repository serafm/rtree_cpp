#include "Node.h"

#include <algorithm>

#include "../builders/RTreeBulkLoad.h"

namespace rtree {

    Node::Node(int id, int level, int capacity)
    : nodeId(id),
    level(level)
    {
        children.reserve(capacity);
        leafs.reserve(capacity);
    }

    Node::Node(int capacity) {
        children.reserve(capacity);
        leafs.reserve(capacity);
    }

    Node::~Node() = default;

    void Node::addChildEntry(std::shared_ptr<Node>& n) {
        children.push_back(n);
        ids.push_back(n->nodeId);

        if (n->mbrMinX < mbrMinX) mbrMinX = n->mbrMinX;
        if (n->mbrMinY < mbrMinY) mbrMinY = n->mbrMinY;
        if (n->mbrMaxX > mbrMaxX) mbrMaxX = n->mbrMaxX;
        if (n->mbrMaxY > mbrMaxY) mbrMaxY = n->mbrMaxY;
    }

    void Node::addLeafEntry(Rectangle rect) {
        leafs.push_back(rect);
        ids.push_back(rect.id);

        if (rect.minX < mbrMinX) mbrMinX = rect.minX;
        if (rect.minY < mbrMinY) mbrMinY = rect.minY;
        if (rect.maxX > mbrMaxX) mbrMaxX = rect.maxX;
        if (rect.maxY > mbrMaxY) mbrMaxY = rect.maxY;
    }
    void Node::sortChildrenByMinX() {
        std::sort(children.begin(), children.end(),
            [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
                return a->mbrMinX < b->mbrMinX;
            });
    }

    void Node::sortLeafsByMinX() {
        std::sort(leafs.begin(), leafs.end(),
            [](const Rectangle& a, const Rectangle& b) {
                return a.minX < b.minX;
            });
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
