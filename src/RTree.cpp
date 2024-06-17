#include "RTree.h"
#include <algorithm>
#include <iostream>
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

    int RTree::getNextNodeId() {
        int nextNodeId = 0;
        if (!deletedNodeIds.empty()) {
            nextNodeId = deletedNodeIds.top();
            deletedNodeIds.pop();
        } else {
            nextNodeId = 1 + highestUsedNodeId++;
        }
        return nextNodeId;
    }

    Node RTree::getNode(int id) {
        return nodeMap.at(id);
    }

    int RTree::getRootNodeId() const {
        return rootNodeId;
    }

    Node RTree::splitNode(Node n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId) {
        // [Pick first entry for each group] Apply algorithm pickSeeds to
        // choose two entries to be the first elements of the groups. Assign
        // each to a group.

        float unionMinX = std::min(n.mbrMinX, newRectMinX);
        float unionMinY = std::min(n.mbrMinY, newRectMinY);
        float unionMaxX = std::max(n.mbrMaxX, newRectMaxX);
        float unionMaxY = std::max(n.mbrMaxY, newRectMaxY);

        float initialArea = (unionMaxX - unionMinX) * (unionMaxY - unionMinY);

        // Copying vector by assign function
        entryStatus.assign(initialEntryStatus.begin(), initialEntryStatus.end());

        Node newNode = Node(getNextNodeId(), n.level, maxNodeEntries);

        nodeMap.insert({newNode.nodeId, newNode});

        pickSeeds(n, newRectMinX, newRectMinY, newRectMaxX, newRectMaxY, newId, newNode);

        // [Check if done] If all entries have been assigned, stop. If one
        // group has so few entries that all the rest must be assigned to it in
        // order for it to have the minimum number m, assign them and stop.
        while (n.entryCount + newNode.entryCount < maxNodeEntries + 1) {
            if (maxNodeEntries + 1 - newNode.entryCount == minNodeEntries) {
                // assign all remaining entries to original node
                for (int i = 0; i < maxNodeEntries; i++) {
                    if (entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {
                        entryStatus[i] = ENTRY_STATUS_ASSIGNED;

                        if (n.entriesMinX[i] < n.mbrMinX) n.mbrMinX = n.entriesMinX[i];
                        if (n.entriesMinY[i] < n.mbrMinY) n.mbrMinY = n.entriesMinY[i];
                        if (n.entriesMaxX[i] > n.mbrMaxX) n.mbrMaxX = n.entriesMaxX[i];
                        if (n.entriesMaxY[i] > n.mbrMaxY) n.mbrMaxY = n.entriesMaxY[i];

                        n.entryCount++;
                    }
                }
                break;
            }
            if (maxNodeEntries + 1 - n.entryCount == minNodeEntries) {
                // assign all remaining entries to new node
                for (int i = 0; i < maxNodeEntries; i++) {
                    if (entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {
                        entryStatus[i] = ENTRY_STATUS_ASSIGNED;
                        newNode.addEntry(n.entriesMinX[i], n.entriesMinY[i], n.entriesMaxX[i], n.entriesMaxY[i], n.ids[i]);
                        n.ids[i] = -1; // an id of -1 indicates the entry is not in use
                    }
                }
                break;
            }

            // [Select entry to assign] Invoke algorithm pickNext to choose the
            // next entry to assign. Add it to the group whose covering rectangle
            // will have to be enlarged least to accommodate it. Resolve ties
            // by adding the entry to the group with smaller area, then to the
            // the one with fewer entries, then to either. Repeat from S2
            pickNext(n, newNode);
        }

        n.reorganize(this);

        // check that the MBR stored for each node is correct.
        if (INTERNAL_CONSISTENCY_CHECKING) {
            Rectangle nMBR = Rectangle(n.mbrMinX, n.mbrMinY, n.mbrMaxX, n.mbrMaxY);
            if (!nMBR.equals(calculateMBR(n))) {
                std::cerr << "Error: splitNode old node MBR wrong" << std::endl;
            }
            Rectangle newNodeMBR = Rectangle(newNode.mbrMinX, newNode.mbrMinY, newNode.mbrMaxX, newNode.mbrMaxY);
            if (!newNodeMBR.equals(calculateMBR(newNode))) {
                std::cerr << "Error: splitNode new node MBR wrong" << std::endl;
            }
        }

        float newArea = Rectangle.area(n.mbrMinX, n.mbrMinY, n.mbrMaxX, n.mbrMaxY) +
                        Rectangle.area(newNode.mbrMinX, newNode.mbrMinY, newNode.mbrMaxX, newNode.mbrMaxY);
        float percentageIncrease = (100 * (newArea - initialArea)) / initialArea;
            std::cout << "Node " << n.nodeId << " split. New area increased by " << percentageIncrease << "%" << std::endl;

        return newNode;
    }

    void RTree::pickSeeds(Node n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId, Node newNode) {
        // Find extreme rectangles along all dimension. Along each dimension,
        // find the entry whose rectangle has the highest low side, and the one
        // with the lowest high side. Record the separation.
        float maxNormalizedSeparation = -1; // initialize to -1 so that even overlapping rectangles will be considered for the seeds
        int highestLowIndex = -1;
        int lowestHighIndex = -1;

        // for the purposes of picking seeds, take the MBR of the node to include
        // the new rectangle aswell.
        if (newRectMinX < n.mbrMinX) n.mbrMinX = newRectMinX;
        if (newRectMinY < n.mbrMinY) n.mbrMinY = newRectMinY;
        if (newRectMaxX > n.mbrMaxX) n.mbrMaxX = newRectMaxX;
        if (newRectMaxY > n.mbrMaxY) n.mbrMaxY = newRectMaxY;

        float mbrLenX = n.mbrMaxX - n.mbrMinX;
        float mbrLenY = n.mbrMaxY - n.mbrMinY;

        float tempHighestLow = newRectMinX;
        int tempHighestLowIndex = -1; // -1 indicates the new rectangle is the seed

        float tempLowestHigh = newRectMaxX;
        int tempLowestHighIndex = -1; // -1 indicates the new rectangle is the seed

        for (int i = 0; i < n.entryCount; i++) {
          float tempLow = n.entriesMinX[i];
          if (tempLow >= tempHighestLow) {
             tempHighestLow = tempLow;
             tempHighestLowIndex = i;
          } else {  // ensure that the same index cannot be both lowestHigh and highestLow
            float tempHigh = n.entriesMaxX[i];
            if (tempHigh <= tempLowestHigh) {
              tempLowestHigh = tempHigh;
              tempLowestHighIndex = i;
            }
          }

          // PS2 [Adjust for shape of the rectangle cluster] Normalize the separations
          // by dividing by the widths of the entire set along the corresponding
          // dimension
          float normalizedSeparation = mbrLenX == 0 ? 1 : (tempHighestLow - tempLowestHigh) / mbrLenX;

          // PS3 [Select the most extreme pair] Choose the pair with the greatest
          // normalized separation along any dimension.
          // Note that if negative it means the rectangles overlapped. However still include
          // overlapping rectangles if that is the only choice available.
          if (normalizedSeparation >= maxNormalizedSeparation) {
            highestLowIndex = tempHighestLowIndex;
            lowestHighIndex = tempLowestHighIndex;
            maxNormalizedSeparation = normalizedSeparation;
          }
        }

        // Repeat for the Y dimension
        tempHighestLow = newRectMinY;
        tempHighestLowIndex = -1; // -1 indicates the new rectangle is the seed

        tempLowestHigh = newRectMaxY;
        tempLowestHighIndex = -1; // -1 indicates the new rectangle is the seed

        for (int i = 0; i < n.entryCount; i++) {
          float tempLow = n.entriesMinY[i];
          if (tempLow >= tempHighestLow) {
             tempHighestLow = tempLow;
             tempHighestLowIndex = i;
          } else {  // ensure that the same index cannot be both lowestHigh and highestLow
            float tempHigh = n.entriesMaxY[i];
            if (tempHigh <= tempLowestHigh) {
              tempLowestHigh = tempHigh;
              tempLowestHighIndex = i;
            }
          }

          // PS2 [Adjust for shape of the rectangle cluster] Normalize the separations
          // by dividing by the widths of the entire set along the corresponding
          // dimension
          float normalizedSeparation = mbrLenY == 0 ? 1 : (tempHighestLow - tempLowestHigh) / mbrLenY;

          // PS3 [Select the most extreme pair] Choose the pair with the greatest
          // normalized separation along any dimension.
          // Note that if negative it means the rectangles overlapped. However still include
          // overlapping rectangles if that is the only choice available.
          if (normalizedSeparation >= maxNormalizedSeparation) {
            highestLowIndex = tempHighestLowIndex;
            lowestHighIndex = tempLowestHighIndex;
            maxNormalizedSeparation = normalizedSeparation;
          }
        }

        // At this point it is possible that the new rectangle is both highestLow and lowestHigh.
        // This can happen if all rectangles in the node overlap the new rectangle.
        // Resolve this by declaring that the highestLowIndex is the lowest Y and,
        // the lowestHighIndex is the largest X (but always a different rectangle)
        if (highestLowIndex == lowestHighIndex) {
          highestLowIndex = -1;
          float tempMinY = newRectMinY;
          lowestHighIndex = 0;
          float tempMaxX = n.entriesMaxX[0];

          for (int i = 1; i < n.entryCount; i++) {
            if (n.entriesMinY[i] < tempMinY) {
              tempMinY = n.entriesMinY[i];
              highestLowIndex = i;
            }
            else if (n.entriesMaxX[i] > tempMaxX) {
              tempMaxX = n.entriesMaxX[i];
              lowestHighIndex = i;
            }
          }
        }

        // highestLowIndex is the seed for the new node.
        if (highestLowIndex == -1) {
          newNode.addEntry(newRectMinX, newRectMinY, newRectMaxX, newRectMaxY, newId);
        } else {
          newNode.addEntry(n.entriesMinX[highestLowIndex], n.entriesMinY[highestLowIndex],
                           n.entriesMaxX[highestLowIndex], n.entriesMaxY[highestLowIndex],
                           n.ids[highestLowIndex]);
          n.ids[highestLowIndex] = -1;

          // move the new rectangle into the space vacated by the seed for the new node
          n.entriesMinX[highestLowIndex] = newRectMinX;
          n.entriesMinY[highestLowIndex] = newRectMinY;
          n.entriesMaxX[highestLowIndex] = newRectMaxX;
          n.entriesMaxY[highestLowIndex] = newRectMaxY;

          n.ids[highestLowIndex] = newId;
        }

        // lowestHighIndex is the seed for the original node.
        if (lowestHighIndex == -1) {
          lowestHighIndex = highestLowIndex;
        }

        entryStatus[lowestHighIndex] = ENTRY_STATUS_ASSIGNED;
        n.entryCount = 1;
        n.mbrMinX = n.entriesMinX[lowestHighIndex];
        n.mbrMinY = n.entriesMinY[lowestHighIndex];
        n.mbrMaxX = n.entriesMaxX[lowestHighIndex];
        n.mbrMaxY = n.entriesMaxY[lowestHighIndex];
    }

    int RTree::pickNext(Node n, Node newNode) {
        float maxDifference = -std::numeric_limits<float>::max();
        int next = 0;
        int nextGroup = 0;

        maxDifference = -std::numeric_limits<float>::max();

        for (int i = 0; i < maxNodeEntries; i++) {
          if (entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {

            if (n.ids[i] == -1) {
             std::cerr << "Error: Node " << n.nodeId << ", entry " + i << " is null" << std::endl;
            }

            float nIncrease = Rectangle.enlargement(n.mbrMinX, n.mbrMinY, n.mbrMaxX, n.mbrMaxY,
                                                    n.entriesMinX[i], n.entriesMinY[i], n.entriesMaxX[i], n.entriesMaxY[i]);
            float newNodeIncrease = Rectangle.enlargement(newNode.mbrMinX, newNode.mbrMinY, newNode.mbrMaxX, newNode.mbrMaxY,
                                                          n.entriesMinX[i], n.entriesMinY[i], n.entriesMaxX[i], n.entriesMaxY[i]);

            float difference = std::abs(nIncrease - newNodeIncrease);

            if (difference > maxDifference) {
              next = i;

              if (nIncrease < newNodeIncrease) {
                nextGroup = 0;
              } else if (newNodeIncrease < nIncrease) {
                nextGroup = 1;
              } else if (Rectangle.area(n.mbrMinX, n.mbrMinY, n.mbrMaxX, n.mbrMaxY) < Rectangle.area(newNode.mbrMinX, newNode.mbrMinY, newNode.mbrMaxX, newNode.mbrMaxY)) {
                nextGroup = 0;
              } else if (Rectangle.area(newNode.mbrMinX, newNode.mbrMinY, newNode.mbrMaxX, newNode.mbrMaxY) < Rectangle.area(n.mbrMinX, n.mbrMinY, n.mbrMaxX, n.mbrMaxY)) {
                nextGroup = 1;
              } else if (newNode.entryCount < maxNodeEntries / 2) {
                nextGroup = 0;
              } else {
                nextGroup = 1;
              }
              maxDifference = difference;
            }
          }
        }

        entryStatus[next] = ENTRY_STATUS_ASSIGNED;

        if (nextGroup == 0) {
          if (n.entriesMinX[next] < n.mbrMinX) n.mbrMinX = n.entriesMinX[next];
          if (n.entriesMinY[next] < n.mbrMinY) n.mbrMinY = n.entriesMinY[next];
          if (n.entriesMaxX[next] > n.mbrMaxX) n.mbrMaxX = n.entriesMaxX[next];
          if (n.entriesMaxY[next] > n.mbrMaxY) n.mbrMaxY = n.entriesMaxY[next];
          n.entryCount++;
        } else {
          // move to new node.
          newNode.addEntry(n.entriesMinX[next], n.entriesMinY[next], n.entriesMaxX[next], n.entriesMaxY[next], n.ids[next]);
          n.ids[next] = -1;
        }

        return next;
    }

    float RTree::nearest(Point p, Node n, float furthestDistanceSq, Collections::TIntArrayList nearestIds) {
        for (int i = 0; i < n.entryCount; i++) {
            float tempDistanceSq = Rectangle.distanceSq(n.entriesMinX[i], n.entriesMinY[i], n.entriesMaxX[i], n.entriesMaxY[i], p.x, p.y);
            if (n.isLeaf()) { // for leaves, the distance is an actual nearest distance
                if (tempDistanceSq < furthestDistanceSq) {
                    furthestDistanceSq = tempDistanceSq;
                    nearestIds.reset();
                }
                if (tempDistanceSq <= furthestDistanceSq) {
                    nearestIds.add(n.ids[i]);
                }
            } else { // for index nodes, only go into them if they potentially could have
                // a rectangle nearer than actualNearest
                if (tempDistanceSq <= furthestDistanceSq) {
                    // search the child node
                    furthestDistanceSq = nearest(p, getNode(n.ids[i]), furthestDistanceSq, nearestIds);
                }
            }
        }
        return furthestDistanceSq;
    }

    bool RTree::intersects(Rectangle r, Collections::Procedure v, Node n) {
        for (int i = 0; i < n.entryCount; i++) {
            if (Rectangle.intersects(r.minX, r.minY, r.maxX, r.maxY, n.entriesMinX[i], n.entriesMinY[i], n.entriesMaxX[i], n.entriesMaxY[i])) {
                if (n.isLeaf()) {
                    if (!v.execute(n.ids[i])) {
                        return false;
                    }
                } else {
                    Node childNode = getNode(n.ids[i]);
                    if (!intersects(r, v, childNode)) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    void RTree::condenseTree(Node l) {
        // CT1 [Initialize] Set n=l. Set the list of eliminated
        // nodes to be empty.
        Node n = l;
        Node parent;
        int parentEntry = 0;

        std::stack<int> eliminatedNodeIds;

        // CT2 [Find parent entry] If N is the root, go to CT6. Otherwise
        // let P be the parent of N, and let En be N's entry in P
        while (n.level != treeHeight) {
            parent = getNode(parents.top());
            parents.pop();
            parentEntry = parentsEntry.top();
            parentsEntry.pop();

            // CT3 [Eliminiate under-full node] If N has too few entries,
            // delete En from P and add N to the list of eliminated nodes
            if (n.entryCount < minNodeEntries) {
                parent.deleteEntry(parentEntry);
                eliminatedNodeIds.push(n.nodeId);
            } else {
                // CT4 [Adjust covering rectangle] If N has not been eliminated,
                // adjust EnI to tightly contain all entries in N
                if (n.mbrMinX != parent.entriesMinX[parentEntry] ||
                    n.mbrMinY != parent.entriesMinY[parentEntry] ||
                    n.mbrMaxX != parent.entriesMaxX[parentEntry] ||
                    n.mbrMaxY != parent.entriesMaxY[parentEntry]) {
                    float deletedMinX = parent.entriesMinX[parentEntry];
                    float deletedMinY = parent.entriesMinY[parentEntry];
                    float deletedMaxX = parent.entriesMaxX[parentEntry];
                    float deletedMaxY = parent.entriesMaxY[parentEntry];
                    parent.entriesMinX[parentEntry] = n.mbrMinX;
                    parent.entriesMinY[parentEntry] = n.mbrMinY;
                    parent.entriesMaxX[parentEntry] = n.mbrMaxX;
                    parent.entriesMaxY[parentEntry] = n.mbrMaxY;
                    parent.recalculateMBRIfInfluencedBy(deletedMinX, deletedMinY, deletedMaxX, deletedMaxY);
                    }
            }
            // CT5 [Move up one level in tree] Set N=P and repeat from CT2
            n = parent;
        }

        // CT6 [Reinsert orphaned entries] Reinsert all entries of nodes in set Q.
        // Entries from eliminated leaf nodes are reinserted in tree leaves as in
        // Insert(), but entries from higher level nodes must be placed higher in
        // the tree, so that leaves of their dependent subtrees will be on the same
        // level as leaves of the main tree
        while (!eliminatedNodeIds.empty()) {
            Node e = getNode(eliminatedNodeIds.top());
            eliminatedNodeIds.pop();
            for (int j = 0; j < e.entryCount; j++) {
                add(e.entriesMinX[j], e.entriesMinY[j], e.entriesMaxX[j], e.entriesMaxY[j], e.ids[j], e.level);
                e.ids[j] = -1;
            }
            e.entryCount = 0;
            deletedNodeIds.push(e.nodeId);
        }
    }

    Node RTree::chooseNode(float minX, float minY, float maxX, float maxY, int level) {
        // CL1 [Initialize] Set N to be the root node
        Node n = getNode(rootNodeId);
        parents = std::stack<int>();
        parentsEntry = std::stack<int>();

        // CL2 [Leaf check] If N is a leaf, return N
        while (true) {
            if (&n == nullptr) {
                std::cerr << "Could not get root node (" << rootNodeId << ")" << std::endl;
            }

            if (n.level == level) {
                return n;
            }

            // CL3 [Choose subtree] If N is not at the desired level, let F be the entry in N
            // whose rectangle FI needs least enlargement to include EI. Resolve
            // ties by choosing the entry with the rectangle of smaller area.
            float leastEnlargement = Rectangle.enlargement(n.entriesMinX[0], n.entriesMinY[0], n.entriesMaxX[0], n.entriesMaxY[0],
                                                           minX, minY, maxX, maxY);
            int index = 0; // index of rectangle in subtree
            for (int i = 1; i < n.entryCount; i++) {
                float tempMinX = n.entriesMinX[i];
                float tempMinY = n.entriesMinY[i];
                float tempMaxX = n.entriesMaxX[i];
                float tempMaxY = n.entriesMaxY[i];
                float tempEnlargement = Rectangle.enlargement(tempMinX, tempMinY, tempMaxX, tempMaxY,
                                                              minX, minY, maxX, maxY);
                if ((tempEnlargement < leastEnlargement) ||
                    ((tempEnlargement == leastEnlargement) &&
                     (Rectangle.area(tempMinX, tempMinY, tempMaxX, tempMaxY) <
                      Rectangle.area(n.entriesMinX[index], n.entriesMinY[index], n.entriesMaxX[index], n.entriesMaxY[index])))) {
                    index = i;
                    leastEnlargement = tempEnlargement;
                      }
            }

            parents.push(n.nodeId);
            parentsEntry.push(index);

            // CL4 [Descend until a leaf is reached] Set N to be the child node
            // pointed to by Fp and repeat from CL2
            n = getNode(n.ids[index]);
        }
    }

    Node RTree::adjustTree(Node n, Node nn) {

        while (n.level != treeHeight) {
            Node parent = getNode(parents.top());
            parents.pop();
            int entry = parentsEntry.top();
            parentsEntry.pop();

            if (parent.ids[entry] != n.nodeId) {
                std::cerr << "Error: entry " << entry << "in node " << parent.nodeId << " should point to node "
                << n.nodeId << "; actually points to node " << parent.ids[entry] << std::endl;
            }

            if (parent.entriesMinX[entry] != n.mbrMinX || parent.entriesMinY[entry] != n.mbrMinY || parent.entriesMaxX[entry] != n.mbrMaxX || parent.entriesMaxY[entry] != n.mbrMaxY) {
                parent.entriesMinX[entry] = n.mbrMinX;
                parent.entriesMinY[entry] = n.mbrMinY;
                parent.entriesMaxX[entry] = n.mbrMaxX;
                parent.entriesMaxY[entry] = n.mbrMaxY;

                parent.recalculateMBR();
            }

            Node newNode;
            if (&nn != nullptr) {
                if (parent.entryCount < maxNodeEntries) {
                    parent.addEntry(nn.mbrMinX, nn.mbrMinY, nn.mbrMaxX, nn.mbrMaxY, nn.nodeId);
                } else {
                    newNode = splitNode(parent, nn.mbrMinX, nn.mbrMinY, nn.mbrMaxX, nn.mbrMaxY, nn.nodeId);
                }
            }

            n = parent;
            nn = newNode;

            parent = Node();
            newNode = Node();
        }

        return nn;
    }

    bool RTree::checkConsistency() {
        return checkConsistency(rootNodeId, treeHeight, Rectangle());
    }

    bool RTree::checkConsistency(int nodeId, int expectedLevel, Rectangle expectedMBR) {
        Node n = getNode(nodeId);

        if (&n == nullptr) {
            std::cerr << "Error: Could not read node " << nodeId << std::endl;
            return false;
        }

        // if tree is empty, then there should be exactly one node, at level 1
        // TODO: also check the MBR is as for a new node
        if (nodeId == rootNodeId && size() == 0) {
            if (n.level != 1) {
                std::cerr << "Error: tree is empty but root node is not at level 1" << std::endl;
                return false;
            }
        }

        if (n.level != expectedLevel) {
            std::cerr << "Error: Node " + nodeId << ", expected level " << expectedLevel << ", actual level " << n.level << std::endl;
            return false;
        }

        Rectangle calculatedMBR = calculateMBR(n);
        Rectangle actualMBR = Rectangle();
        actualMBR.minX = n.mbrMinX;
        actualMBR.minY = n.mbrMinY;
        actualMBR.maxX = n.mbrMaxX;
        actualMBR.maxY = n.mbrMaxY;

        if (!actualMBR.equals(calculatedMBR)) {
            std::cerr << "Error: Node " << nodeId << ", calculated MBR does not equal stored MBR" << std::endl;
            if (actualMBR.minX != n.mbrMinX) {
                std::cerr << "  actualMinX=" << actualMBR.minX << ", calc=" << calculatedMBR.minX << std::endl;
            }
            if (actualMBR.minY != n.mbrMinY) {
                std::cerr << "  actualMinY=" << actualMBR.minY << ", calc=" << calculatedMBR.minY << std::endl;
            }
            if (actualMBR.maxX != n.mbrMaxX) {
                std::cerr << "  actualMaxX=" << actualMBR.maxX << ", calc=" << calculatedMBR.maxX << std::endl;
            }
            if (actualMBR.maxY != n.mbrMaxY) {
                std::cerr << "  actualMaxY=" << actualMBR.maxY << ", calc=" << calculatedMBR.maxY << std::endl;
            }

            return false;
        }

        if (expectedMBR != NULL && !actualMBR.equals(expectedMBR)) {
            std::cerr << "Error: Node " << nodeId << ", expected MBR (from parent) does not equal stored MBR" << std::endl;
            return false;
        }

        // Check for corruption where a parent entry is the same object as the child MBR
        if (expectedMBR != NULL && actualMBR.sameObject(expectedMBR)) {
            std::cerr << "Error: Node " << nodeId << " MBR using same rectangle object as parent's entry" << std::endl;
            return false;
        }

        for (int i = 0; i < n.entryCount; i++) {
            if (n.ids[i] == -1) {
                std::cerr << "Error: Node " << nodeId << ", Entry " << i << " is null" << std::endl;
                return false;
            }

            if (n.level > 1) { // if not a leaf
                if (!checkConsistency(n.ids[i], n.level - 1, Rectangle(n.entriesMinX[i], n.entriesMinY[i], n.entriesMaxX[i], n.entriesMaxY[i]))) {
                    return false;
                }
            }
        }
        return true;

    }

    Rectangle RTree::calculateMBR(Node n) {
        Rectangle mbr = Rectangle();

        for (int i = 0; i < n.entryCount; i++) {
            if (n.entriesMinX[i] < mbr.minX) mbr.minX = n.entriesMinX[i];
            if (n.entriesMinY[i] < mbr.minY) mbr.minY = n.entriesMinY[i];
            if (n.entriesMaxX[i] > mbr.maxX) mbr.maxX = n.entriesMaxX[i];
            if (n.entriesMaxY[i] > mbr.maxY) mbr.maxY = n.entriesMaxY[i];
        }

        return mbr;
    }


}
