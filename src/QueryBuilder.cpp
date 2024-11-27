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

    void QueryBuilder::GetRange(Rectangle& range) {
        contains(range);
        printRangeQuery(range, m_ids);
    }

    void QueryBuilder::GetNearestNeighbors(Point& p, int count) {
        nearestN(p, count);
        printNearestNeighbors(p, m_distanceQueue);
    }

    void QueryBuilder::nearestN(const Point &p, const int count) {
        // Return immediately if given an invalid "count" parameter
        if (count <= 0) {
            return;
        }

        m_parents = std::stack<int>();
        m_parents.push(m_rtreeA.m_rootNodeId);

        m_parentsEntry = std::stack<int>();
        m_parentsEntry.push(-1);

        float furthestNeighborDistance = MAXFLOAT;

        while (!m_parents.empty()) {
            int currentNodeId = m_parents.top();
            m_parents.pop();
            m_parentsEntry.pop();

            auto n = m_rtreeA.getNode(currentNodeId);
            if (n == nullptr) {
                continue;
            }

            if (!n->isLeaf()) {
                // Traverse child nodes
                for (int i = 0; i < n->entryCount; i++) {
                    m_parents.push(n->ids[i]);
                    m_parentsEntry.push(i);
                }
            } else {
                // Process leaf node entries
                for (int i = 0; i < n->entryCount; i++) {
                    auto& entry = n->entries[i];
                    float entryDistanceSq = Rectangle::distanceSq(entry.minX, entry.minY,
                                                                  entry.maxX, entry.maxY,
                                                                  p.x, p.y);
                    int entryId = n->ids[i];

                    if (entryDistanceSq <= furthestNeighborDistance) {

                        // Add to the priority queue
                        m_distanceQueue.emplace(entryDistanceSq, entryId);

                        // Maintain the size of the priority queue
                        if (m_distanceQueue.size() > count) {
                            m_distanceQueue.pop();
                        }

                        // Update the furthest neighbor distance
                        if (m_distanceQueue.size() >= count) {
                            furthestNeighborDistance = m_distanceQueue.top().first;
                        }
                    }
                }
            }
        }
    }

    void QueryBuilder::contains(Rectangle& range) {
        // Initialize stacks for traversal
        m_parents = std::stack<int>();
        m_parents.push(m_rtreeA.m_rootNodeId);

        m_parentsEntry = std::stack<int>();
        m_parentsEntry.push(0);  // Start with 0th entry in the root node

        // Traverse the R-tree
        while (!m_parents.empty()) {
            int nodeId = m_parents.top();
            m_parents.pop();

            auto n = m_rtreeA.getNode(nodeId);
            if (!n) {
                std::cerr << "Error: Invalid node with ID " << nodeId << std::endl;
                continue;
            }

            int startIndex = m_parentsEntry.top();
            m_parentsEntry.pop();

            if (!n->isLeaf()) {
                // Process non-leaf nodes: Check for intersections
                for (int i = startIndex; i < n->entryCount; i++) {
                    if (Rectangle::intersects(range.minX, range.minY, range.maxX, range.maxY,
                                               n->entries[i].minX, n->entries[i].minY, n->entries[i].maxX, n->entries[i].maxY)) {
                        // Push child node for further exploration
                        m_parents.push(n->ids[i]);
                        m_parentsEntry.push(0);  // Reset start index for child node
                    }
                }
            } else {
                // Process leaf nodes: Check for containment or intersection
                for (int i = startIndex; i < n->entryCount; i++) {
                    bool isContained = Rectangle::contains(range.minX, range.minY, range.maxX, range.maxY,
                                                           n->entries[i].minX, n->entries[i].minY, n->entries[i].maxX, n->entries[i].maxY);
                    bool isIntersected = Rectangle::intersects(range.minX, range.minY, range.maxX, range.maxY,
                                                               n->entries[i].minX, n->entries[i].minY, n->entries[i].maxX, n->entries[i].maxY);

                    if (isContained || isIntersected) {
                        // Add entry to results if it intersects or is contained
                        m_ids.push_back(n->ids[i]);
                    }
                }
            }
        }
    }


    void QueryBuilder::intersects(Rectangle& rect) {

    }

    std::set<int> QueryBuilder::intersects(Rectangle& r, std::shared_ptr<Node>& n) {

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

    void QueryBuilder::printIntersectedRectangles(const std::set<int>& ids) {

    }
}
