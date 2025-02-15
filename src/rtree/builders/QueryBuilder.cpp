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

    void QueryBuilder::Range(Rectangle& range) {
        contains(range);
        //printRangeQuery(range, m_ids);
    }

    void QueryBuilder::NearestNeighbors(Point& p, int count) {
        nearestN(p, count);
        //printNearestNeighbors(p, m_distanceQueue);
    }

    void QueryBuilder::Join() {
        join(m_rtreeA, m_rtreeB);
        //printJoinQuery();
    }

    void QueryBuilder::join(RTreeBulkLoad& rtreeA, RTreeBulkLoad& rtreeB) {
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
    }

    void QueryBuilder::nearestN(const Point &p, const int count) {
        if (count <= 0) {
            return;
        }

        // Clear previously stored results.
        m_distanceQueue = std::priority_queue<std::pair<float, int>>();

        float furthestNeighborDistance = MAXFLOAT;

        // A min-heap for nodes based on their bounding box distance to the query point.
        std::priority_queue<
            std::pair<float,int>,
            std::vector<std::pair<float,int>>,
            std::greater<std::pair<float,int>>
        > nodeQueue;

        // Start with the root node.
        int rootNodeId = m_rtreeA.getRootNodeId();
        auto rootNode = m_rtreeA.getNode(rootNodeId);
        if (!rootNode) {
            return; // no root node
        }

        // Use the node's stored MBR instead of recomputing it.
        float rootDist = Rectangle::distanceSq(
            rootNode->mbrMinX, rootNode->mbrMinY,
            rootNode->mbrMaxX, rootNode->mbrMaxY,
            p.x, p.y
        );
        nodeQueue.push({rootDist, rootNodeId});

        // Best-first search.
        while (!nodeQueue.empty()) {
            auto [dist, nodeId] = nodeQueue.top();
            nodeQueue.pop();

            // Prune if the node's bounding box is farther than the current furthest neighbor.
            if (m_distanceQueue.size() == static_cast<size_t>(count) && dist > furthestNeighborDistance) {
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
                        p.x, p.y
                    );
                    if (m_distanceQueue.size() < static_cast<size_t>(count) || childDist <= furthestNeighborDistance) {
                        nodeQueue.push({childDist, n->ids[i]});
                    }
                }
            } else {
                // For leaf nodes, process each entry.
                for (int i = 0; i < n->entryCount; i++) {
                    float entryDistanceSq = Rectangle::distanceSq(
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i],
                        p.x, p.y
                    );
                    int entryId = n->ids[i];

                    if (m_distanceQueue.size() < static_cast<size_t>(count)) {
                        m_distanceQueue.emplace(entryDistanceSq, entryId);
                        if (m_distanceQueue.size() == static_cast<size_t>(count)) {
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

    void QueryBuilder::contains(Rectangle& range) {
        m_ids.clear();
        std::stack<int> parents;
        std::stack<int> parentsEntry;

        // Push the root node and initialize entry index to -1
        parents.push(m_rtreeA.getRootNodeId());
        parentsEntry.push(-1);

        while (!parents.empty()) {
            int nodeId = parents.top();
            auto n = m_rtreeA.getNode(nodeId);
            int startIndex = parentsEntry.top() + 1;

            if (!n) {
                std::cerr << "Error: Invalid node with ID " << nodeId << std::endl;
                // Pop both stacks to move on
                parents.pop();
                parentsEntry.pop();
                continue;
            }

            if (!n->isLeaf()) {
                bool intersects = false;
                for (int i = startIndex; i < n->entryCount; i++) {
                    if (n->entriesMinX[i] > range.maxX)
                        break;
                    if (Rectangle::intersects(
                        range.minX, range.minY, range.maxX, range.maxY,
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i]))
                    {
                        // Found an intersecting entry, go deeper
                        parents.push(n->ids[i]);
                        parentsEntry.pop();   // remove old top
                        parentsEntry.push(i); // record where we left off in the parent
                        parentsEntry.push(-1);// initialize child's start index
                        intersects = true;
                        break; // break to process the newly added child node in next iteration
                    }
                }
                if (intersects) {
                    continue; // process the child node in the next iteration
                }
            } else {
                // Leaf node: Check for containment or intersection
                for (int i = 0; i < n->entryCount; i++) {
                    if (Rectangle::intersects(
                        range.minX, range.minY, range.maxX, range.maxY,
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i])) {
                        m_ids.push_back(n->ids[i]);
                    }
                }
            }

            // Done processing this node, pop stacks
            parents.pop();
            parentsEntry.pop();
        }
    }

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
