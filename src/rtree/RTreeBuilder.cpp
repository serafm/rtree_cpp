#include "RTreeBuilder.h"
#include <iostream>
#include <cmath>

namespace rtree {

    RTreeBuilder::RTreeBuilder() :
        m_initialEntryStatus(DEFAULT_MAX_NODE_ENTRIES),
        m_entryStatus(DEFAULT_MAX_NODE_ENTRIES),
        m_maxNodeEntries(DEFAULT_MAX_NODE_ENTRIES),
        m_minNodeEntries(DEFAULT_MIN_NODE_ENTRIES)
    {
        m_initialEntryStatus.assign(m_maxNodeEntries, ENTRY_STATUS_UNASSIGNED);
        m_root = std::make_shared<Node>(m_rootNodeId, 1);
        m_nodeMap.emplace(m_rootNodeId, m_root);
    }

    void RTreeBuilder::addEntry(const Rectangle & r)
    {
        add(r.minX, r.minY, r.maxX, r.maxY, m_entryId, 1);
        m_entryId++;
        m_size++;
    }

    void RTreeBuilder::add(float minX, float minY, float maxX, float maxY, int id, int level) {
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
            auto root = std::make_shared<Node>(m_rootNodeId, m_treeHeight);

            root->addEntry(newNode->mbrMinX, newNode->mbrMinY, newNode->mbrMaxX, newNode->mbrMaxY, newNode->nodeId);
            root->addEntry(oldRoot->mbrMinX, oldRoot->mbrMinY, oldRoot->mbrMaxX, oldRoot->mbrMaxY, oldRoot->nodeId);

            m_nodeMap.emplace(root->getNodeId(), root);
        }
    }

    std::shared_ptr<Node> RTreeBuilder::splitNode(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId)
    {
        m_entryStatus.assign(m_initialEntryStatus.begin(), m_initialEntryStatus.end());

        auto newNode = std::make_shared<Node>(getNextNodeId(), n->level);
        m_nodeMap.emplace(newNode->nodeId, newNode);

        if (newNode != nullptr) {
            pickSeeds(n, newRectMinX, newRectMinY, newRectMaxX, newRectMaxY, newId, newNode);
        } else {
            std::cerr << "Error: Failed to split node is null" << std::endl;
            std::exit(1);
        }

        while ((n->entryCount + newNode->entryCount) < m_maxNodeEntries + 1) {
            if ((m_maxNodeEntries + 1 - newNode->entryCount) == m_minNodeEntries) {
                for (int i = 0; i < m_maxNodeEntries; i++) {
                    if (m_entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {
                        m_entryStatus[i] = ENTRY_STATUS_ASSIGNED;

                        if (n->entriesMinX[i] < n->mbrMinX) n->mbrMinX = n->entriesMinX[i];
                        if (n->entriesMinY[i] < n->mbrMinY) n->mbrMinY = n->entriesMinY[i];
                        if (n->entriesMaxX[i] > n->mbrMaxX) n->mbrMaxX = n->entriesMaxX[i];
                        if (n->entriesMaxY[i] > n->mbrMaxY) n->mbrMaxY = n->entriesMaxY[i];

                        n->entryCount++;
                    }
                }
                break;
            }
            if ((m_maxNodeEntries + 1 - n->entryCount) == m_minNodeEntries) {
                for (int i = 0; i < m_maxNodeEntries; i++) {
                    if (m_entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {
                        m_entryStatus[i] = ENTRY_STATUS_ASSIGNED;
                        newNode->addEntry(n->entriesMinX[i], n->entriesMinY[i], n->entriesMaxX[i], n->entriesMaxY[i], n->ids[i]);
                        n->ids[i] = -1;
                    }
                }
                break;
            }
            pickNext(n, newNode);
        }
        n->reorganize(m_maxNodeEntries);
        return newNode;
    }

    void RTreeBuilder::pickSeeds(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId, const std::shared_ptr<Node>& newNode) {
        float maxNormalizedSeparation = -1;
        int highestLowIndex = -1;
        int lowestHighIndex = -1;

        if (newRectMinX < n->mbrMinX) n->mbrMinX = newRectMinX;
        if (newRectMinY < n->mbrMinY) n->mbrMinY = newRectMinY;
        if (newRectMaxX > n->mbrMaxX) n->mbrMaxX = newRectMaxX;
        if (newRectMaxY > n->mbrMaxY) n->mbrMaxY = newRectMaxY;

        float mbrLenX = n->mbrMaxX - n->mbrMinX;
        float mbrLenY = n->mbrMaxY - n->mbrMinY;

        // X-Dimension
        float tempHighestLow = newRectMinX;
        int tempHighestLowIndex = -1;

        float tempLowestHigh = newRectMaxX;
        int tempLowestHighIndex = -1;

        for (int i = 0; i < n->entryCount; i++) {
            float tempLow = n->entriesMinX[i];
            if (tempLow >= tempHighestLow) {
                tempHighestLow = tempLow;
                tempHighestLowIndex = i;
            } else {
                float tempHigh = n->entriesMaxX[i];
                if (tempHigh <= tempLowestHigh) {
                    tempLowestHigh = tempHigh;
                    tempLowestHighIndex = i;
                }
            }

            // Ensure no negative separation
            float normalizedSeparation = (mbrLenX == 0.0f) ? 1.0f : ((tempHighestLow - tempLowestHigh) / mbrLenX);

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
            float tempLow = n->entriesMinY[i];
            if (tempLow >= tempHighestLow) {
                tempHighestLow = tempLow;
                tempHighestLowIndex = i;
            } else {
                float tempHigh = n->entriesMaxY[i];
                if (tempHigh <= tempLowestHigh) {
                    tempLowestHigh = tempHigh;
                    tempLowestHighIndex = i;
                }
            }

            // Ensure no negative separation
            float normalizedSeparation = (mbrLenY == 0.0f) ? 1.0f : ((tempHighestLow - tempLowestHigh) / mbrLenY);

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
            float tempMaxX = n->entriesMaxX[0];

            for (int i = 1; i < n->entryCount; i++) {
                if (n->entriesMinY[i] < tempMinY) {
                    tempMinY = n->entriesMinY[i];
                    highestLowIndex = i;
                } else if (n->entriesMaxX[i] > tempMaxX) {
                    tempMaxX = n->entriesMaxX[i];
                    lowestHighIndex = i;
                }
            }
        }

        if (highestLowIndex == -1) {
            newNode->addEntry(
                newRectMinX,
                newRectMinY,
                newRectMaxX,
                newRectMaxY,
                newId);
        } else {
            newNode->addEntry(
                n->entriesMinX[highestLowIndex],
                n->entriesMinY[highestLowIndex],
                n->entriesMaxX[highestLowIndex],
                n->entriesMaxY[highestLowIndex],
                n->ids[highestLowIndex]
                );

            n->ids[highestLowIndex] = -1;

            n->entriesMinX[highestLowIndex] = newRectMinX;
            n->entriesMinY[highestLowIndex] = newRectMinY;
            n->entriesMaxX[highestLowIndex] = newRectMaxX;
            n->entriesMaxY[highestLowIndex] = newRectMaxY;

            n->ids[highestLowIndex] = newId;
        }

        if (lowestHighIndex == -1) {
            lowestHighIndex = highestLowIndex;
        }

        // Update the entry count properly (do not overwrite it)
        m_entryStatus[lowestHighIndex] = ENTRY_STATUS_ASSIGNED;
        n->entryCount = 1;  // Decrease by one after moving an entry to newNode
        n->mbrMinX = n->entriesMinX[lowestHighIndex];
        n->mbrMinY = n->entriesMinY[lowestHighIndex];
        n->mbrMaxX = n->entriesMaxX[lowestHighIndex];
        n->mbrMaxY = n->entriesMaxY[lowestHighIndex];
    }

    int RTreeBuilder::pickNext(const std::shared_ptr<Node>& n, const std::shared_ptr<Node>& newNode) {
        float maxDifference = -MAXFLOAT;
        int next = 0;
        int nextGroup = 0;

        // Ensure there's at least one unassigned entry
        if (n->entryCount == 0) {
            std::cerr << "Error: No entries to pick." << std::endl;
            return -1;
        }

        for (int i = 0; i < m_maxNodeEntries; i++) {
            if (m_entryStatus[i] == ENTRY_STATUS_UNASSIGNED) {
                if (n->ids[i] == -1) {
                    std::cerr << "Error: Node " << n->nodeId << ", entry " << i << " has invalid ID" << std::endl;
                }

                float nIncrease = Rectangle::enlargement(n->mbrMinX, n->mbrMinY, n->mbrMaxX, n->mbrMaxY,
                                                 n->entriesMinX[i], n->entriesMinY[i], n->entriesMaxX[i], n->entriesMaxY[i]);
                float newNodeIncrease = Rectangle::enlargement(newNode->mbrMinX, newNode->mbrMinY, newNode->mbrMaxX, newNode->mbrMaxY,
                                                              n->entriesMinX[i], n->entriesMinY[i], n->entriesMaxX[i], n->entriesMaxY[i]);


                float difference = std::abs(nIncrease - newNodeIncrease);

                if (difference > maxDifference) {
                    next = i;

                    // Adjust group based on enlargement comparison
                    if (nIncrease < newNodeIncrease) {
                        nextGroup = 0;
                    } else if (newNodeIncrease < nIncrease) {
                        nextGroup = 1;
                    } else if (Rectangle::area(n->mbrMinX, n->mbrMinY, n->mbrMaxX, n->mbrMaxY) < Rectangle::area(newNode->mbrMinX, newNode->mbrMinY, newNode->mbrMaxX, newNode->mbrMaxY)) {
                        nextGroup = 0;
                    } else if (Rectangle::area(newNode->mbrMinX, newNode->mbrMinY, newNode->mbrMaxX, newNode->mbrMaxY) < Rectangle::area(n->mbrMinX, n->mbrMinY, n->mbrMaxX, n->mbrMaxY)) {
                        nextGroup = 1;
                    } else if (newNode->entryCount < (m_maxNodeEntries / 2)) {
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
            if (n->entriesMinX[next] < n->mbrMinX) n->mbrMinX = n->entriesMinX[next];
            if (n->entriesMinY[next] < n->mbrMinY) n->mbrMinY = n->entriesMinY[next];
            if (n->entriesMaxX[next] > n->mbrMaxX) n->mbrMaxX = n->entriesMaxX[next];
            if (n->entriesMaxY[next] > n->mbrMaxY) n->mbrMaxY = n->entriesMaxY[next];
            n->entryCount++;
        } else {
            // Move to the new node
            newNode->addEntry(n->entriesMinX[next], n->entriesMinY[next], n->entriesMaxX[next], n->entriesMaxY[next], n->ids[next]);
            n->ids[next] = -1;
        }
        return next;
    }

    std::shared_ptr<Node> RTreeBuilder::chooseNode(float minX, float minY, float maxX, float maxY, int level) {
        auto n = getNode(m_rootNodeId);
        if (!n) {
            std::cerr << "Could not get root node " << m_rootNodeId << std::endl;
            return nullptr;
        }

        m_parents = std::stack<int>();
        m_parentsEntry = std::stack<int>();

        while (true) {
            if (n == nullptr) {
                std::cerr << "Could not get root node (" + std::to_string(m_rootNodeId) + ")" << std::endl;
            }

            if (n->level == level) {
                return n;
            }

            float leastEnlargement = Rectangle::enlargement(n->entriesMinX[0], n->entriesMinY[0], n->entriesMaxX[0], n->entriesMaxY[0],
                                                     minX, minY, maxX, maxY);
            int index = 0;

            for (int i = 0; i < n->entryCount; ++i) {
                float tempMinX = n->entriesMinX[i];
                float tempMinY = n->entriesMinY[i];
                float tempMaxX = n->entriesMaxX[i];
                float tempMaxY = n->entriesMaxY[i];

                float tempEnlargement = Rectangle::enlargement(tempMinX, tempMinY, tempMaxX, tempMaxY,
                                                                     minX, minY, maxX, maxY);

                if ((tempEnlargement < leastEnlargement) ||
                    ((tempEnlargement == leastEnlargement) &&
                        (Rectangle::area(tempMinX, tempMinY, tempMaxX, tempMaxY) <
                        Rectangle::area(n->entriesMinX[index], n->entriesMinY[index], n->entriesMaxX[index], n->entriesMaxY[index]))))
                    {
                        index = i;
                        leastEnlargement = tempEnlargement;
                    }
            }
            m_parents.push(n->nodeId);
            m_parentsEntry.push(index);
            n = getNode(n->ids[index]);
        }
    }

    std::shared_ptr<Node> RTreeBuilder::adjustTree(std::shared_ptr<Node> n, std::shared_ptr<Node> nn) {
        while (n->level != m_treeHeight) {
            auto parent = getNode(m_parents.top());
            m_parents.pop();
            int entry = m_parentsEntry.top();
            m_parentsEntry.pop();

            if (parent->ids[entry] != n->nodeId) {
                std::cerr << "Error: entry " << entry << " in node " <<
                     parent->nodeId << " should point to node " <<
                     n->nodeId << ", actually points to node " << parent->ids[entry] << std::endl;
            }

            if (parent->entriesMinX[entry] != n->mbrMinX ||
               parent->entriesMinY[entry] != n->mbrMinY ||
               parent->entriesMaxX[entry] != n->mbrMaxX ||
               parent->entriesMaxY[entry] != n->mbrMaxY) {

                parent->entriesMinX[entry] = n->mbrMinX;
                parent->entriesMinY[entry] = n->mbrMinY;
                parent->entriesMaxX[entry] = n->mbrMaxX;
                parent->entriesMaxY[entry] = n->mbrMaxY;

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

            n = std::move(parent);
            nn = std::move(newNode);

        }
        return nn;
    }

    int RTreeBuilder::getNextNodeId() {
        int nextNodeId = 0;
        if (!m_deletedNodeIds.empty()) {
            nextNodeId = m_deletedNodeIds.top();
            m_deletedNodeIds.pop();
        } else {
            nextNodeId = 1 + m_highestUsedNodeId++;
        }
        return nextNodeId;
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

} // rtree