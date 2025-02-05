#include "RTreeBuilder.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace rtree {

    RTreeBuilder::RTreeBuilder() :
        m_maxNodeEntries(DEFAULT_MAX_NODE_ENTRIES),
        m_minNodeEntries(DEFAULT_MIN_NODE_ENTRIES)
    {
    }

    void RTreeBuilder::bulkLoad(std::vector<Rectangle>& rectangles) {
        // Clear existing tree
        m_nodeMap.clear();
        m_size = 0;
        m_treeHeight = 0;

        // Calculate optimal node fill
        const int optimalFill = (m_maxNodeEntries + m_minNodeEntries) / 2;

        // Build tree bottom-up
        auto leafNodes = createLeafLevel(rectangles, optimalFill);
        std::vector<std::shared_ptr<Node>> currentLevel = leafNodes;
        int currentHeight = 1;

        // Keep creating levels until we can fit all nodes under a single root
        while (currentLevel.size() > m_maxNodeEntries) {
            currentLevel = createNextLevel(currentLevel, optimalFill);
            currentHeight++;
        }

        // Create root node
        m_rootNodeId = getNextNodeId();
        m_root = createNode(currentLevel, currentHeight + 1);
        m_nodeMap[m_rootNodeId] = m_root;

        // Update tree metadata
        m_size = rectangles.size();
        m_treeHeight = currentHeight + 1;
    }

    std::vector<std::shared_ptr<Node>> RTreeBuilder::createLeafLevel(std::vector<Rectangle>& rectangles, int nodeCapacity) {
        std::vector<std::shared_ptr<Node>> leafNodes;

        // Sort by x coordinate
        std::sort(rectangles.begin(), rectangles.end(),
                 [](const Rectangle& a, const Rectangle& b) { return a.minX < b.minX; });

        // Calculate number of vertical slices
        int totalRects = rectangles.size();
        int rectsPerSlice = static_cast<int>(std::sqrt(totalRects));
        int numSlices = (totalRects + rectsPerSlice - 1) / rectsPerSlice;

        // Process each slice
        for (int slice = 0; slice < numSlices; slice++) {
            int start = slice * rectsPerSlice;
            int end = std::min(start + rectsPerSlice, totalRects);

            // Sort slice by y coordinate
            std::sort(rectangles.begin() + start, rectangles.begin() + end,
                     [](const Rectangle& a, const Rectangle& b) { return a.minY < b.minY; });

            // Group rectangles into nodes
            for (int i = start; i < end; i += nodeCapacity) {
                int groupEnd = std::min(i + nodeCapacity, end);
                auto node = createLeafNode(rectangles, i, groupEnd);
                m_nodeMap[node->nodeId] = node;
                leafNodes.push_back(node);
            }
        }

        return leafNodes;
    }

    std::vector<std::shared_ptr<Node>> RTreeBuilder::createNextLevel(std::vector<std::shared_ptr<Node>>& nodes, int nodeCapacity) {
        std::vector<std::shared_ptr<Node>> parentNodes;

        // Sort nodes by x coordinate of their MBR
        std::sort(nodes.begin(), nodes.end(),
                 [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
                     return a->mbrMinX < b->mbrMinX;
                 });

        // Group nodes into parent nodes
        for (size_t i = 0; i < nodes.size(); i += nodeCapacity) {
            size_t end = std::min(i + nodeCapacity, nodes.size());
            std::vector<std::shared_ptr<Node>> children(nodes.begin() + i, nodes.begin() + end);
            auto parent = createNode(children, nodes[i]->level + 1);
            m_nodeMap[parent->nodeId] = parent;
            parentNodes.push_back(parent);
        }

        return parentNodes;
    }

    std::shared_ptr<Node> RTreeBuilder::createNode(const std::vector<std::shared_ptr<Node>>& children, int level) {
        auto node = std::make_shared<Node>(getNextNodeId(), level);

        for (const auto& child : children) {
            node->addEntry(child->mbrMinX, child->mbrMinY, child->mbrMaxX, child->mbrMaxY, child->nodeId);
        }

        return node;
    }

    std::shared_ptr<Node> RTreeBuilder::createLeafNode(const std::vector<Rectangle>& rectangles, int start, int end) {
        auto node = std::make_shared<Node>(getNextNodeId(), 1);

        for (int i = start; i < end; i++) {
            node->addEntry(rectangles[i].minX, rectangles[i].minY,
                          rectangles[i].maxX, rectangles[i].maxY, i);
        }

        return node;
    }

    int RTreeBuilder::getNextNodeId() {
        return m_nextNodeId++;
    }

    std::shared_ptr<Node> RTreeBuilder::getNode(int id) {
        return m_nodeMap.at(id);
    }

    int RTreeBuilder::treeSize() const {
        return m_size;
    }

    int RTreeBuilder::numNodes() const {
        return m_nodeMap.size();
    }

    int RTreeBuilder::getRootNodeId() const {
        return m_rootNodeId;
    }

} // namespace rtree