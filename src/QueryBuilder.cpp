#include "QueryBuilder.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <queue>
#include <set>
#include <vector>

#include "rtree/RTreeBuilder.h"

namespace rtree {

    QueryBuilder::QueryBuilder(RTreeBuilder& rtreeA) : m_rtreeA(rtreeA), m_rtreeB({}) {}
    QueryBuilder::QueryBuilder(RTreeBuilder& rtreeA, RTreeBuilder& rtreeB) : m_rtreeA(rtreeA), m_rtreeB(rtreeB) {}

    void QueryBuilder::Range(Rectangle& range) {
        contains(range);
        printRangeQuery(range, m_ids);
    }

    void QueryBuilder::NearestNeighbors(Point& p, int count) {
        nearestN(p, count);
        printNearestNeighbors(p, m_distanceQueue);
    }

    void QueryBuilder::Join() {
        join(m_rtreeA, m_rtreeB);
        printJoinQuery();
    }

    // Join function implementation
    void QueryBuilder::join(RTreeBuilder& rtreeA, RTreeBuilder& rtreeB) {
        m_joinRectangles.clear();

        // Start from the root nodes of both R-trees
        auto rootA = rtreeA.getNode(rtreeA.getRootNodeId());
        auto rootB = rtreeB.getNode(rtreeB.getRootNodeId());

        // Stack of node pairs to be processed
        std::stack<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>> stack;
        stack.push({rootA, rootB});

        while (!stack.empty()) {
            auto [nodeA, nodeB] = stack.top();
            stack.pop();

            // Check if nodes intersect at the MBR level
            if (!Rectangle::intersects(
                    nodeA->mbrMinX, nodeA->mbrMinY, nodeA->mbrMaxX, nodeA->mbrMaxY,
                    nodeB->mbrMinX, nodeB->mbrMinY, nodeB->mbrMaxX, nodeB->mbrMaxY)) {
                continue; // No intersection, prune
            }

            // If both nodes are leaves, do a pairwise check of their entries
            if (nodeA->isLeaf() && nodeB->isLeaf()) {
                for (int i = 0; i < nodeA->entryCount; i++) {
                    for (int j = 0; j < nodeB->entryCount; j++) {
                        if (Rectangle::intersects(
                                nodeA->entriesMinX[i], nodeA->entriesMinY[i], nodeA->entriesMaxX[i], nodeA->entriesMaxY[i],
                                nodeB->entriesMinX[j], nodeB->entriesMinY[j], nodeB->entriesMaxX[j], nodeB->entriesMaxY[j])) {
                            int idA = nodeA->ids[i];
                            int idB = nodeB->ids[j];
                            m_joinRectangles[idA].push_back(idB);
                        }
                    }
                }
            } else if (!nodeA->isLeaf() && !nodeB->isLeaf()) {
                // If both are internal nodes, compare their children
                for (int i = 0; i < nodeA->entryCount; i++) {
                    auto childA = rtreeA.getNode(nodeA->ids[i]);
                    for (int j = 0; j < nodeB->entryCount; j++) {
                        auto childB = rtreeB.getNode(nodeB->ids[j]);

                        // Only push if bounding boxes intersect
                        if (Rectangle::intersects(
                                childA->mbrMinX, childA->mbrMinY, childA->mbrMaxX, childA->mbrMaxY,
                                childB->mbrMinX, childB->mbrMinY, childB->mbrMaxX, childB->mbrMaxY)) {
                            stack.push({childA, childB});
                        }
                    }
                }
            } else if (!nodeA->isLeaf() && nodeB->isLeaf()) {
                // nodeA is internal, nodeB is leaf
                for (int i = 0; i < nodeA->entryCount; i++) {
                    auto childA = rtreeA.getNode(nodeA->ids[i]);
                    if (Rectangle::intersects(
                            childA->mbrMinX, childA->mbrMinY, childA->mbrMaxX, childA->mbrMaxY,
                            nodeB->mbrMinX, nodeB->mbrMinY, nodeB->mbrMaxX, nodeB->mbrMaxY)) {
                        stack.push({childA, nodeB});
                    }
                }
            } else {
                // nodeA is leaf, nodeB is internal
                for (int j = 0; j < nodeB->entryCount; j++) {
                    auto childB = rtreeB.getNode(nodeB->ids[j]);
                    if (Rectangle::intersects(
                            nodeA->mbrMinX, nodeA->mbrMinY, nodeA->mbrMaxX, nodeA->mbrMaxY,
                            childB->mbrMinX, childB->mbrMinY, childB->mbrMaxX, childB->mbrMaxY)) {
                        stack.push({nodeA, childB});
                    }
                }
            }
        }
    }

    void QueryBuilder::nearestN(const Point &p, const int count) {
        // Return immediately if given an invalid "count" parameter
        if (count <= 0) {
            return;
        }

        // Stack to manage traversal: each element is a pair of node ID and the index of the last child node processed
        std::stack<std::pair<int, int>> traversalStack;
        traversalStack.push({m_rtreeA.getRootNodeId(), -1});

        float furthestNeighborDistance = MAXFLOAT;

        while (!traversalStack.empty()) {
            auto [currentNodeId, startIndex] = traversalStack.top();
            traversalStack.pop();

            auto n = m_rtreeA.getNode(currentNodeId);
            if (n == nullptr) {
                continue;
            }

            if (!n->isLeaf()) {
                // Traverse child nodes starting from startIndex + 1
                for (int i = startIndex + 1; i < n->entryCount; i++) {
                    float distanceSq = Rectangle::distanceSq(
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i],
                        p.x, p.y);

                    // Consider this child if it could contain closer neighbors
                    if (m_distanceQueue.size() < count || distanceSq <= furthestNeighborDistance) {
                        // Push the current node back onto the stack with updated startIndex
                        traversalStack.push({currentNodeId, i});

                        // Push child node onto the stack with startIndex -1
                        traversalStack.push({n->ids[i], -1});

                        // Break to process the child node next
                        break;
                    }
                }
            } else {
                // Process leaf node entries
                for (int i = 0; i < n->entryCount; i++) {
                    float entryDistanceSq = Rectangle::distanceSq(
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i],
                        p.x, p.y);
                    int entryId = n->ids[i];

                    if (m_distanceQueue.size() < count) {
                        // Add to the priority queue
                        m_distanceQueue.emplace(entryDistanceSq, entryId);

                        // Update the furthest neighbor distance if we've reached the desired count
                        if (m_distanceQueue.size() == count) {
                            furthestNeighborDistance = m_distanceQueue.top().first;
                        }
                    } else if (entryDistanceSq < furthestNeighborDistance) {
                        // Replace the furthest neighbor with the closer one
                        m_distanceQueue.pop();
                        m_distanceQueue.emplace(entryDistanceSq, entryId);
                        furthestNeighborDistance = m_distanceQueue.top().first;
                    }
                }
            }
        }
    }

    void QueryBuilder::contains(Rectangle& range) {
        // Initialize stack for traversal
        std::stack<int> nodeStack;
        nodeStack.push(m_rtreeA.getRootNodeId());

        m_parentsEntry = std::stack<int>();
        m_parentsEntry.push(-1);

        // Traverse the R-tree
        while (!nodeStack.empty()) {
            int nodeId = nodeStack.top();
            nodeStack.pop();
            int startIndex = m_parentsEntry.top() + 1;

            auto n = m_rtreeA.getNode(nodeId);

            if (!n) {
                std::cerr << "Error: Invalid node with ID " << nodeId << std::endl;
                continue;
            }

            if (!n->isLeaf()) {
                // Process non-leaf nodes: Check for intersections
                for (int i = startIndex; i < n->entryCount; i++) {
                    if (Rectangle::intersects(
                        range.minX, range.minY, range.maxX, range.maxY,
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i])) {

                        // Push child node for further exploration
                        nodeStack.push(n->ids[i]);
                        }
                }
            } else {
                // Process leaf nodes: Check for containment or intersection
                for (int i = 0; i < n->entryCount; i++) {
                    bool isContained = Rectangle::contains(
                        range.minX, range.minY, range.maxX, range.maxY,
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i]);
                    bool isIntersected = Rectangle::intersects(
                        range.minX, range.minY, range.maxX, range.maxY,
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i]);

                    if (isContained || isIntersected) {
                        // Add entry to results if it intersects or is contained
                        m_ids.push_back(n->ids[i]);
                    }
                }
            }
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
