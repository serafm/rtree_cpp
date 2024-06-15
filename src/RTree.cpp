#include "RTree.h"
#include <algorithm>
#include <limits>
#include <queue>
#include <vector>
#include <utility>

#include "Collections/TIntArrayList.h"

namespace SpatialIndex {

    RTree::RTree() = default;

    void RTree::init() {
        maxNodeEntries = DEFAULT_MAX_NODE_ENTRIES;
        minNodeEntries = DEFAULT_MIN_NODE_ENTRIES;
        entryStatus = std::vector<int8_t>(maxNodeEntries);
        initialEntryStatus = std::vector<int8_t>(maxNodeEntries);

        for (int i = 0; i < maxNodeEntries; i++) {
            initialEntryStatus[i] = ENTRY_STATUS_UNASSIGNED;
        }

        Node root = Node(rootNodeId, 1, maxNodeEntries);
        nodeMap.insert({rootNodeId, root});
    }

    void RTree::add(Rectangle r, int id) {
        add(r.minX, r.minY, r.maxX, r.maxY, id, 1);
        size++;

        if (INTERNAL_CONSISTENCY_CHECKING) {
            checkConsistency();
        }
    }

    void RTree::add(float minX, float minY, float maxX, float maxY, int id, int level) {
        Node n = chooseNode(minX, minY, maxX, maxY, level);
        Node newLeaf;

        if (n.entryCount < maxNodeEntries) {
            n.addEntry(minX, minY, maxX, maxY, id);
        } else {
            newLeaf = splitNode(n, minX, minY, maxX, maxY, id);
        }

        Node newNode = adjustTree(n, newLeaf);

        if ( &newNode != nullptr) {
            int oldRootNodeId = rootNodeId;
            Node oldRoot = getNode(oldRootNodeId);

            rootNodeId = getNextNodeId();
            treeHeight++;
            Node root = Node(rootNodeId, treeHeight, maxNodeEntries);
            root.addEntry(newNode.mbrMinX, newNode.mbrMinY, newNode.mbrMaxX, newNode.mbrMaxY, newNode.nodeId);
            root.addEntry(oldRoot.mbrMinX, oldRoot.mbrMinY, oldRoot.mbrMaxX, oldRoot.mbrMaxY, oldRoot.nodeId)
            nodeMap.insert({rootNodeId, root});
        }
    }

    bool RTree::del(Rectangle r, int id) {
        parents = std::stack<int>();
        parents.push(rootNodeId);

        parentsEntry = std::stack<int>();
        parentsEntry.push(-1);
        Node n;
        int foundIndex = -1;

        while (foundIndex == -1 && parents.size() > 0) {
            n = getNode(parents.top());
            int startIndex = parentsEntry.top() + 1;

            if (!n.isLeaf()) {
                bool contains = false;
                for (int i = startIndex; i < n.entryCount; i++) {
                    if (r.contains(n.entriesMinX[i], n.entriesMinY[i], n.entriesMaxX[i], n.entriesMaxY[i],
                     r.minX, r.minY, r.maxX, r.maxY)) {
                        parents.push(n.ids[i]);
                        parentsEntry.pop();
                        parentsEntry.push(i); // this becomes the start index when the child has been searched
                        parentsEntry.push(-1);
                        contains = true;
                        break; // ie go to next iteration of while()
                     }
                }
                if (contains) {
                    continue;
                }
            } else {
                foundIndex = n.findEntry(r.minX, r.minY, r.maxX, r.maxY, id);
            }

            parents.pop();
            parentsEntry.pop();
        }

        if (foundIndex != -1 && &n != nullptr) {
            n.deleteEntry(foundIndex);
            condenseTree(n);
            size--;
        }

        Node root = getNode(rootNodeId);
        while (root.entryCount == 1 && treeHeight > 1) {
            deletedNodeIds.push(rootNodeId);
            root.entryCount = 0;
            rootNodeId = root.ids[0];
            treeHeight--;
            root = getNode(rootNodeId);
        }

        // if the tree is now empty, then set the MBR of the root node back to it's original state
        // (this is only needed when the tree is empty, as this is the only state where an empty node
        // is not eliminated)
        if (size == 0) {
            root.mbrMinX = std::numeric_limits<float>::max();
            root.mbrMinY = std::numeric_limits<float>::max();
            root.mbrMaxX = -std::numeric_limits<float>::max();
            root.mbrMaxY = -std::numeric_limits<float>::max();
        }

        if (INTERNAL_CONSISTENCY_CHECKING) {
            checkConsistency();
        }

        return (foundIndex != -1);
    }

    void RTree::nearest(Point p, Collections::Procedure v, float furthestDistance) {
        Node rootNode = getNode(rootNodeId);

        float furthestDistanceSq = furthestDistance * furthestDistance;
        Collections::TIntArrayList nearestIds = Collections::TIntArrayList();
        nearest(p, rootNode, furthestDistanceSq, nearestIds);

        nearestIds.forEach(v);
        nearestIds.reset();
    }












}
