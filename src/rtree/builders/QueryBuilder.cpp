#include "QueryBuilder.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <vector>

#include "RTreeBulkLoad.h"

namespace rtree {

    QueryBuilder::QueryBuilder(RTreeBulkLoad& rtreeA) : m_rtreeA(rtreeA), m_rtreeB({}) {}
    QueryBuilder::QueryBuilder(RTreeBulkLoad& rtreeA, RTreeBulkLoad& rtreeB) : m_rtreeA(rtreeA), m_rtreeB(rtreeB) {}

    /*void QueryBuilder::Range(Rectangle& r) {
        range(m_rtreeA, r);
        //std::cout << "size: " << m_ids.size() << std::endl;
        //printRangeQuery(range, m_ids);
    }

    void QueryBuilder::NearestNeighbors(Point& p, int count) {
        nearestN(p, count);
        //printNearestNeighbors(p, m_distanceQueue);
    }*/

    /*void QueryBuilder::Join() {
        join(m_rtreeA, m_rtreeB);
        //printJoinQuery();
    }*/

    /*void QueryBuilder::join(RTreeBulkLoad& rtreeA, RTreeBulkLoad& rtreeB) {
        m_joinRectangles.clear(); // Clear previous results

        auto rootA = rtreeA.getNode(rtreeA.getRootNodeId());
        auto rootB = rtreeB.getNode(rtreeB.getRootNodeId());

        std::stack<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>> nodePairs;
        nodePairs.push({rootA, rootB});

        while (!nodePairs.empty()) {
            auto [nodeA, nodeB] = nodePairs.top();
            nodePairs.pop();

            // Prune if the two MBRs do not intersect.
            if (!Rectangle::intersects(
                    nodeA->mbrMinX, nodeA->mbrMinY, nodeA->mbrMaxX, nodeA->mbrMaxY,
                    nodeB->mbrMinX, nodeB->mbrMinY, nodeB->mbrMaxX, nodeB->mbrMaxY))
            {
                continue;
            }

            // Case 1: Both nodes are leaves – do pairwise comparisons.
            if (nodeA->isLeaf() && nodeB->isLeaf()) {
                for (int i = 0; i < nodeA->entryCount; i++) {
                    for (int j = 0; j < nodeB->entryCount; j++) {
                        if (Rectangle::intersects(
                                nodeA->entriesMinX[i], nodeA->entriesMinY[i], nodeA->entriesMaxX[i], nodeA->entriesMaxY[i],
                                nodeB->entriesMinX[j], nodeB->entriesMinY[j], nodeB->entriesMaxX[j], nodeB->entriesMaxY[j]))
                        {
                            int idA = nodeA->ids[i];
                            int idB = nodeB->ids[j];
                            m_joinRectangles[idA].push_back(idB);
                        }
                    }
                }
            }
            // Case 2: Both nodes are internal.
            else if (!nodeA->isLeaf() && !nodeB->isLeaf()) {
                // For each child of nodeA, use binary search in nodeB's sorted entries.
                // Note: This assumes nodeB's entries are sorted by minX.
                for (int i = 0; i < nodeA->entryCount; i++) {
                    auto childA = rtreeA.getNode(nodeA->ids[i]);

                    int low = 0, high = nodeB->entryCount;
                    while (low < high) {
                        int mid = (low + high) / 2;
                        if (nodeB->entriesMaxX[mid] < childA->mbrMinX)
                            low = mid + 1;
                        else
                            high = mid;
                    }
                    // Scan from low until nodeB’s child's minX is beyond childA’s maxX.
                    for (int j = low; j < nodeB->entryCount; j++) {
                        if (nodeB->entriesMinX[j] > childA->mbrMaxX)
                            break;
                        auto childB = rtreeB.getNode(nodeB->ids[j]);
                        if (Rectangle::intersects(
                                childA->mbrMinX, childA->mbrMinY, childA->mbrMaxX, childA->mbrMaxY,
                                childB->mbrMinX, childB->mbrMinY, childB->mbrMaxX, childB->mbrMaxY))
                        {
                            nodePairs.push({childA, childB});
                        }
                    }
                }
            }
            // Case 3: nodeA is internal, nodeB is a leaf.
            else if (!nodeA->isLeaf() && nodeB->isLeaf()) {
                for (int i = 0; i < nodeA->entryCount; i++) {
                    auto childA = rtreeA.getNode(nodeA->ids[i]);
                    if (childA->mbrMinX > nodeB->mbrMaxX)
                        break;
                    if (Rectangle::intersects(
                            childA->mbrMinX, childA->mbrMinY, childA->mbrMaxX, childA->mbrMaxY,
                            nodeB->mbrMinX, nodeB->mbrMinY, nodeB->mbrMaxX, nodeB->mbrMaxY))
                    {
                        nodePairs.push({childA, nodeB});
                    }
                }
            }
            // Case 4: nodeA is a leaf, nodeB is internal.
            else {
                for (int j = 0; j < nodeB->entryCount; j++) {
                    auto childB = rtreeB.getNode(nodeB->ids[j]);
                    if (childB->mbrMinX > nodeA->mbrMaxX)
                        break;
                    if (Rectangle::intersects(
                            nodeA->mbrMinX, nodeA->mbrMinY, nodeA->mbrMaxX, nodeA->mbrMaxY,
                            childB->mbrMinX, childB->mbrMinY, childB->mbrMaxX, childB->mbrMaxY))
                    {
                        nodePairs.push({nodeA, childB});
                    }
                }
            }
        }
    }*/

    void QueryBuilder::nearestN(const Point &p, const int count) {
        if (count <= 0) {
            return;
        }

        // Clear previously stored results.
        m_distanceQueue = std::priority_queue<std::pair<float, int>>();

        float furthestNeighborDistance = MAXFLOAT;
        const int targetCount = count;
        // Cache the target count as size_t to avoid repeated casts in the loops.
        const size_t targetCountLocal = static_cast<size_t>(targetCount);

        // Cache query coordinates.
        const float qx = p.x;
        const float qy = p.y;

        // A min-heap for nodes based on their bounding box distance to the query point.
        using NodePair = std::pair<float, int>;
        std::priority_queue<
            NodePair,
            std::vector<NodePair>,
            std::greater<NodePair>
        > nodeQueue;

        // Start with the root node.
        int rootNodeId = m_rtreeA.getRootNodeId();
        auto rootNode = m_rtreeA.getNode(rootNodeId);
        if (!rootNode) {
            return; // no root node
        }

        float rootDist = Rectangle::distanceSq(
            rootNode->mbrMinX, rootNode->mbrMinY,
            rootNode->mbrMaxX, rootNode->mbrMaxY,
            qx, qy
        );
        nodeQueue.emplace(rootDist, rootNodeId);

        // Best-first search.
        while (!nodeQueue.empty()) {
            // Early termination: if the smallest node in the queue is farther than our current candidate,
            // no better candidate exists.
            if (m_distanceQueue.size() == targetCountLocal &&
                nodeQueue.top().first > furthestNeighborDistance) {
                break;
            }

            auto [dist, nodeId] = nodeQueue.top();
            nodeQueue.pop();

            // Prune this node if its distance is already worse.
            if (m_distanceQueue.size() == targetCountLocal && dist > furthestNeighborDistance) {
                continue;
            }

            auto n = m_rtreeA.getNode(nodeId);
            if (!n) {
                continue;
            }

            if (!n->isLeaf()) {
                // For internal nodes, push children into the nodeQueue.
                for (int i = 0; i < n->entryCount; i++) {
                    float childDist = Rectangle::distanceSq(
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i],
                        qx, qy
                    );
                    if (m_distanceQueue.size() < targetCountLocal ||
                        childDist <= furthestNeighborDistance) {
                        nodeQueue.emplace(childDist, n->ids[i]);
                    }
                }
            } else {
                // For leaf nodes, process each entry.
                for (int i = 0; i < n->entryCount; i++) {
                    float entryDistanceSq = Rectangle::distanceSq(
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i],
                        qx, qy
                    );
                    int entryId = n->ids[i];

                    if (m_distanceQueue.size() < targetCountLocal) {
                        m_distanceQueue.emplace(entryDistanceSq, entryId);
                        if (m_distanceQueue.size() == targetCountLocal) {
                            furthestNeighborDistance = m_distanceQueue.top().first;
                        }
                    } else if (entryDistanceSq < m_distanceQueue.top().first) {
                        m_distanceQueue.pop();
                        m_distanceQueue.emplace(entryDistanceSq, entryId);
                        furthestNeighborDistance = m_distanceQueue.top().first;
                    }
                }
            }
        }
    }

    void getLeafs(RTreeBulkLoad& rtree, int nodeId, std::vector<int>& leafs) {
        auto node = rtree.getNode(nodeId);
        if (!node) return;
        if (node->isLeaf()) {
            for (int i = 0; i < node->entryCount; i++) {
                leafs.push_back(node->ids[i]);
            }
        } else {
            for (int i = 0; i < node->entryCount; i++) {
                getLeafs(rtree, node->ids[i], leafs);
            }
        }
    }

    /*void QueryBuilder::range(RTreeBulkLoad& rtree, const Rectangle& range) {
        m_ids.clear();
        std::queue<int> nodeQueue;

        float minX = range.minX;
        float minY = range.minY;
        float maxX = range.maxX;
        float maxY = range.maxY;

        nodeQueue.push(rtree.getRootNodeId());

        while (!nodeQueue.empty()) {
            int nodeId = nodeQueue.front();
            nodeQueue.pop(); // the actual node
            auto n = rtree.getNode(nodeId);

            if (!n) {
                std::cerr << "Error: Invalid node with ID " << nodeId << std::endl;
                continue;
            }

            /*if (!Rectangle::intersects(minX, minY, maxX, maxY,
                            n->mbrMinX, n->mbrMinY,
                            n->mbrMaxX, n->mbrMaxY))
            {
                continue;
            }#1#

            if (!n->isLeaf()) {
                if (Rectangle::contains(minX, minY, maxX, maxY, n->mbrMinX, n->mbrMinY, n->mbrMaxX, n->mbrMaxY)) {
                    getLeafs(rtree, n->nodeId, m_ids);
                    continue;
                }

                for (int i = 0; i < n->entryCount; i++) {
                    if (n->entries[i].intersects(
                            minX, minY, maxX, maxY))
                        {
                            nodeQueue.push(n->ids[i]);
                        }
                }
            } else {
                for (int i = 0; i < n->entryCount; i++) {

                    if (n->entries[i].intersects(
                            minX, minY, maxX, maxY))
                        {
                            m_ids.push_back(n->ids[i]);
                        }
                }
            }
        }
    }*/

    void QueryBuilder::printNearestNeighbors(Point& p, std::priority_queue<std::pair<float, int>>& queue) {
        std::vector<std::pair<float, int>> elements;

        // Early return if count = 1
        if (queue.size() == 1) {
            std::cout << "\nTarget Point " + p.toString() << std::endl;
            std::cout << "Nearest " << queue.size() << " rectangle:" << std::endl;
            std::cout << "ID: " << queue.top().second << " Distance: " << queue.top().first << std::endl;
            queue.pop();
            return;
        }

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
        std::cout << "\nTarget Point " + p.toString() << std::endl;
        std::cout << "Nearest " << elements.size() << " rectangles:" << std::endl;
        for (const auto& [distance, id] : elements) {
            std::cout << "ID: " << id << " Distance: " << distance << std::endl;
        }
    }

    void QueryBuilder::printRangeQuery(Rectangle& range, const std::vector<int>& ids) {
        std::cout << "\nRange: " + range.toString() << std::endl;
        std::cout << "Contained rectangles:" << std::endl;

        for (const int id : ids) {
            std::cout << "Rectangle with ID: " << id << " was contained" << std::endl;
        }
    }

    void QueryBuilder::printJoinQuery() {
        std::cout << "Join Query Results:" << std::endl;

        for (const auto& pair : m_joinRectangles) {
            int idA = pair.first;

            const auto& intersectedIds = pair.second;

            std::cout << "Rectangle ID in RTreeA: " << idA << " intersects with IDs in RTreeB: ";
            for (int idB : intersectedIds) {
                std::cout << idB << " ";
            }
            std::cout << std::endl;
        }
    }
}
