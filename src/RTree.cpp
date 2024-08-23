#include "RTree.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

#include "Collections/IntVector.h"
#include "Collections/PriorityQueue.h"
#include "Rectangle.h"

using namespace std;

namespace rtree {

    RTree::RTree()
    {
        maxNodeEntries = DEFAULT_MAX_NODE_ENTRIES;
        minNodeEntries = DEFAULT_MIN_NODE_ENTRIES;
        entryStatus.assign(maxNodeEntries, ENTRY_STATUS_UNASSIGNED);
        initialEntryStatus = entryStatus;
        root = Node(rootNodeId, 1);
        nodeMap.insert({rootNodeId, &root});
    }

    void RTree::add(Rectangle& r, uint32_t id)
    {
        cout << "Adding rectangle with ID: " << id << endl;
        add(r.minX, r.minY, r.maxX, r.maxY, id, 1);
        size++;
    }

    void RTree::add(float minX, float minY, float maxX, float maxY, uint32_t id, int level) {
        Node& n = chooseNode(minX, minY, maxX, maxY, level, rootNodeId);
        Node newLeaf;

        // Check if node has space for new entry
        if (n.entryCount < maxNodeEntries) {
            n.addEntry(minX, minY, maxX, maxY, id);
            //nodeMap.insert({n.getNodeId(), &n});
        } else {
            newLeaf = splitNode(n, minX, minY, maxX, maxY, id);
        }

        Node& newNode = adjustTree(n, newLeaf);

        if (!newNode.isEmpty()) {
            uint32_t oldRootNodeId = rootNodeId;
            Node& oldRoot = getNode(oldRootNodeId);

            rootNodeId = getNextNodeId();
            treeHeight++;
            Node* root = new Node(rootNodeId, treeHeight);

            root->addEntry(newNode.mbrMinX, newNode.mbrMinY, newNode.mbrMaxX, newNode.mbrMaxY, newNode.nodeId);
            root->addEntry(oldRoot.mbrMinX, oldRoot.mbrMinY, oldRoot.mbrMaxX, oldRoot.mbrMaxY, oldRoot.nodeId);

            nodeMap.insert({root->getNodeId(), root});
        }
    }

    bool RTree::del(Rectangle& r, uint32_t id) {
        parents = std::stack<uint32_t>();
        parents.push(rootNodeId);

        parentsEntry = std::stack<uint32_t>();
        parentsEntry.push(-1);
        Node n;
        int foundIndex = -1;

        while (foundIndex == -1 && !parents.empty()) {
            n = getNode(parents.top());
            uint32_t startIndex = parentsEntry.top() + 1;

            if (!n.isLeaf()) {
                cout << "Searching node " << n.nodeId << ", from index " << startIndex << endl;
                bool contains = false;
                for (uint32_t i = startIndex; i < n.entryCount; i++) {
                    if (Rectangle::contains(n.entries[i].minX, n.entries[i].minY, n.entries[i].maxX, n.entries[i].maxY,
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

        if (foundIndex != -1 && !n.isEmpty() ) {
            n.deleteEntry(foundIndex);
            condenseTree(n);
            size--;
        }

        Node& root = getNode(rootNodeId);
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
        return (foundIndex != -1);
    }

    void RTree::nearest(Point& p, float furthestDistance) {
        auto rootNode = getNode(rootNodeId);

        float furthestDistanceSq = furthestDistance * furthestDistance;
        auto nearestIds = Collections::IntVector();

        std::cout << "Nearest rectangles to point (" << p.x << "," << p.y << ") are: " << std::endl;

        auto printNearest = [&]() {
            for (int i = 0; i < nearestIds.size(); i++) {
                std::cout << "Rectangle with ID: " << nearestIds.get(i) << std::endl;
            }
        };

        nearest(p, rootNode, furthestDistanceSq, nearestIds);

        printNearest();

        nearestIds.reset();
    }

    void RTree::createNearestNDistanceQueue(Point& p, int count, Collections::PriorityQueue& distanceQueue, float furthestDistance) {
        //  return immediately if given an invalid "count" parameter
        if (count <= 0) {
            return;
        }

        parents = std::stack<uint32_t>();
        parents.push(rootNodeId);

        parentsEntry = std::stack<uint32_t>();
        parentsEntry.push(-1);

        auto savedValues = Collections::IntVector();
        float savedPriority = 0;

        float furthestDistanceSq = furthestDistance * furthestDistance;

        while (parents.size() > 0) {
            auto n = getNode(parents.top());
            uint32_t startIndex = parentsEntry.top() + 1;

            if (!n.isLeaf()) {
                // go through every entry in the index node to check
                // if it could contain an entry closer than the farthest entry
                // currently stored.
                bool near = false;
                for (uint32_t i = startIndex; i < n.entryCount; i++) {
                    if (Rectangle::distanceSq(n.entries[i].minX, n.entries[i].minY,
                                           n.entries[i].maxX, n.entries[i].maxY,
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
                    float entryDistanceSq = Rectangle::distanceSq(n.entries[i].minX, n.entries[i].minY, n.entries[i].maxX, n.entries[i].maxY, p.x, p.y);
                    uint32_t entryId = n.ids[i];

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

    void RTree::nearestNUnsorted(Point& p, int count, float furthestDistance) {
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

        auto response = [&]() {
            // Here we simply print to console instead of logging
            std::cout << "Rectangle with distance= " << distanceQueue.getValue() << std::endl;
            distanceQueue.pop();
        };

        while (distanceQueue.size() > 0) {
            response();
        }
    }

    void RTree::nearestN(Point& p, int count, float furthestDistance) {
        auto distanceQueue = Collections::PriorityQueue(false);
        createNearestNDistanceQueue(p, count, distanceQueue, furthestDistance);
        distanceQueue.setSortOrder(true);

        auto response = [&]() {
            // Here we simply print to console instead of logging
            std::cout << "Rectangle with distance= " << distanceQueue.getValue() << std::endl;
            distanceQueue.pop();
        };

        while (distanceQueue.size() > 0) {
            response();
        }
    }

    void RTree::intersects(Rectangle& r) {
        auto rootNode = getNode(rootNodeId);
        intersects(r, rootNode);
    }

    void RTree::contains(Rectangle& r) {
        // find all rectangles in the tree that are contained by the passed rectangle
        // written to be non-recursive (should model other searches on this?)
        parents = std::stack<uint32_t>();
        parents.push(rootNodeId);

        parentsEntry = std::stack<uint32_t>();
        parentsEntry.push(-1);

        auto response = [&]() {
            for (uint32_t id : ids) {
                std::cout << "Rectangle with ID: " << id << " was contained" << std::endl;
            }
        };

        while (!parents.empty()) {
            auto n = getNode(parents.top());
            int startIndex = parentsEntry.top() + 1;

            if (!n.isLeaf()) {
                // go through every entry in the index node to check
                // if it intersects the passed rectangle. If so, it
                // could contain entries that are contained.
                bool intersects = false;
                for (int i = startIndex; i < n.entryCount; i++) {
                    if (Rectangle::intersects(r.minX, r.minY, r.maxX, r.maxY,
                                             n.entries[i].minX, n.entries[i].minY, n.entries[i].maxX, n.entries[i].maxY)) {
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
                    if (Rectangle::contains(r.minX, r.minY, r.maxX, r.maxY,
                                           n.entries[i].minX, n.entries[i].minY, n.entries[i].maxX, n.entries[i].maxY)) {
                        ids.push_back(n.ids[i]);
                    }
                }
            }

            parents.pop();
            parentsEntry.pop();
        }
        response();
    }

    int RTree::treeSize() const {
        return size;
    }

    Rectangle RTree::getBounds() {
        Rectangle bounds;

        auto n = getNode(getRootNodeId());
        if (!n.isEmpty() && n.entryCount > 0) {
            bounds = Rectangle();
            bounds.minX = n.mbrMinX;
            bounds.minY = n.mbrMinY;
            bounds.maxX = n.mbrMaxX;
            bounds.maxY = n.mbrMaxY;
        }
        return bounds;
    }

    uint32_t RTree::getNextNodeId() {
        uint32_t nextNodeId;
        if (!deletedNodeIds.empty()) {
            nextNodeId = deletedNodeIds.top();
            deletedNodeIds.pop();
        } else {
            nextNodeId = 1 + highestUsedNodeId++;
        }
        return nextNodeId;
    }

    Node& RTree::getNode(uint32_t id) {
        return *nodeMap.at(id);
    }

    uint32_t RTree::getRootNodeId() const {
        return rootNodeId;
    }

    Node& RTree::splitNode(Node& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, uint32_t newId) {
        // entryStatus.assign(initialEntryStatus.begin(), initialEntryStatus.end());
        Node* newNode =  new Node(getNextNodeId(), n.level);
        nodeMap.insert({newNode->nodeId, newNode});

        if (!newNode->isEmpty()) {
            pickSeeds(n, newRectMinX, newRectMinY, newRectMaxX, newRectMaxY, newId, *newNode);
        } else {
            std::cerr << "Error: Failed to split node is null" << std::endl;
            std::exit(1);
        }

        while (n.entryCount + newNode->entryCount < maxNodeEntries + 1) {
            if (maxNodeEntries + 1 - newNode->entryCount == minNodeEntries) {
                for (int i = 0; i < maxNodeEntries; i++) {
                    if (entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {
                        entryStatus[i] = ENTRY_STATUS_ASSIGNED;

                        if (n.entries[i].minX < n.mbrMinX) {
                            n.mbrMinX = n.entries[i].minX;
                        }
                        if (n.entries[i].minY < n.mbrMinY) {
                            n.mbrMinY = n.entries[i].minY;
                        }
                        if (n.entries[i].maxX > n.mbrMaxX) {
                            n.mbrMaxX = n.entries[i].maxX;
                        }
                        if (n.entries[i].maxY > n.mbrMaxY) {
                            n.mbrMaxY = n.entries[i].maxY;
                        }

                        n.entryCount++;
                    }
                }
                break;
            }
            if (maxNodeEntries + 1 - n.entryCount == minNodeEntries) {
                for (int i = 0; i < maxNodeEntries; i++) {
                    if (entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {
                        entryStatus[i] = ENTRY_STATUS_ASSIGNED;
                        newNode->addEntry(n.entries[i].minX, n.entries[i].minY, n.entries[i].maxX, n.entries[i].maxY, n.ids[i]);
                        n.ids[i] = 0;
                    }
                }
                break;
            }
            pickNext(n, *newNode);
        }
        n.reorganize(maxNodeEntries);
        return *newNode;
    }

    void RTree::pickSeeds(Node& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, uint32_t newId, Node& newNode) {
        float maxNormalizedSeparation = -1;
        int highestLowIndex = -1;
        int lowestHighIndex = -1;

        // for the purposes of picking seeds, take the MBR of the node to include
        // the new rectangle aswell.
        if (newRectMinX < n.mbrMinX) {
            n.mbrMinX = newRectMinX;
        }
        if (newRectMinY < n.mbrMinY) {
            n.mbrMinY = newRectMinY;
        }
        if (newRectMaxX > n.mbrMaxX) {
            n.mbrMaxX = newRectMaxX;
        }
        if (newRectMaxY > n.mbrMaxY) {
            n.mbrMaxY = newRectMaxY;
        }

        float mbrLenX = n.mbrMaxX - n.mbrMinX;
        float mbrLenY = n.mbrMaxY - n.mbrMinY;

        float tempHighestLow = newRectMinX;
        int tempHighestLowIndex = -1;

        float tempLowestHigh = newRectMaxX;
        int tempLowestHighIndex = -1;

        for (int i = 0; i < n.entryCount; i++) {
            float tempLow = n.entries[i].minX;
            if (tempLow >= tempHighestLow) {
                tempHighestLow = tempLow;
                tempHighestLowIndex = i;
            } else {
                float tempHigh = n.entries[i].maxX;
                if (tempHigh <= tempLowestHigh) {
                    tempLowestHigh = tempHigh;
                    tempLowestHighIndex = i;
                }
            }

            float normalizedSeparation;

            if (mbrLenX == 0) {
                normalizedSeparation = 1.0f;  // Avoid division by zero, default to 1
            } else {
                normalizedSeparation = (tempHighestLow - tempLowestHigh) / mbrLenX;  // Normalize the separation
            }

            if (normalizedSeparation >= maxNormalizedSeparation) {
                highestLowIndex = tempHighestLowIndex;
                lowestHighIndex = tempLowestHighIndex;
                maxNormalizedSeparation = normalizedSeparation;
            }
        }

        tempHighestLow = newRectMinY;
        tempHighestLowIndex = -1;

        tempLowestHigh = newRectMaxY;
        tempLowestHighIndex = -1;

        for (int i = 0; i < n.entryCount; i++) {
            float tempLow = n.entries[i].minY;
            if (tempLow >= tempHighestLow) {
                tempHighestLow = tempLow;
                tempHighestLowIndex = i;
            } else {
                float tempHigh = n.entries[i].maxY;
                if (tempHigh <= tempLowestHigh) {
                    tempLowestHigh = tempHigh;
                    tempLowestHighIndex = i;
                }
            }

            float normalizedSeparation;

            if (mbrLenY == 0) {
                normalizedSeparation = 1.0f;  // Avoid division by zero, default to 1
            } else {
                normalizedSeparation = (tempHighestLow - tempLowestHigh) / mbrLenY;  // Normalize the separation
            }

            if (normalizedSeparation > 1 || normalizedSeparation < -1) {
                std::cerr << "Error: Normalized separation is not between -1 and 1" << std::endl;
            }

            if (normalizedSeparation >= maxNormalizedSeparation) {
                highestLowIndex = tempHighestLowIndex;
                lowestHighIndex = tempLowestHighIndex;
                maxNormalizedSeparation = normalizedSeparation;
            }
        }

        if (highestLowIndex == lowestHighIndex) {
            highestLowIndex = 0;
            float tempMinY = newRectMinY;
            lowestHighIndex = 0;
            float tempMaxX = n.entries[0].maxX;

            for (int i = 1; i < n.entryCount; i++) {
                if (n.entries[i].minY < tempMinY) {
                    tempMinY = n.entries[i].minY;
                    highestLowIndex = i;
                } else if (n.entries[i].maxX > tempMaxX) {
                    tempMaxX = n.entries[i].maxX;
                    lowestHighIndex = i;
                }
            }
        }

        if (highestLowIndex == 0) {
            newNode.addEntry(newRectMinX, newRectMinY, newRectMaxX, newRectMaxY, newId);
        } else {
            newNode.addEntry(n.entries[highestLowIndex].minX, n.entries[highestLowIndex].minY, n.entries[highestLowIndex].maxX, n.entries[highestLowIndex].maxY, n.ids[highestLowIndex]);
            n.ids[highestLowIndex] = 0;
            n.entries[highestLowIndex].minX = newRectMinX;
            n.entries[highestLowIndex].minY = newRectMinY;
            n.entries[highestLowIndex].maxX = newRectMaxX;
            n.entries[highestLowIndex].maxY = newRectMaxY;
            n.ids[highestLowIndex] = newId;
        }

        if (lowestHighIndex == 0) {
            lowestHighIndex = highestLowIndex;
        }

        entryStatus[lowestHighIndex] = ENTRY_STATUS_ASSIGNED;
        n.entryCount = 1;
        n.mbrMinX = n.entries[lowestHighIndex].minX;
        n.mbrMinY = n.entries[lowestHighIndex].minY;
        n.mbrMaxX = n.entries[lowestHighIndex].maxX;
        n.mbrMaxY = n.entries[lowestHighIndex].maxY;
    }

    int RTree::pickNext(Node& n, Node& newNode) {
        float maxDifference = -std::numeric_limits<float>::max();
        int next = 0;
        int nextGroup = 0;

        for (int i = 0; i < maxNodeEntries; i++) {
            if (entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {
                if (n.ids[i] == -1) {
                    std::cerr << "Error: Node " << n.nodeId << ", entry " << i << " is null" << std::endl;
                }

                float nIncrease = Rectangle::enlargement(
                    n.mbrMinX,
                    n.mbrMinY,
                    n.mbrMaxX,
                    n.mbrMaxY,
                    n.entries[i].minX,
                    n.entries[i].minY,
                    n.entries[i].maxX,
                    n.entries[i].maxY);

                float newNodeIncrease = Rectangle::enlargement(
                    newNode.mbrMinX,
                    newNode.mbrMinY,
                    newNode.mbrMaxX,
                    newNode.mbrMaxY,
                    n.entries[i].minX,
                    n.entries[i].minY,
                    n.entries[i].maxX,
                    n.entries[i].maxY);

                float difference = std::abs(nIncrease - newNodeIncrease);

                if (difference > maxDifference) {
                    next = i;

                    if (nIncrease < newNodeIncrease) {
                        nextGroup = 0;
                    } else if (newNodeIncrease < nIncrease) {
                        nextGroup = 1;
                    } else if (Rectangle::area(n.mbrMinX, n.mbrMinY, n.mbrMaxX, n.mbrMaxY) < Rectangle::area(newNode.mbrMinX, newNode.mbrMinY, newNode.mbrMaxX, newNode.mbrMaxY)) {
                        nextGroup = 0;
                    } else if (Rectangle::area(newNode.mbrMinX, newNode.mbrMinY, newNode.mbrMaxX, newNode.mbrMaxY) < Rectangle::area(n.mbrMinX, n.mbrMinY, n.mbrMaxX, n.mbrMaxY)) {
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
            if (n.entries[next].minX < n.mbrMinX) {
                n.mbrMinX = n.entries[next].minX;
            }
            if (n.entries[next].minY < n.mbrMinY) {
                n.mbrMinY = n.entries[next].minY;
            }
            if (n.entries[next].maxX > n.mbrMaxX) {
                n.mbrMaxX = n.entries[next].maxX;
            }
            if (n.entries[next].maxY > n.mbrMaxY) {
                n.mbrMaxY = n.entries[next].maxY;
            }
            n.entryCount++;
        } else {
            // move to new node.
            newNode.addEntry(n.entries[next].minX, n.entries[next].minY, n.entries[next].maxX, n.entries[next].maxY, n.ids[next]);
            n.ids[next] = 0;
        }

        return next;
    }

    float RTree::nearest(Point& p, Node& n, float furthestDistanceSq, Collections::IntVector& nearestIds) {
        for (int i = 0; i < n.entryCount; i++) {
            float tempDistanceSq = Rectangle::distanceSq(n.entries[i].minX, n.entries[i].minY, n.entries[i].maxX, n.entries[i].maxY, p.x, p.y);
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

    bool RTree::intersects(Rectangle& r, Node& n) {
        for (int i = 0; i < n.entryCount; i++) {
            if (Rectangle::intersects(
                r.minX,
                r.minY,
                r.maxX,
                r.maxY,
                n.entries[i].minX,
                n.entries[i].minY,
                n.entries[i].maxX,
                n.entries[i].maxY)) {
                if (n.isLeaf()) {
                    // TODO: Lambda function
                    /*if (!v.execute(n.ids[i])) {
                        return false;
                    }*/
                } else {
                    Node& childNode = getNode(n.ids[i]);
                    /*if (!intersects(r, v, childNode)) {
                        return false;
                    }*/
                }
            }
        }
        return true;
    }

    void RTree::condenseTree(Node& l) {
        // CT1 [Initialize] Set n=l. Set the list of eliminated
        // nodes to be empty.
        auto n = &l;
        Node parent;
        uint32_t parentEntry = 0;

        std::stack<uint32_t> eliminatedNodeIds;

        // CT2 [Find parent entry] If N is the root, go to CT6. Otherwise
        // let P be the parent of N, and let En be N's entry in P
        while (n->level != treeHeight) {
            parent = getNode(parents.top());
            parents.pop();
            parentEntry = parentsEntry.top();
            parentsEntry.pop();

            // CT3 [Eliminiate under-full node] If N has too few entries,
            // delete En from P and add N to the list of eliminated nodes
            if (n->entryCount < minNodeEntries) {
                parent.deleteEntry(parentEntry);
                eliminatedNodeIds.push(n->nodeId);
            } else {
                // CT4 [Adjust covering rectangle] If N has not been eliminated,
                // adjust EnI to tightly contain all entries in N
                if (n->mbrMinX != parent.entries[parentEntry].minX ||
                    n->mbrMinY != parent.entries[parentEntry].minY ||
                    n->mbrMaxX != parent.entries[parentEntry].maxX ||
                    n->mbrMaxY != parent.entries[parentEntry].maxY)
                {

                    float deletedMinX = parent.entries[parentEntry].minX;
                    float deletedMinY = parent.entries[parentEntry].minY;
                    float deletedMaxX = parent.entries[parentEntry].maxX;
                    float deletedMaxY = parent.entries[parentEntry].maxY;

                    parent.entries[parentEntry].minX = n->mbrMinX;
                    parent.entries[parentEntry].minY = n->mbrMinY;
                    parent.entries[parentEntry].maxX = n->mbrMaxX;
                    parent.entries[parentEntry].maxY = n->mbrMaxY;

                    parent.recalculateMBRIfInfluencedBy(deletedMinX, deletedMinY, deletedMaxX, deletedMaxY);
                }
            }
            // CT5 [Move up one level in tree] Set N=P and repeat from CT2
            n = &parent;
        }

        // CT6 [Reinsert orphaned entries] Reinsert all entries of nodes in set Q.
        // Entries from eliminated leaf nodes are reinserted in tree leaves as in
        // Insert(), but entries from higher level nodes must be placed higher in
        // the tree, so that leaves of their dependent subtrees will be on the same
        // level as leaves of the main tree
        while (!eliminatedNodeIds.empty()) {
            auto e = getNode(eliminatedNodeIds.top());
            eliminatedNodeIds.pop();
            for (int j = 0; j < e.entryCount; j++) {
                add(e.entries[j].minX, e.entries[j].minY, e.entries[j].maxX, e.entries[j].maxY, e.ids[j], e.level);
                e.ids[j] = 0;
            }
            e.entryCount = 0;
            deletedNodeIds.push(e.nodeId);
        }
    }

    Node& RTree::chooseNode(float minX, float minY, float maxX, float maxY, int level, uint32_t staringNodeId) {
        Node& n = getNode(staringNodeId);
        parents = std::stack<uint32_t>();
        parentsEntry = std::stack<uint32_t>();

        if (n.isEmpty()) {
            cerr << "Could not get root node " << rootNodeId << endl;
        }

        if (n.level == level) {
            return n;
        }

        float leastEnlargement = Rectangle::enlargement(
            n.entries[0].minX,
            n.entries[0].minY,
            n.entries[0].maxX,
            n.entries[0].maxY,
            minX,
            minY,
            maxX,
            maxY);

        int index = 0; // index of rectangle in subtree
        for (int i = 1; i < n.entryCount; i++) {
            float tempMinX = n.entries[0].minX;
            float tempMinY = n.entries[0].minY;
            float tempMaxX = n.entries[0].maxX;
            float tempMaxY = n.entries[0].maxY;
            float tempEnlargement = Rectangle::enlargement(tempMinX, tempMinY, tempMaxX, tempMaxY,
                                                          minX, minY, maxX, maxY);
            if ((tempEnlargement < leastEnlargement) ||
                ((tempEnlargement == leastEnlargement) &&
                 (Rectangle::area(tempMinX, tempMinY, tempMaxX, tempMaxY) <
                  Rectangle::area(
                      n.entries[0].minX,
                      n.entries[0].minY,
                      n.entries[0].maxX,
                      n.entries[0].maxY))))
                {
                    index = i;
                    leastEnlargement = tempEnlargement;
                }
        }

        parents.push(n.nodeId);
        parentsEntry.push(index);

        chooseNode(minX, minY, maxX, maxY, level, n.ids[index]);
    }

    Node& RTree::adjustTree(Node& n, Node& nn) {
        while (n.level != treeHeight) {
            auto parent = getNode(parents.top());
            parents.pop();
            int entry = parentsEntry.top();
            parentsEntry.pop();

            if (parent.ids[entry] != n.nodeId) {
                cerr << "Error: entry " << entry << " in node " <<
                     parent.nodeId << " should point to node " <<
                     n.nodeId << ", actually points to node " << parent.ids[entry] << endl;
            }

            if (parent.entries[entry].minX != n.mbrMinX ||
              parent.entries[entry].minY != n.mbrMinY ||
              parent.entries[entry].maxX != n.mbrMaxX ||
              parent.entries[entry].maxY != n.mbrMaxY) {

                parent.entries[entry].minX = n.mbrMinX;
                parent.entries[entry].minY = n.mbrMinY;
                parent.entries[entry].maxX = n.mbrMaxX;
                parent.entries[entry].maxY = n.mbrMaxY;

                parent.recalculateMBR();
            }

            Node newNode;
            if (nn.isEmpty()) {
                if (parent.entryCount < maxNodeEntries) {
                    parent.addEntry(nn.mbrMinX, nn.mbrMinY, nn.mbrMaxX, nn.mbrMaxY, nn.nodeId);
                } else {
                    newNode = splitNode(parent, nn.mbrMinX, nn.mbrMinY, nn.mbrMaxX, nn.mbrMaxY, nn.nodeId);
                }
            }

            // AT5 [Move up to next level] Set N = P and set NN = PP if a split
            // occurred. Repeat from AT2
            n = parent;
            nn = newNode;

            parent = Node();
            newNode = Node();
        }

        return nn;
    }

    Rectangle& RTree::calculateMBR(Node& n) {
        auto mbr = Rectangle();

        for (int i = 0; i < n.entryCount; i++) {
            if (n.entries[i].minX < mbr.minX) mbr.minX = n.entries[i].minX;
            if (n.entries[i].minY < mbr.minY) mbr.minY = n.entries[i].minY;
            if (n.entries[i].maxX > mbr.maxX) mbr.maxX = n.entries[i].maxX;
            if (n.entries[i].maxY > mbr.maxY) mbr.maxY = n.entries[i].maxY;
        }

        return mbr;
    }

    uint32_t RTree::numNodes() const {
        return nodeMap.size();
    }

}
