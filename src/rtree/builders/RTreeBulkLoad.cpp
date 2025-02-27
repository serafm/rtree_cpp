#include "RTreeBulkLoad.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <queue>
#include <stack>
#include <fstream>

namespace rtree {

    RTreeBulkLoad::RTreeBulkLoad(int capacity) : m_capacity(capacity) {}

    void RTreeBulkLoad::bulkLoad(std::vector<Rectangle>& rectangles) {
        m_totalRectangles = static_cast<int>(rectangles.size());
        auto leafNodes = createLeafLevel(rectangles, m_capacity);
        std::vector<Node*> currentLevel = leafNodes;
        int currentHeight = 1;

        while (currentLevel.size() > m_capacity) {
            currentLevel = createNextLevel(currentLevel, m_capacity);
            currentHeight++;
        }

        if (currentLevel.size() == 1) {
            m_root = currentLevel.front();
            m_rootNodeId = m_root->nodeId;
            treeHeight = currentHeight;
        } else {
            m_root = createNode(currentLevel, currentHeight + 1);
            m_rootNodeId = m_root->nodeId;
            treeHeight = currentHeight + 1;
        }
    }

    std::vector<Node*> RTreeBulkLoad::createLeafLevel(std::vector<Rectangle>& rectangles, int nodeCapacity) {
        std::vector<Node*> leafNodes;

        // Initial sort by minX (rough ordering)
        std::sort(rectangles.begin(), rectangles.end(),
              [](const Rectangle& a, const Rectangle& b) {
                  return a.minX < b.minX;
              });

        int numOfLeafs = std::ceil(m_totalRectangles / (double) nodeCapacity);
        int numGroups = (numOfLeafs / (double) std::sqrt(numOfLeafs));
        int groupSize = std::ceil((double)std::sqrt(numOfLeafs))*nodeCapacity;

        for (int j = 0; j < numGroups; j++) {
            int start = j * groupSize;
            int end = std::min(start + groupSize, m_totalRectangles);

            // Secondary sort by minY to group spatially
            std::sort(rectangles.begin() + start, rectangles.begin() + end,
                  [](const Rectangle& a, const Rectangle& b) {
                      return a.minY < b.minY;
                  });

            // Now, partition the group into leaf nodes
            for (int i = start; i < end; i += nodeCapacity) {
                int nodeEnd = std::min(i + nodeCapacity, end);
                auto node = createLeafNode(rectangles, i, nodeEnd);
                leafNodes.push_back(node);
            }
        }
        return leafNodes;
    }

    std::vector<Node*> RTreeBulkLoad::createNextLevel(std::vector<Node*>& nodes, int nodeCapacity) {
        std::vector<Node*> parentNodes;
        int totalNodes = static_cast<int>(nodes.size());

        if (totalNodes == 0) return parentNodes;

        // Primary sort by the x-coordinate of the MBR.
        std::sort(nodes.begin(), nodes.end(),
                  [](const Node* a, const Node* b) {
                      return a->mbrMinX < b->mbrMinX;
                  });

        int numGroups = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(totalNodes) / nodeCapacity)));
        int groupSize = (totalNodes + numGroups - 1) / numGroups;

        for (int g = 0; g < numGroups; g++) {
            int start = g * groupSize;
            int end = std::min(start + groupSize, totalNodes);

            // Secondary sort by minY for grouping.
            std::sort(nodes.begin() + start, nodes.begin() + end,
                  [](const Node* a, const Node* b) {
                      return a->mbrMinY < b->mbrMinY;
                  });

            // Now, for each parent node, re-sort its entries by minX.
            for (int i = start; i < end; i += nodeCapacity) {
                int nodeEnd = std::min(i + nodeCapacity, end);
                std::vector<Node*> children(nodes.begin() + i, nodes.begin() + nodeEnd);
                int childLevel = children.front()->level;  // All children have the same level
                auto parent = createNode(children, childLevel + 1);
                parentNodes.push_back(parent);
            }
        }
        return parentNodes;
    }

    Node* RTreeBulkLoad::createNode(std::vector<Node*>& children, int level) {
        auto node = new Node(getNextNodeId(), level, m_capacity);
        for (const auto child : children) {
            node->addChildEntry(child);
        }
        node->sortChildrenByMinX();
        return node;
    }

    Node* RTreeBulkLoad::createLeafNode(const std::vector<Rectangle>& rectangles, int start, int end) {
        auto node = new Node(getNextNodeId(), 1, m_capacity);
        for (int i = start; i < end; i++) {
            node->addLeafEntry(rectangles[i]);
        }
        node->sortLeafsByMinX();
        return node;
    }

    int RTreeBulkLoad::getNextNodeId() {
        return m_nextNodeId++;
    }

    int RTreeBulkLoad::treeSize() const {
        return m_totalRectangles;
    }

    // Queries

    void RTreeBulkLoad::getLeafs(Node* node, std::vector<int>& leafs) {
        
        if (node->isLeaf()) {
            leafs.insert(leafs.end(), node->ids.begin(), node->ids.end());
        }
        else {
            for (auto child : node->children) {
                getLeafs(child, leafs);
            }
        }
    }

void RTreeBulkLoad::range(const Rectangle& r) {
        std::vector<int> m_ids;
        std::stack<Node*> nodeStack;

        const float minX = r.minX;
        const float minY = r.minY;
        const float maxX = r.maxX;
        const float maxY = r.maxY;

        nodeStack.push(m_root);

        while (!nodeStack.empty()) {
            const auto n = nodeStack.top();
            nodeStack.pop();

            if (!intersects(minX, minY, maxX, maxY,
                n->mbrMinX, n->mbrMinY, n->mbrMaxX, n->mbrMaxY))
                continue;

            if (!n->isLeaf()) {
                if (Rectangle::contains(minX, minY, maxX, maxY,
                    n->mbrMinX, n->mbrMinY, n->mbrMaxX, n->mbrMaxY))
                {
                    getLeafs(n, m_ids);
                    continue;
                }

                for (auto child : n->children) {

                    if(child->mbrMaxX < minX)
                        continue;

                    if (child->mbrMinX > maxX) {
                        break;
                    }

                    if (intersects(minX, minY, maxX, maxY,
                        child->mbrMinX, child->mbrMinY, child->mbrMaxX, child->mbrMaxY))
                    {
                        nodeStack.push(child);
                    }
                }
                continue;
            }

            uint32_t size = m_ids.size();
            m_ids.resize(size+m_capacity);

            int currentLeaf = 0, currentRange = 0;
            uint32_t totalLeafs = n->leafs.size();

            while(currentLeaf < totalLeafs && currentRange < 1){
                if (minX < n->leafs[currentLeaf].minX){
                    sweepLeafsNext(r, n->leafs, currentLeaf, totalLeafs, m_ids, size);
                    currentRange++;
                }else{
                    sweepLeafs(n->leafs[currentLeaf], r, currentRange, 1, m_ids, size);
                    currentLeaf++;
                }
            }

           m_ids.resize(size);
        }
        //std::cout << m_ids.size() << std::endl;
        // Write results to a file
        /*std::ofstream outFile("/home/serafm/dev/rtree_cpp/range_results.txt", std::ios::app);
        if (!outFile) {
            std::cerr << "Error: Unable to open file for writing results!" << std::endl;
            return;
        }
        outFile << m_ids.size() << "\n";*/
    }

    void RTreeBulkLoad::sweepLeafs(Rectangle& leaf, const Rectangle& rangeQ, int start, int size, std::vector<int>& results, uint32_t& res_size){
        int counter = start;
        
        while(counter < size && leaf.maxX >= rangeQ.minX){
            if (leaf.minY > rangeQ.maxY || leaf.maxY < rangeQ.minY){
                counter++;
                continue;
            }
            results[res_size++] = leaf.id;
            counter++;
        }
    }

    void RTreeBulkLoad::sweepLeafsNext(const Rectangle& rangeQ, std::vector<Rectangle>& leafs, int start, int size, std::vector<int>& results, uint32_t& res_size){
        int counter = start;
        
        while (counter < size && rangeQ.maxX >= leafs[counter].minX){
            if (rangeQ.minY > leafs[counter].maxY || rangeQ.maxY < leafs[counter].minY) {
                counter++;
                continue;
            }
            results[res_size++] = leafs[counter].id;
            counter++;
        }
    }

    void RTreeBulkLoad::nearestN(const Point &p, const int count) {
        std::priority_queue<std::pair<float, int>> m_distanceQueue;
        const float qx = p.x;
        const float qy = p.y;

        float furthestNeighborDistance = MAXFLOAT;

        // A min-heap for nodes based on their bounding box distance to the query point.
        using NodePair = std::pair<float, Node*>;
        std::priority_queue<
            NodePair,
            std::vector<NodePair>,
            std::greater<NodePair>
        > nodeQueue;

        nodeQueue.emplace(MAXFLOAT, m_root);

        // Best-first search.
        while (!nodeQueue.empty()) {
            const auto [dist, n] = nodeQueue.top();
            nodeQueue.pop();

            // Exit if no more nodes smaller than the maximum already in queue
            if (m_distanceQueue.size() == count && dist >= furthestNeighborDistance) {
                break;
            }

            if (!n->isLeaf()) {
                // For internal nodes, push children into the nodeQueue.
                for (const auto child : n->children) {
                    float childDist = Rectangle::distance(
                        child->mbrMinX, child->mbrMinY,
                        child->mbrMaxX, child->mbrMaxY,
                        qx, qy
                    );
                    nodeQueue.emplace(childDist, child);
                }
                continue;
            }
            // For leaf nodes, process each entry.
            for (const auto leaf : n->leafs) {
                const float entryDistance = Rectangle::distance(
                    leaf.minX, leaf.minY, leaf.maxX, leaf.maxY,
                    qx, qy
                );

                uint32_t queue_size = m_distanceQueue.size();
                if (queue_size < count) {
                    m_distanceQueue.emplace(entryDistance, leaf.id);
                    if (queue_size == count) {
                        furthestNeighborDistance = m_distanceQueue.top().first;
                    }
                } else if (entryDistance < m_distanceQueue.top().first) {
                    m_distanceQueue.pop();
                    m_distanceQueue.emplace(entryDistance, leaf.id);
                    furthestNeighborDistance = entryDistance;
                }
            }
        }
        /*while (!m_distanceQueue.empty()) {
            auto [dist, id] = m_distanceQueue.top();
            std::cout << id << " : " << dist << std::endl;
            m_distanceQueue.pop();
        }*/
    }

    void RTreeBulkLoad::join(RTreeBulkLoad& rtreeB) {
        std::map<int, std::vector<int>> m_joinRectangles;
        std::stack<std::pair<Node*, Node*>> nodePairs;
        nodePairs.emplace(this->m_root, rtreeB.m_root);

        while (!nodePairs.empty()) {
            auto [nodeA, nodeB] = nodePairs.top();
            nodePairs.pop();

            // Prune if the two MBRs do not intersect.
            if (!intersects(
                    nodeA->mbrMinX, nodeA->mbrMinY, nodeA->mbrMaxX, nodeA->mbrMaxY,
                    nodeB->mbrMinX, nodeB->mbrMinY, nodeB->mbrMaxX, nodeB->mbrMaxY))
            {
                continue;
            }

            // Case 1: Both nodes are leaves – do pairwise comparisons.
            if (nodeA->isLeaf() && nodeB->isLeaf()) {
                for (const auto leafA : nodeA->leafs) {
                    for (const auto leafB : nodeB->leafs) {
                        if (intersects(
                                leafA.minX, leafA.minY, leafA.maxX, leafA.maxY,
                                leafB.minX, leafB.minY, leafB.maxX, leafB.maxY))
                        {
                            m_joinRectangles[leafA.id].push_back(leafB.id);
                        }
                    }
                }
            }
            // Case 2: Both nodes are internal.
            else if (!nodeA->isLeaf() && !nodeB->isLeaf()) {
                // For each child of nodeA, use binary search in nodeB's sorted entries.
                // Note: This assumes nodeB's entries are sorted by minX.
                for (const auto childA : nodeA->children) {
                    // Scan from low until nodeB’s child's minX is beyond childA’s maxX.
                    for (const auto childB : nodeB->children) {
                        if (childB->mbrMinX > childA->mbrMaxX)
                            break;
                        if (intersects(
                                childA->mbrMinX, childA->mbrMinY, childA->mbrMaxX, childA->mbrMaxY,
                                childB->mbrMinX, childB->mbrMinY, childB->mbrMaxX, childB->mbrMaxY))
                        {
                            nodePairs.emplace(childA, childB);
                        }
                    }
                }
            }
            // Case 3: nodeA is internal, nodeB is a leaf.
            else if (!nodeA->isLeaf() && nodeB->isLeaf()) {
                for (const auto childA : nodeA->children) {
                    if (childA->mbrMinX > nodeB->mbrMaxX)
                        break;
                    if (intersects(
                            childA->mbrMinX, childA->mbrMinY, childA->mbrMaxX, childA->mbrMaxY,
                            nodeB->mbrMinX, nodeB->mbrMinY, nodeB->mbrMaxX, nodeB->mbrMaxY))
                    {
                        nodePairs.emplace(childA, nodeB);
                    }
                }
            }
            // Case 4: nodeA is a leaf, nodeB is internal.
            else {
                for (const auto childB : nodeB->children) {
                    if (childB->mbrMinX > nodeA->mbrMaxX)
                        break;
                    if (intersects(
                            nodeA->mbrMinX, nodeA->mbrMinY, nodeA->mbrMaxX, nodeA->mbrMaxY,
                            childB->mbrMinX, childB->mbrMinY, childB->mbrMaxX, childB->mbrMaxY))
                    {
                        nodePairs.emplace(nodeA, childB);
                    }
                }
            }
        }
    }

} // namespace rtree