#include "RTree.h"
#include <algorithm>
#include <limits>
#include <vector>

#include "Collections/TIntArrayList.h"
#include "Collections/PriorityQueue.h"
#include "Rectangle.h"

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

    void RTree::createNearestNDistanceQueue(Point p, int count, Collections::PriorityQueue distanceQueue, float furthestDistance) {
        //  return immediately if given an invalid "count" parameter
        if (count <= 0) {
            return;
        }

        parents = std::stack<int>();
        parents.push(rootNodeId);

        parentsEntry = std::stack<int>();
        parentsEntry.push(-1);

        Collections::TIntArrayList savedValues = Collections::TIntArrayList();
        float savedPriority = 0;

        // TODO: possible shortcut here - could test for intersection with the
        //       MBR of the root node. If no intersection, return immediately.

        float furthestDistanceSq = furthestDistance * furthestDistance;

        while (parents.size() > 0) {
            Node n = getNode(parents.top());
            int startIndex = parentsEntry.top() + 1;

            if (!n.isLeaf()) {
                // go through every entry in the index node to check
                // if it could contain an entry closer than the farthest entry
                // currently stored.
                bool near = false;
                for (int i = startIndex; i < n.entryCount; i++) {
                    if (SpatialIndex::Rectangle::distanceSq(n.entriesMinX[i], n.entriesMinY[i],
                                           n.entriesMaxX[i], n.entriesMaxY[i],
                                           p.x, p.y) <= furthestDistanceSq) {
                        parents.push(n.ids[i]);
                        parentsEntry.pop();
                        parentsEntry.push(i); // this becomes the start index when the child has been searched
                        parentsEntry.push(-1);
                        near = true;
                        break; // ie go to next iteration of while()
                                           }
                }
                if (near) {
                    continue;
                }
            } else {
                // go through every entry in the leaf to check if
                // it is currently one of the nearest N entries.
                for (int i = 0; i < n.entryCount; i++) {
                    float entryDistanceSq = SpatialIndex::Rectangle::distanceSq(n.entriesMinX[i], n.entriesMinY[i],
                                                             n.entriesMaxX[i], n.entriesMaxY[i],
                                                             p.x, p.y);
                    int entryId = n.ids[i];

                    if (entryDistanceSq <= furthestDistanceSq) {
                        distanceQueue.insert(entryId, entryDistanceSq);

                        while (distanceQueue.size() > count) {
                            // normal case - we can simply remove the lowest priority (highest distance) entry
                            int value = distanceQueue.getValue();
                            float distanceSq = distanceQueue.getPriority();
                            distanceQueue.pop();

                            // rare case - multiple items of the same priority (distance)
                            if (distanceSq == distanceQueue.getPriority()) {
                                savedValues.add(value);
                                savedPriority = distanceSq;
                            } else {
                                savedValues.reset();
                            }
                        }

                        // if the saved values have the same distance as the
                        // next one in the tree, add them back in.
                        if (savedValues.size() > 0 && savedPriority == distanceQueue.getPriority()) {
                            for (int svi = 0; svi < savedValues.size(); svi++) {
                                distanceQueue.insert(savedValues.get(svi), savedPriority);
                            }
                            savedValues.reset();
                        }

                        // narrow the search, if we have already found N items
                        if (distanceQueue.getPriority() < furthestDistanceSq && distanceQueue.size() >= count) {
                            furthestDistanceSq = distanceQueue.getPriority();
                        }
                    }
                }
            }
            parents.pop();
            parentsEntry.pop();
        }
    }

    void RTree::nearestNUnsorted(Point p, Collections::Procedure v, int count, float furthestDistance) {
        // This implementation is designed to give good performance
        // where
        //   o N is high (100+)
        //   o The results do not need to be sorted by distance.
        //
        // Uses a priority queue as the underlying data structure.
        //
        // Note that more than N items will be returned if items N and N+x have the
        // same priority.
        Collections::PriorityQueue distanceQueue = Collections::PriorityQueue(false);
        createNearestNDistanceQueue(p, count, distanceQueue, furthestDistance);

        while (distanceQueue.size() > 0) {
            v.execute(distanceQueue.getValue());
            distanceQueue.pop();
        }
    }

    void RTree::nearestN(Point p, Collections::Procedure v, int count, float furthestDistance) {
        Collections::PriorityQueue distanceQueue = Collections::PriorityQueue(false);
        createNearestNDistanceQueue(p, count, distanceQueue, furthestDistance);
        distanceQueue.setSortOrder(true);

        while (distanceQueue.size() > 0) {
            v.execute(distanceQueue.getValue());
            distanceQueue.pop();
        }
    }

    void RTree::intersects(Rectangle r, Collections::Procedure v) {
        Node rootNode = getNode(rootNodeId);
        intersects(r, v, rootNode);
    }

    void RTree::contains(Rectangle r, Collections::Procedure v) {
        // find all rectangles in the tree that are contained by the passed rectangle
        // written to be non-recursive (should model other searches on this?)
        parents = std::stack<int>();
        parents.push(rootNodeId);

        parentsEntry = std::stack<int>();
        parentsEntry.push(-1);

        // TODO: possible shortcut here - could test for intersection with the
        // MBR of the root node. If no intersection, return immediately.

        while (parents.size() > 0) {
            Node n = getNode(parents.top());
            int startIndex = parentsEntry.top() + 1;

            if (!n.isLeaf()) {
                // go through every entry in the index node to check
                // if it intersects the passed rectangle. If so, it
                // could contain entries that are contained.
                bool intersects = false;
                for (int i = startIndex; i < n.entryCount; i++) {
                    if (Rectangle.intersects(r.minX, r.minY, r.maxX, r.maxY,
                                             n.entriesMinX[i], n.entriesMinY[i], n.entriesMaxX[i], n.entriesMaxY[i])) {
                        parents.push(n.ids[i]);
                        parentsEntry.pop();
                        parentsEntry.push(i); // this becomes the start index when the child has been searched
                        parentsEntry.push(-1);
                        intersects = true;
                        break; // ie go to next iteration of while()
                                             }
                }
                if (intersects) {
                    continue;
                }
            } else {
                // go through every entry in the leaf to check if
                // it is contained by the passed rectangle
                for (int i = 0; i < n.entryCount; i++) {
                    if (Rectangle.contains(r.minX, r.minY, r.maxX, r.maxY,
                                           n.entriesMinX[i], n.entriesMinY[i], n.entriesMaxX[i], n.entriesMaxY[i])) {
                        if (!v.execute(n.ids[i])) {
                            return;
                        }
                                           }
                }
            }
            parents.pop();
            parentsEntry.pop();
        }
    }

    int RTree::treeSize() const {
        return size;
    }

    Rectangle RTree::getBounds() {
        Rectangle bounds;

        Node n = getNode(getRootNodeId());
        if (&n != nullptr && n.entryCount > 0) {
            bounds = Rectangle();
            bounds.minX = n.mbrMinX;
            bounds.minY = n.mbrMinY;
            bounds.maxX = n.mbrMaxX;
            bounds.maxY = n.mbrMaxY;
        }
        return bounds;
    }

}
