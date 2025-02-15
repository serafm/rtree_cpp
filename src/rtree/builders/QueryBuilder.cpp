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
        m_joinRectangles.clear(); // Clear previous join results to prepare for a fresh spatial join

        // Start from the root nodes of both R-trees
        auto rootA = rtreeA.getNode(rtreeA.getRootNodeId());
        auto rootB = rtreeB.getNode(rtreeB.getRootNodeId());

        // Stack for iterative traversal of node pairs from both R-trees
        std::stack<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>> nodePairs;
        nodePairs.push({rootA, rootB}); // Initialize the nodePairs with the root nodes of both trees

        // Depth-first traversal of the R-tree pairs
        while (!nodePairs.empty()) {
            auto [nodeA, nodeB] = nodePairs.top(); // Get the top pair of nodes from the nodePairs
            nodePairs.pop(); // Remove the processed pair from the nodePairs

            // Check if the MBRs of the current pair of nodes intersect
            if (!Rectangle::intersects(
                    nodeA->mbrMinX, nodeA->mbrMinY, nodeA->mbrMaxX, nodeA->mbrMaxY,
                    nodeB->mbrMinX, nodeB->mbrMinY, nodeB->mbrMaxX, nodeB->mbrMaxY)) {
                continue; // If no intersection, prune the pair and move on
            }

            // Case 1: Both nodes are leaves
            if (nodeA->isLeaf() && nodeB->isLeaf()) {
                // Perform pairwise comparisons of all entries in both nodes
                for (int i = 0; i < nodeA->entryCount; i++) {
                    for (int j = 0; j < nodeB->entryCount; j++) {
                        // Check if the rectangles of the two entries intersect
                        if (Rectangle::intersects(
                                nodeA->entriesMinX[i], nodeA->entriesMinY[i], nodeA->entriesMaxX[i], nodeA->entriesMaxY[i],
                                nodeB->entriesMinX[j], nodeB->entriesMinY[j], nodeB->entriesMaxX[j], nodeB->entriesMaxY[j])) {
                            // Add the intersecting pair (ID of entryA, ID of entryB) to the results
                            int idA = nodeA->ids[i];
                            int idB = nodeB->ids[j];
                            m_joinRectangles[idA].push_back(idB);
                        }
                    }
                }
            }
            // Case 2: Both nodes are internal (non-leaf) nodes
            else if (!nodeA->isLeaf() && !nodeB->isLeaf()) {
                // Compare each child node of nodeA with each child node of nodeB
                for (int i = 0; i < nodeA->entryCount; i++) {
                    auto childA = rtreeA.getNode(nodeA->ids[i]); // Get child node from R-tree A
                    for (int j = 0; j < nodeB->entryCount; j++) {
                        auto childB = rtreeB.getNode(nodeB->ids[j]); // Get child node from R-tree B

                        // Push the pair onto the nodePairs if their bounding boxes intersect
                        if (Rectangle::intersects(
                                childA->mbrMinX, childA->mbrMinY, childA->mbrMaxX, childA->mbrMaxY,
                                childB->mbrMinX, childB->mbrMinY, childB->mbrMaxX, childB->mbrMaxY)) {
                            nodePairs.push({childA, childB});
                        }
                    }
                }
            }
            // Case 3: NodeA is internal, NodeB is a leaf
            else if (!nodeA->isLeaf() && nodeB->isLeaf()) {
                // Compare each child node of nodeA with the leaf nodeB
                for (int i = 0; i < nodeA->entryCount; i++) {
                    auto childA = rtreeA.getNode(nodeA->ids[i]); // Get child node from R-tree A
                    // Push the pair onto the nodePairs if their bounding boxes intersect
                    if (Rectangle::intersects(
                            childA->mbrMinX, childA->mbrMinY, childA->mbrMaxX, childA->mbrMaxY,
                            nodeB->mbrMinX, nodeB->mbrMinY, nodeB->mbrMaxX, nodeB->mbrMaxY)) {
                        nodePairs.push({childA, nodeB});
                    }
                }
            }
            // Case 4: NodeA is a leaf, NodeB is internal
            else {
                // Compare the leaf nodeA with each child node of nodeB
                for (int j = 0; j < nodeB->entryCount; j++) {
                    auto childB = rtreeB.getNode(nodeB->ids[j]); // Get child node from R-tree B
                    // Push the pair onto the nodePairs if their bounding boxes intersect
                    if (Rectangle::intersects(
                            nodeA->mbrMinX, nodeA->mbrMinY, nodeA->mbrMaxX, nodeA->mbrMaxY,
                            childB->mbrMinX, childB->mbrMinY, childB->mbrMaxX, childB->mbrMaxY)) {
                        nodePairs.push({nodeA, childB});
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

        // Clear previously stored results
        m_distanceQueue = std::priority_queue<std::pair<float, int>>();

        float furthestNeighborDistance = MAXFLOAT;

        // A min-heap for nodes based on their bounding box distance to the query point
        // We store pairs of (distance, nodeId), and use std::greater<> to get the smallest distance on top
        std::priority_queue<
            std::pair<float,int>,
            std::vector<std::pair<float,int>>,
            std::greater<std::pair<float,int>>
        > nodeQueue;

        // Start by pushing the root node
        int rootNodeId = m_rtreeA.getRootNodeId();
        auto rootNode = m_rtreeA.getNode(rootNodeId);
        if (!rootNode) {
            return; // no root node
        }

        // Compute bounding box of the root node to measure its distance from point p
        // Typically the root node bounding box is either stored or we can derive it from its entries.
        float nodeMinX = MAXFLOAT;
        float nodeMinY = MAXFLOAT;
        float nodeMaxX = -MAXFLOAT;
        float nodeMaxY = -MAXFLOAT;

        for (int i = 0; i < rootNode->entryCount; i++) {
            nodeMinX = std::min(nodeMinX, rootNode->entriesMinX[i]);
            nodeMinY = std::min(nodeMinY, rootNode->entriesMinY[i]);
            nodeMaxX = std::max(nodeMaxX, rootNode->entriesMaxX[i]);
            nodeMaxY = std::max(nodeMaxY, rootNode->entriesMaxY[i]);
        }

        float rootDist = Rectangle::distanceSq(nodeMinX, nodeMinY, nodeMaxX, nodeMaxY, p.x, p.y);
        nodeQueue.push({rootDist, rootNodeId});

        // Best-first search loop
        while (!nodeQueue.empty()) {
            // Get the node with the smallest bounding box distance
            auto [dist, nodeId] = nodeQueue.top();
            nodeQueue.pop();

            // If this distance is already greater than the furthest neighbor we have found,
            // this node cannot yield better results, so skip it.
            if (m_distanceQueue.size() == count && dist > furthestNeighborDistance) {
                continue;
            }

            auto n = m_rtreeA.getNode(nodeId);
            if (!n) {
                continue;
            }

            if (!n->isLeaf()) {
                // Non-leaf node: explore its children
                for (int i = 0; i < n->entryCount; i++) {
                    float childDist = Rectangle::distanceSq(
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i],
                        p.x, p.y
                    );

                    // Only consider pushing this child if it could possibly improve results
                    if (m_distanceQueue.size() < count || childDist <= furthestNeighborDistance) {
                        nodeQueue.push({childDist, n->ids[i]});
                    }
                }
            } else {
                // Leaf node: process actual data entries
                for (int i = 0; i < n->entryCount; i++) {
                    float entryDistanceSq = Rectangle::distanceSq(
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i],
                        p.x, p.y
                    );
                    int entryId = n->ids[i];

                    if (m_distanceQueue.size() < count) {
                        // Add to the priority queue of results (max heap)
                        m_distanceQueue.emplace(entryDistanceSq, entryId);
                        if (m_distanceQueue.size() == count) {
                            furthestNeighborDistance = m_distanceQueue.top().first;
                        }
                    } else if (entryDistanceSq < m_distanceQueue.top().first) {
                        // Replace the worst candidate
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
                    bool isContained = Rectangle::contains(
                        range.minX, range.minY, range.maxX, range.maxY,
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i]);
                    bool isIntersected = Rectangle::intersects(
                        range.minX, range.minY, range.maxX, range.maxY,
                        n->entriesMinX[i], n->entriesMinY[i],
                        n->entriesMaxX[i], n->entriesMaxY[i]);

                    if (isContained || isIntersected) {
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
