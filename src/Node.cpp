#include "Node.h"
#include <cstddef>
#include <iostream>

namespace rtree {

    Node::Node(int id, int level)
    : nodeId(id),
    level(level)
    {
        // Preallocate space for entries if known at construction
        entries.reserve(50);
        ids.reserve(50);
    }

    Node::Node() {
        // Preallocate space for entries if known at construction
        entries.reserve(50);
        ids.reserve(50);
    }

    Node::~Node() = default;

    void Node::addEntry(float minX, float minY, float maxX, float maxY, int id) {
        Entry entry{minX, minY, maxX, maxY, id};

        if (isEmptyOrIncomplete(entry)) {
            std::cerr << "ERROR: Entry is empty or incomplete!" << std::endl;
            std::exit(1);
        }

        if (entries.size() <= entries.capacity()) {
            entries.push_back(entry);
            ids.push_back(id);
        }

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
            recalculateMBR();
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

    void Node::reorganize(int maxNodeEntries) {
        int countdownIndex = maxNodeEntries - 1;
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

    void Node::getNodeEntries() {
        for (int i = 0; i < entryCount; i++) {
            std::cout << "Entry " << i << ": (" << entries[i].minX << ", " << entries[i].minY << ") - ("
                      << entries[i].maxX << ", " << entries[i].maxY << ") with ID: " << ids[i] << std::endl;
        }
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

    bool Node::isEmptyOrIncomplete(Entry& entry) {
        if (entry.id == 0 || entry.minX == 0 || entry.maxX == 0 || entry.minY == 0 || entry.maxY == 0) {
            return true;
        }
        return false;
    }

}
