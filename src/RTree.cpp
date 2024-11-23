#include "RTree.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <queue>
#include <set>
#include <vector>

#include <boost/container/vector.hpp>

#include "Rectangle.h"

namespace spatialindex {

    RTree::RTree() :
        m_initialEntryStatus(m_entryStatus),
        m_maxNodeEntries(DEFAULT_MAX_NODE_ENTRIES),
        m_minNodeEntries(DEFAULT_MIN_NODE_ENTRIES)
    {
        m_entryStatus.assign(m_maxNodeEntries, ENTRY_STATUS_UNASSIGNED);
        m_root = std::make_shared<Node>(m_rootNodeId, 1);
        m_nodeMap[m_rootNodeId] = m_root;
    }

    void RTree::add(const Rectangle & r, const int id)
    {
        add(r.minX, r.minY, r.maxX, r.maxY, id, 1);
        m_size++;
    }

    void RTree::add(float minX, float minY, float maxX, float maxY, int id, int level) {
        m_parents = std::stack<int>();
        m_parentsEntry = std::stack<int>();
        const auto& node = chooseNode(minX, minY, maxX, maxY, level);
        std::shared_ptr<Node> newLeaf;

        // Check if the node has space for new entry
        if (node->entryCount < m_maxNodeEntries)
        {
            node->addEntry(minX, minY, maxX, maxY, id);
        } else {
            newLeaf = splitNode(node, minX, minY, maxX, maxY, id);
        }

        const auto newNode = adjustTree(node, newLeaf);

        if (newNode != nullptr)
        {
            const int oldRootNodeId = m_rootNodeId;
            const std::shared_ptr<Node> oldRoot = getNode(oldRootNodeId);

            m_rootNodeId = getNextNodeId();
            m_treeHeight++;
            const auto root = std::make_shared<Node>(m_rootNodeId, m_treeHeight);

            root->addEntry(newNode->mbrMinX, newNode->mbrMinY, newNode->mbrMaxX, newNode->mbrMaxY, newNode->nodeId);
            root->addEntry(oldRoot->mbrMinX, oldRoot->mbrMinY, oldRoot->mbrMaxX, oldRoot->mbrMaxY, oldRoot->nodeId);

            m_nodeMap[root->getNodeId()] = root;
        }
    }

    int RTree::getNextNodeId() {
        int nextNodeId;
        if (!m_deletedNodeIds.empty()) {
            nextNodeId = m_deletedNodeIds.top();
            m_deletedNodeIds.pop();
        } else {
            nextNodeId = 1 + m_highestUsedNodeId++;
        }
        return nextNodeId;
    }

    std::shared_ptr<Node> &RTree::getNode(const int id) {
        return m_nodeMap[id];
    }

    int RTree::getRootNodeId() const {
        return m_rootNodeId;
    }

    std::shared_ptr<Node> RTree::splitNode(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId)
    {
        // entryStatus.assign(initialEntryStatus.begin(), initialEntryStatus.end());
        auto newNode = std::make_shared<Node>(getNextNodeId(), n->level);
        m_nodeMap.insert(std::make_pair(newNode->nodeId, newNode));

        if (newNode != nullptr) {
            pickSeeds(n, newRectMinX, newRectMinY, newRectMaxX, newRectMaxY, newId, newNode);
        } else {
            std::cerr << "Error: Failed to split node is null" << std::endl;
            std::exit(1);
        }

        while (n->entryCount + newNode->entryCount < m_maxNodeEntries + 1) {
            if (m_maxNodeEntries + 1 - newNode->entryCount == m_minNodeEntries) {
                for (int i = 0; i < m_maxNodeEntries; i++) {
                    if (m_entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {
                        m_entryStatus[i] = ENTRY_STATUS_ASSIGNED;

                        if (n->entries[i].minX < n->mbrMinX) {
                            n->mbrMinX = n->entries[i].minX;
                        }
                        if (n->entries[i].minY < n->mbrMinY) {
                            n->mbrMinY = n->entries[i].minY;
                        }
                        if (n->entries[i].maxX > n->mbrMaxX) {
                            n->mbrMaxX = n->entries[i].maxX;
                        }
                        if (n->entries[i].maxY > n->mbrMaxY) {
                            n->mbrMaxY = n->entries[i].maxY;
                        }

                        n->entryCount++;
                    }
                }
                break;
            }
            if (m_maxNodeEntries + 1 - n->entryCount == m_minNodeEntries) {
                for (int i = 0; i < m_maxNodeEntries; i++) {
                    if (m_entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {
                        m_entryStatus[i] = ENTRY_STATUS_ASSIGNED;
                        newNode->addEntry(n->entries[i].minX, n->entries[i].minY, n->entries[i].maxX, n->entries[i].maxY, n->ids[i]);
                        n->deleteEntry(i);
                    }
                }
                break;
            }
            pickNext(n, newNode);
        }
        n->reorganize(m_maxNodeEntries);
        return newNode;
    }

    void RTree::pickSeeds(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId, const std::shared_ptr<Node>& newNode) {
        float maxNormalizedSeparation = -1;
        int highestLowIndex = -1;
        int lowestHighIndex = -1;

        // Update MBR of the node to include the new rectangle
        n->mbrMinX = std::min(n->mbrMinX, newRectMinX);
        n->mbrMinY = std::min(n->mbrMinY, newRectMinY);
        n->mbrMaxX = std::max(n->mbrMaxX, newRectMaxX);
        n->mbrMaxY = std::max(n->mbrMaxY, newRectMaxY);

        float mbrLenX = n->mbrMaxX - n->mbrMinX;
        float mbrLenY = n->mbrMaxY - n->mbrMinY;

        // X-Dimension
        float tempHighestLow = newRectMinX;
        int tempHighestLowIndex = -1;

        float tempLowestHigh = newRectMaxX;
        int tempLowestHighIndex = -1;

        for (int i = 0; i < n->entryCount; i++) {
            float tempLow = n->entries[i].minX;
            if (tempLow >= tempHighestLow) {
                tempHighestLow = tempLow;
                tempHighestLowIndex = i;
            } else {
                float tempHigh = n->entries[i].maxX;
                if (tempHigh <= tempLowestHigh) {
                    tempLowestHigh = tempHigh;
                    tempLowestHighIndex = i;
                }
            }

            float normalizedSeparation = mbrLenX == 0 ? 1.0f : (tempHighestLow - tempLowestHigh) / mbrLenX;

            if (normalizedSeparation >= maxNormalizedSeparation) {
                highestLowIndex = tempHighestLowIndex;
                lowestHighIndex = tempLowestHighIndex;
                maxNormalizedSeparation = normalizedSeparation;
            }
        }

        // Y-Dimension
        tempHighestLow = newRectMinY;
        tempHighestLowIndex = -1;

        tempLowestHigh = newRectMaxY;
        tempLowestHighIndex = -1;

        for (int i = 0; i < n->entryCount; i++) {
            float tempLow = n->entries[i].minY;
            if (tempLow >= tempHighestLow) {
                tempHighestLow = tempLow;
                tempHighestLowIndex = i;
            } else {
                float tempHigh = n->entries[i].maxY;
                if (tempHigh <= tempLowestHigh) {
                    tempLowestHigh = tempHigh;
                    tempLowestHighIndex = i;
                }
            }

            float normalizedSeparation = mbrLenY == 0 ? 1.0f : (tempHighestLow - tempLowestHigh) / mbrLenY;

            if (normalizedSeparation > 1 || normalizedSeparation < -1) {
                // std::cerr << "Error: Normalized separation is not between -1 and 1" << std::endl;
            }

            if (normalizedSeparation >= maxNormalizedSeparation) {
                highestLowIndex = tempHighestLowIndex;
                lowestHighIndex = tempLowestHighIndex;
                maxNormalizedSeparation = normalizedSeparation;
            }
        }

        // Handle potential overlap cases
        if (highestLowIndex == lowestHighIndex) {
            highestLowIndex = -1;
            float tempMinY = newRectMinY;
            lowestHighIndex = 0;
            float tempMaxX = n->entries[0].maxX;

            for (int i = 1; i < n->entryCount; i++) {
                if (n->entries[i].minY < tempMinY) {
                    tempMinY = n->entries[i].minY;
                    highestLowIndex = i;
                } else if (n->entries[i].maxX > tempMaxX) {
                    tempMaxX = n->entries[i].maxX;
                    lowestHighIndex = i;
                }
            }
        }

        if (highestLowIndex == -1) {
            newNode->addEntry(newRectMinX, newRectMinY, newRectMaxX, newRectMaxY, newId);
        } else {
            newNode->addEntry(n->entries[highestLowIndex].minX, n->entries[highestLowIndex].minY, n->entries[highestLowIndex].maxX, n->entries[highestLowIndex].maxY, n->ids[highestLowIndex]);

            n->ids[highestLowIndex] = -1;

            n->entries[highestLowIndex].minX = newRectMinX;
            n->entries[highestLowIndex].minY = newRectMinY;
            n->entries[highestLowIndex].maxX = newRectMaxX;
            n->entries[highestLowIndex].maxY = newRectMaxY;

            n->ids[highestLowIndex] = newId;
        }

        if (lowestHighIndex == -1) {
            lowestHighIndex = highestLowIndex;
        }

        //m_entryStatus[lowestHighIndex] = ENTRY_STATUS_ASSIGNED;
        n->entryCount = 1;
        n->mbrMinX = n->entries[lowestHighIndex].minX;
        n->mbrMinY = n->entries[lowestHighIndex].minY;
        n->mbrMaxX = n->entries[lowestHighIndex].maxX;
        n->mbrMaxY = n->entries[lowestHighIndex].maxY;
    }

    int RTree::pickNext(const std::shared_ptr<Node>& n, const std::shared_ptr<Node>& newNode) {
        float maxDifference = -std::numeric_limits<float>::max();
        int next = 0;
        int nextGroup = 0;

        for (int i = 0; i < n->entryCount; i++) {
            if (m_entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {
                if (n->ids[i] == -1) {
                    std::cerr << "Error: Node " << n->nodeId << ", entry " << i << " is null" << std::endl;
                }

                float nIncrease = Rectangle::enlargement(
                    n->mbrMinX,
                    n->mbrMinY,
                    n->mbrMaxX,
                    n->mbrMaxY,
                    n->entries[i].minX,
                    n->entries[i].minY,
                    n->entries[i].maxX,
                    n->entries[i].maxY);

                float newNodeIncrease = Rectangle::enlargement(
                    newNode->mbrMinX,
                    newNode->mbrMinY,
                    newNode->mbrMaxX,
                    newNode->mbrMaxY,
                    n->entries[i].minX,
                    n->entries[i].minY,
                    n->entries[i].maxX,
                    n->entries[i].maxY);

                float difference = std::abs(nIncrease - newNodeIncrease);

                if (difference > maxDifference) {
                    next = i;

                    if (nIncrease < newNodeIncrease) {
                        nextGroup = 0;
                    } else if (newNodeIncrease < nIncrease) {
                        nextGroup = 1;
                    } else if (Rectangle::area(n->mbrMinX, n->mbrMinY, n->mbrMaxX, n->mbrMaxY) < Rectangle::area(newNode->mbrMinX, newNode->mbrMinY, newNode->mbrMaxX, newNode->mbrMaxY)) {
                        nextGroup = 0;
                    } else if (Rectangle::area(newNode->mbrMinX, newNode->mbrMinY, newNode->mbrMaxX, newNode->mbrMaxY) < Rectangle::area(n->mbrMinX, n->mbrMinY, n->mbrMaxX, n->mbrMaxY)) {
                        nextGroup = 1;
                    } else if (newNode->entryCount < m_maxNodeEntries / 2) {
                        nextGroup = 0;
                    } else {
                        nextGroup = 1;
                    }
                    maxDifference = difference;
                }
            }
        }

        m_entryStatus[next] = ENTRY_STATUS_ASSIGNED;

        if (nextGroup == 0) {
            if (n->entries[next].minX < n->mbrMinX) {
                n->mbrMinX = n->entries[next].minX;
            }
            if (n->entries[next].minY < n->mbrMinY) {
                n->mbrMinY = n->entries[next].minY;
            }
            if (n->entries[next].maxX > n->mbrMaxX) {
                n->mbrMaxX = n->entries[next].maxX;
            }
            if (n->entries[next].maxY > n->mbrMaxY) {
                n->mbrMaxY = n->entries[next].maxY;
            }
            n->entryCount++;
        } else {
            // move to new node.
            newNode->addEntry(n->entries[next].minX, n->entries[next].minY, n->entries[next].maxX, n->entries[next].maxY, n->ids[next]);
            n->deleteEntry(next);
        }
        return next;
    }

    std::shared_ptr<Node> RTree::chooseNode(float minX, float minY, float maxX, float maxY, int level) {
        auto n = getNode(m_rootNodeId);
        if (!n) {
            std::cerr << "Could not get root node " << m_rootNodeId << std::endl;
            return nullptr;
        }

        m_parents = std::stack<int>();
        m_parentsEntry = std::stack<int>();

        while (n && n->level != level) {
            float leastEnlargement = std::numeric_limits<float>::max();
            int index = -1;

            for (int i = 0; i < n->entryCount; ++i) {
                const auto& entry = n->entries[i];
                float tempEnlargement = Rectangle::enlargement(entry.minX, entry.minY, entry.maxX, entry.maxY, minX, minY, maxX, maxY);

                if (tempEnlargement < leastEnlargement) {
                    leastEnlargement = tempEnlargement;
                    index = i;
                } else if (tempEnlargement == leastEnlargement) {
                    if (index == -1) {
                        std::cerr << "Error finding a suitable child node." << std::endl;
                        return nullptr;
                    }

                    if (Rectangle::area(entry.minX, entry.minY, entry.maxX, entry.maxY) <
                        Rectangle::area(n->entries[index].minX, n->entries[index].minY, n->entries[index].maxX, n->entries[index].maxY)) {
                        index = i;
                        }
                }
            }

            if (index == -1) {
                std::cerr << "Error finding a suitable child node." << std::endl;
                return nullptr;
            }

            m_parents.push(n->nodeId);
            m_parentsEntry.push(index);
            n = getNode(n->ids[index]);
        }

        return n;
    }

    std::shared_ptr<Node> RTree::adjustTree(std::shared_ptr<Node> n, std::shared_ptr<Node> nn) {
        while (n->level != m_treeHeight) {
            auto parent = getNode(m_parents.top());
            m_parents.pop();
            const int entry = m_parentsEntry.top();
            m_parentsEntry.pop();

            if (parent->ids[entry] != n->nodeId) {
                std::cerr << "Error: entry " << entry << " in node " <<
                     parent->nodeId << " should point to node " <<
                     n->nodeId << ", actually points to node " << parent->ids[entry] << std::endl;
            }

            if (parent->entries[entry].minX != n->mbrMinX ||
              parent->entries[entry].minY != n->mbrMinY ||
              parent->entries[entry].maxX != n->mbrMaxX ||
              parent->entries[entry].maxY != n->mbrMaxY) {

                parent->entries[entry].minX = n->mbrMinX;
                parent->entries[entry].minY = n->mbrMinY;
                parent->entries[entry].maxX = n->mbrMaxX;
                parent->entries[entry].maxY = n->mbrMaxY;

                parent->recalculateMBR();
            }

            std::shared_ptr<Node> newNode;
            if (nn != nullptr) {
                if (parent->entryCount < m_maxNodeEntries) {
                    parent->addEntry(nn->mbrMinX, nn->mbrMinY, nn->mbrMaxX, nn->mbrMaxY, nn->nodeId);
                } else {
                    newNode = splitNode(parent, nn->mbrMinX, nn->mbrMinY, nn->mbrMaxX, nn->mbrMaxY, nn->nodeId);
                }
            }

            // AT5 [Move up to the next level] Set N = P and set NN = PP if a split
            // occurred. Repeat from AT2
            n = std::move(parent);
            nn = std::move(newNode);

            //parent = std::make_shared<Node>();
            //newNode = std::make_shared<Node>();
        }
        return nn;
    }

    int RTree::treeSize() const {
        return m_size;
    }

    int RTree::numNodes() const {
        return m_nodeMap.size();
    }


    float RTree::nearest(Point& p, std::shared_ptr<Node>& n, float furthestDistanceSq, boost::container::vector<int>& nearestIds) {
        // TODO: Fix calculation its not accurate.
        for (int i = 0; i < n->entryCount; i++) {
            float tempDistanceSq = Rectangle::distanceSq(n->entries[i].minX, n->entries[i].minY, n->entries[i].maxX, n->entries[i].maxY, p.x, p.y);
            if (n->isLeaf()) { // for leaves, the distance is the actual nearest distance
                if (tempDistanceSq < furthestDistanceSq) {
                    furthestDistanceSq = tempDistanceSq;
                    nearestIds.clear();
                }
                if (tempDistanceSq <= furthestDistanceSq) {
                    nearestIds.push_back(n->ids[i]);
                }
            } else { // for index nodes, only go into them if they potentially could have
                // a rectangle nearer than actualNearest
                if (tempDistanceSq <= furthestDistanceSq) {
                    // search the child node
                    furthestDistanceSq = nearest(p, getNode(n->ids[i]), furthestDistanceSq, nearestIds);
                }
            }
        }
        return furthestDistanceSq;
    }

    std::set<int> RTree::intersects(Rectangle& r, std::shared_ptr<Node>& n) {
        std::set<int> intersectedRectangles;
        // Iterate through each entry in the current node
        for (int i = 0; i < n->entryCount; i++) {
            if (n->isLeaf()) {
                // Check if the rectangles intersect
                if (Rectangle::intersects(r.minX, r.minY, r.maxX, r.maxY, n->entries[i].minX, n->entries[i].minY,
                                          n->entries[i].maxX, n->entries[i].maxY)) {
                    // Collect the intersecting rectangles
                    intersectedRectangles.emplace(n->entries[i].id);
                }
            } else {
                // If the node is not a leaf, the entry represents a child node
                // Recursively check the child node for intersections
                intersects(r, getNode(n->ids[i]));
            }
        }
        return intersectedRectangles;
    }

    Rectangle & RTree::calculateMBR(Node& n) {
        auto mbr = Rectangle();

        for (int i = 0; i < n.entryCount; i++) {
            if (n.entries[i].minX < mbr.minX) mbr.minX = n.entries[i].minX;
            if (n.entries[i].minY < mbr.minY) mbr.minY = n.entries[i].minY;
            if (n.entries[i].maxX > mbr.maxX) mbr.maxX = n.entries[i].maxX;
            if (n.entries[i].maxY > mbr.maxY) mbr.maxY = n.entries[i].maxY;
        }

        return mbr;
    }

    void RTree::nearest(Point& p, float furthestDistance) {
        auto rootNode = getNode(m_rootNodeId);

        const float furthestDistanceSq = furthestDistance * furthestDistance;
        auto nearestIds = boost::container::vector<int>();

        const auto dist = nearest(p, rootNode, furthestDistanceSq, nearestIds);

        printNearest(p, nearestIds, dist);
        nearestIds.clear();
    }

    void RTree::createNearestNDistanceQueue(const Point & p, const int count, const float furthestDistance) {
        // Return immediately if given an invalid "count" parameter
        if (count <= 0) {
            return;
        }

        m_parents = std::stack<int>();
        m_parents.push(m_rootNodeId);

        m_parentsEntry = std::stack<int>();
        m_parentsEntry.push(-1);

        // Initialize furthest distance square
        float furthestDistanceSq = furthestDistance * furthestDistance;

        while (!m_parents.empty()) {
            // Validate the node pointer
            auto n = getNode(m_parents.top());
            if (n == nullptr) {
                // Handle error or skip iteration
                m_parents.pop();
                m_parentsEntry.pop();
                continue;
            }
            int startIndex = m_parentsEntry.top() + 1;

            if (!n->isLeaf()) {
                // Traverse each entry in the index node
                bool foundNearChild = false;
                for (int i = startIndex; i < n->entryCount; i++) {
                    if (Rectangle::distanceSq(n->entries[i].minX, n->entries[i].minY,
                                              n->entries[i].maxX, n->entries[i].maxY,
                                              p.x, p.y) <= furthestDistanceSq) {
                        m_parents.push(n->ids[i]);
                        m_parentsEntry.pop();
                        m_parentsEntry.push(i);
                        m_parentsEntry.push(-1);
                        foundNearChild = true;
                        break;
                    }
                }
                if (foundNearChild) {
                    continue;
                }
            } else {
                // Leaf node: Check entries for nearest N neighbors
                for (int i = 0; i < n->entryCount; i++) {
                    float entryDistanceSq = Rectangle::distanceSq(n->entries[i].minX, n->entries[i].minY,
                                                                  n->entries[i].maxX, n->entries[i].maxY,
                                                                  p.x, p.y);
                    int entryId = n->ids[i];

                    if (entryDistanceSq <= furthestDistanceSq) {
                        // Add to the priority queue
                        m_distanceQueue.emplace(entryDistanceSq, entryId);

                        // Maintain the size of the priority queue
                        if (m_distanceQueue.size() > count) {
                            m_distanceQueue.pop();
                        }

                        // Update furthestDistanceSq if the queue is full
                        if (m_distanceQueue.size() >= count) {
                            furthestDistanceSq = m_distanceQueue.top().first;
                        }
                    }
                }
            }

            // Remove the current node from stack after processing
            m_parents.pop();
            m_parentsEntry.pop();
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
        createNearestNDistanceQueue(p, count, furthestDistance);

        auto response = [&]() {
            // Here we simply print to console instead of logging
            //std::cout << "Rectangle with distance= " << m_distanceQueue.getValue() << std::endl;
            m_distanceQueue.pop();
        };

        while (m_distanceQueue.size() > 0) {
            response();
        }
    }

    void RTree::nearestN(Point& p, int count, float furthestDistance) {
        createNearestNDistanceQueue(p, count, furthestDistance);
        printSortedQueue(m_distanceQueue);
    }

    void RTree::intersects(Rectangle& rect) {
        auto rootNode = getNode(m_rootNodeId);
        auto result = intersects(rect, rootNode);
        printIntersectedRectangles(result);
    }

    void RTree::contains(Rectangle& r) {
        m_parents = std::stack<int>();
        m_parents.push(m_rootNodeId);

        m_parentsEntry = std::stack<int>();
        m_parentsEntry.push(-1);

        while (!m_parents.empty()) {
            auto n = getNode(m_parents.top());
            int startIndex = m_parentsEntry.top() + 1;

            if (!n->isLeaf()) {
                // Check for intersections with the passed rectangle
                bool intersects = false;
                for (int i = startIndex; i < n->entryCount; i++) {
                    if (Rectangle::intersects(r.minX, r.minY, r.maxX, r.maxY,
                                               n->entries[i].minX, n->entries[i].minY, n->entries[i].maxX, n->entries[i].maxY))
                        {
                            m_parents.push(n->ids[i]);
                            m_parentsEntry.pop();
                            m_parentsEntry.push(i); // Update start index for child node
                            m_parentsEntry.push(-1);
                            intersects = true;
                            break; // Go to the next iteration of while()
                        }
                }
                if (intersects) {
                    continue; // Continue to the next parent node
                }
            } else {
                // Check for containment in the leaf node
                for (int i = 0; i < n->entryCount; i++) {
                    if (Rectangle::contains(r.minX, r.minY, r.maxX, r.maxY,
                                             n->entries[i].minX, n->entries[i].minY, n->entries[i].maxX, n->entries[i].maxY))
                        {
                            m_ids.push_back(n->ids[i]);
                        }
                }
            }

            // Pop the current node and entry index
            m_parents.pop();
            m_parentsEntry.pop();
        }
        printContainedRectangles(m_ids);
    }

    void RTree::printNearest(Point& p, boost::container::vector<int>& nearestIds, float distance) {
        std::cout << "\nNearest rectangle to point " + p.toString() << std::endl;
        std::cout << "ID: " << nearestIds.at(0) << " Distance: " << distance << std::endl;
    }

    void RTree::printSortedQueue(std::priority_queue<std::pair<float, int>>& queue) {
        std::vector<std::pair<float, int>> elements;

        // Extract all elements from the queue
        while (!queue.empty()) {
            elements.push_back(queue.top());
            queue.pop();
        }

        // Sort in ascending order (the smallest distance first)
        std::sort(elements.begin(), elements.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

        // Print sorted elements
        std::cout << "\nNearest " << elements.size() << " rectangles (ascending order):" << std::endl;
        for (const auto& [distance, id] : elements) {
            std::cout << "ID: " << id << " Distance: " << distance << std::endl;
        }
    }

    void RTree::printContainedRectangles(const std::vector<int>& ids) {
        std::cout << "\nContained rectangles:" << std::endl;
        for (int id : ids) {
            std::cout << "Rectangle with ID: " << id << " was contained" << std::endl;
        }
    }

    void RTree::printIntersectedRectangles(const std::set<int>& ids) {
        for (int id : ids) {
            std::cout << "Rectangle with ID: " << id << " was intersected" << std::endl;
        }
    }
}
