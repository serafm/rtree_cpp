#include "Node.h"
#include <cstddef>
#include "RTree.h"

namespace SpatialIndex {

    class RTree;

    Node::Node(int nodeId, int level, int maxNodeEntries) :
        nodeId(nodeId), level(level), entries(maxNodeEntries) {
        // Preallocate space for entries if known at construction
        entries.reserve(maxNodeEntries);
    }

    Node::~Node() = default;

    void Node::addEntry(float minX, float minY, float maxX, float maxY, int id) {
        Entry entry{minX, minY, maxX, maxY, id};
        entries.insert(entries.begin() + entryCount, entry);

        // Efficiently update MBR only if necessary
        if (entryCount == 0) {
            mbrMinX = minX;
            mbrMinY = minY;
            mbrMaxX = maxX;
            mbrMaxY = maxY;
        } else {
            mbrMinX = std::min(mbrMinX, minX);
            mbrMinY = std::min(mbrMinY, minY);
            mbrMaxX = std::max(mbrMaxX, maxX);
            mbrMaxY = std::max(mbrMaxY, maxY);
        }
        entryCount++;
    }

    int Node::findEntry(float minX, float minY, float maxX, float maxY, int id) {
        for (Entry entry : entries) {
            if (id == entry.id && entry.minX == minX && entry.minY == minY && entry.maxX == maxX && entry.maxY == maxY) {
                std::cout << "Entry found with ID: " << entry.id << std::endl;
                return 0;
            }
        }
        return -1;
    }

    void Node::deleteEntry(int index) {
        int lastIndex = entryCount - 1;
        if (index != lastIndex) {
            // Swap the entry to delete with the last entry
            std::swap(entries[index], entries[lastIndex]);
        }

        // Remove the last entry (previously swapped or originally to be deleted)
        Entry deletedEntry = entries.back();
        entries.pop_back();
        entryCount--;

        // Check if deleted entry influenced MBR and recalculate if necessary
        recalculateMBRIfInfluencedBy(deletedEntry.minX, deletedEntry.minY, deletedEntry.maxX, deletedEntry.maxY);
    }

    void Node::recalculateMBRIfInfluencedBy(float deletedMinX, float deletedMinY, float deletedMaxX, float deletedMaxY) {
        if (mbrMinX == deletedMinX || mbrMinY == deletedMinY || mbrMaxX == deletedMaxX || mbrMaxY == deletedMaxY) {
            mbrMaxX = mbrMaxY = std::numeric_limits<float>::lowest();
            mbrMinX = mbrMinY = std::numeric_limits<float>::max();
            for (Entry entry: entries) {
                mbrMaxX = std::max(mbrMaxX, entry.maxX);
                mbrMaxY = std::max(mbrMaxY, entry.maxY);
                mbrMinX = std::min(mbrMinX, entry.minX);
                mbrMinY = std::min(mbrMinY, entry.minY);
            }
        }
    }

    void Node::recalculateMBR() {
        mbrMaxX = mbrMaxY = std::numeric_limits<float>::lowest();
        mbrMinX = mbrMinY = std::numeric_limits<float>::max();
        for (Entry entry: entries) {
            mbrMaxX = std::max(mbrMaxX, entry.maxX);
            mbrMaxY = std::max(mbrMaxY, entry.maxY);
            mbrMinX = std::min(mbrMinX, entry.minX);
            mbrMinY = std::min(mbrMinY, entry.minY);
        }
    }

    void Node::reorganize(RTree* rtree) {
        int countdownIndex = rtree->maxNodeEntries - 1;
        for (int index = 0; index < entryCount; index++) {
            if (ids[index] == -1) {
                while (ids[countdownIndex] == -1 && countdownIndex > index) {
                    countdownIndex--;
                }
                entries[index].minX = entries[countdownIndex].minX;
                entries[index].minY = entries[countdownIndex].minY;
                entries[index].maxX = entries[countdownIndex].maxX;
                entries[index].maxY = entries[countdownIndex].maxY;
                ids[index] = ids[countdownIndex];
                ids[countdownIndex] = -1;
            }
        }
    }

    int Node::getEntryCount() const {
        return entryCount;
    }

    int Node::getId(int index) const {
        if (index < entryCount) {
            return ids[index];
        }
        return -1;
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
