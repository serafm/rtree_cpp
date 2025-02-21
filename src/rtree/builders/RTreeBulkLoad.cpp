#include "RTreeBulkLoad.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <queue>
#include <stack>

namespace rtree {

    RTreeBulkLoad::RTreeBulkLoad(int capacity) : m_capacity(capacity) {}

    void RTreeBulkLoad::bulkLoad(std::vector<Rectangle>& rectangles) {
        // Build leaf level
        m_totalRectangles = static_cast<int>(rectangles.size());
        auto leafNodes = createLeafLevel(rectangles, m_capacity);
        std::vector<std::shared_ptr<Node>> currentLevel = leafNodes;
        int currentHeight = 1;  // Leaf is level 1

        while (currentLevel.size() > m_capacity) {
            currentLevel = createNextLevel(currentLevel, m_capacity);
            currentHeight++;
        }

        if (currentLevel.size() == 1) {
            m_root = currentLevel.front();
            m_rootNodeId = m_root->nodeId;
            treeHeight = currentHeight; // no extra node created
        } else {
            // Otherwise, create a new root node that has these 'currentLevel' nodes as children
            m_root = createNode(currentLevel, currentHeight + 1);
            m_nodeMap[m_root->nodeId] = m_root;
            m_rootNodeId = m_root->nodeId;
            treeHeight = currentHeight + 1;
        }
    }

    std::vector<std::shared_ptr<Node>> RTreeBulkLoad::createLeafLevel(std::vector<Rectangle>& rectangles, int nodeCapacity) {
        std::vector<std::shared_ptr<Node>> leafNodes;

        // Initial sort by minX (rough ordering)
        std::sort(rectangles.begin(), rectangles.end(),
              [](const Rectangle& a, const Rectangle& b) {
                  return a.minX < b.minX;
              });

        int numGroups = std::ceil(std::sqrt(m_totalRectangles / nodeCapacity));
        int groupSize = m_totalRectangles / numGroups;  // integer division

        for (int g = 0; g < numGroups; g++) {
            int start = g * groupSize;
            int end = std::min(start + groupSize, m_totalRectangles);

            // Secondary sort by minY to group spatially
            std::sort(rectangles.begin() + start, rectangles.begin() + end,
                  [](const Rectangle& a, const Rectangle& b) {
                      return a.minY < b.minY;
                  });

            // Now, partition the group into leaf nodes, but re-sort each node's entries by minX.
            for (int i = start; i < end; i += nodeCapacity) {
                int nodeEnd = std::min(i + nodeCapacity, end);
                auto node = createLeafNode(rectangles, i, nodeEnd);
                m_nodeMap[node->nodeId] = node;
                leafNodes.push_back(node);
            }
        }

        return leafNodes;
    }
    
    std::vector<std::shared_ptr<Node>> RTreeBulkLoad::createNextLevel(std::vector<std::shared_ptr<Node>>& nodes, int nodeCapacity) {
        std::vector<std::shared_ptr<Node>> parentNodes;
        int totalNodes = static_cast<int>(nodes.size());

        if (totalNodes == 0) return parentNodes;

        // Primary sort by the x-coordinate of the MBR.
        std::sort(nodes.begin(), nodes.end(),
                  [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
                      return a->mbrMinX < b->mbrMinX;
                  });

        int numGroups = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(totalNodes) / nodeCapacity)));
        int groupSize = (totalNodes + numGroups - 1) / numGroups;

        for (int g = 0; g < numGroups; g++) {
            int start = g * groupSize;
            int end = std::min(start + groupSize, totalNodes);

            // Secondary sort by minY for grouping.
            std::sort(nodes.begin() + start, nodes.begin() + end,
                  [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
                      return a->mbrMinY < b->mbrMinY;
                  });

            // Now, for each parent node, re-sort its entries by minX.
            for (int i = start; i < end; i += nodeCapacity) {
                int nodeEnd = std::min(i + nodeCapacity, end);
                std::vector<std::shared_ptr<Node>> children(nodes.begin() + i, nodes.begin() + nodeEnd);
                int childLevel = children.front()->level;  // All children have the same level
                auto parent = createNode(children, childLevel + 1);
                m_nodeMap[parent->nodeId] = parent;
                parentNodes.push_back(parent);
            }
        }

        return parentNodes;
    }

    std::shared_ptr<Node> RTreeBulkLoad::createNode(std::vector<std::shared_ptr<Node>>& children, int level) {
        auto node = std::make_shared<Node>(getNextNodeId(), level, m_capacity);
        for (auto& child : children) {
            node->addChildEntry(child);
        }
        node->sortChildrenByMinX();
        return node;
    }

    std::shared_ptr<Node> RTreeBulkLoad::createLeafNode(const std::vector<Rectangle>& rectangles, int start, int end) {
        auto node = std::make_shared<Node>(getNextNodeId(), 1, m_capacity);
        for (int i = start; i < end; i++) {
            node->addLeafEntry(rectangles[i]);
        }
        node->sortLeafsByMinX();
        return node;
    }

    int RTreeBulkLoad::getNextNodeId() {
        return m_nextNodeId++;
    }

    std::shared_ptr<Node> RTreeBulkLoad::getNode(int id) {
        return m_nodeMap.at(id);
    }

    int RTreeBulkLoad::treeSize() const {
        return m_totalRectangles;
    }

    int RTreeBulkLoad::numNodes() const {
        return m_nodeMap.size();
    }

    int RTreeBulkLoad::getRootNodeId() const {
        return m_rootNodeId;
    }

    // Queries

    void RTreeBulkLoad::getLeafs(int nodeId, std::vector<int>& leafs) {
        auto node = m_nodeMap.at(nodeId);

        if (node->isLeaf()) {
            leafs.insert(leafs.end(), node->ids.begin(), node->ids.end());
        } else {
            for (const auto& child : node->children) {
                getLeafs(child->nodeId, leafs);
            }
        }
    }

    void RTreeBulkLoad::range(const Rectangle& range) {
        std::vector<int> m_ids;
        std::queue<std::shared_ptr<Node>> nodeQueue;

        const float minX = range.minX;
        const float minY = range.minY;
        const float maxX = range.maxX;
        const float maxY = range.maxY;

        if (!m_root) return;  // Prevent null root

        nodeQueue.push(m_root);

        while (!nodeQueue.empty()) {
            auto n = nodeQueue.front();
            nodeQueue.pop();

            if (!intersects(minX, minY, maxX, maxY, n->mbrMinX, n->mbrMinY, n->mbrMaxX, n->mbrMaxY))
                continue;

            if (!n->isLeaf()) {
                if (Rectangle::contains(minX, minY, maxX, maxY, n->mbrMinX, n->mbrMinY, n->mbrMaxX, n->mbrMaxY)) {
                    getLeafs(n->nodeId, m_ids);
                    continue;
                }

                for (const auto& child : n->children) {
                    if (child->mbrMinX > maxX) {
                        break;
                    }

                    if (child && intersects(minX, minY, maxX, maxY, child->mbrMinX, child->mbrMinY, child->mbrMaxX, child->mbrMaxY)) {
                        nodeQueue.push(child);
                    }
                }
                continue;
            }
            for (const auto& leaf : n->leafs) {
                if (leaf.minX > maxX) {
                    break;
                }

                if (leaf.intersects(minX, minY, maxX, maxY)) {
                    m_ids.push_back(leaf.id);
                }
            }
        }
       //std::cout << "Num of ids: " << m_ids.size() << std::endl;
    }

    void RTreeBulkLoad::nearestN(const Point &p, const int count) {
        std::priority_queue<std::pair<float, int>> m_distanceQueue;
        if (count <= 0) {
            return;
        }

        // Clear previously stored results.
        //m_distanceQueue = std::priority_queue<std::pair<float, int>>();

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
        int rootNodeId = getRootNodeId();
        auto rootNode = getNode(rootNodeId);
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

            auto n = getNode(nodeId);
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
        while (!m_distanceQueue.empty()) {
            auto [dist, id] = m_distanceQueue.top();
            std::cout << id << " : " << dist << std::endl;
            m_distanceQueue.pop();
        }
    }

} // namespace rtree