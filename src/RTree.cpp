#include "RTree.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

#include "Collections/IntVector.h"
#include "Collections/PriorityQueue.h"
#include "Rectangle.h"

namespace rtree {

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
        std::cout << "Adding rectangle with ID: " << id << std::endl;
        add(r.minX, r.minY, r.maxX, r.maxY, id, 1);
        m_size++;
    }

    void RTree::add(float minX, float minY, float maxX, float maxY, int id, int level) {
        m_parents = std::stack<int>();
        m_parentsEntry = std::stack<int>();
        const auto n = chooseNode(minX, minY, maxX, maxY, level);
        std::shared_ptr<Node> newLeaf;

        // Check if node has space for new entry
        if (n->entryCount < m_maxNodeEntries)
        {
            n->addEntry(minX, minY, maxX, maxY, id);
        } else {
            newLeaf = splitNode(n, minX, minY, maxX, maxY, id);
        }

        const auto newNode = adjustTree(n, newLeaf);

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

    std::shared_ptr<Node> RTree::getNode(const int id) {
        return m_nodeMap[id];
    }

    int RTree::getRootNodeId() const {
        return m_rootNodeId;
    }

    std::shared_ptr<Node> RTree::splitNode(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId)
    {
        // entryStatus.assign(initialEntryStatus.begin(), initialEntryStatus.end());
        auto newNode =  std::make_shared<Node>(getNextNodeId(), n->level);
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
                        n->entries[i] = {0, 0, 0, 0};
                        n->entryCount = n->entryCount - 1;
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
                std::cerr << "Error: Normalized separation is not between -1 and 1" << std::endl;
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

        for (int i = 0; i < m_maxNodeEntries; i++) {
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
            n->ids[next] = 0;
            n->entries[next] = {0, 0, 0, 0};
        }

        return next;
    }

    void RTree::condenseTree(const std::shared_ptr<Node> &l) {
        // CT1 [Initialize] Set n=l. Set the list of eliminated
        // nodes to be empty.
        auto n = l;
        int parentEntry = 0;

        std::stack<int> eliminatedNodeIds;

        // CT2 [Find parent entry] If N is the root, go to CT6. Otherwise,
        // let P be the parent of N, and let En be N's entry in P
        while (n->level != m_treeHeight) {
            const std::shared_ptr<Node> parent = getNode(m_parents.top());
            m_parents.pop();
            parentEntry = m_parentsEntry.top();
            m_parentsEntry.pop();

            // CT3 [Eliminate under-full node] If N has too few entries,
            // delete En from P and add N to the list of eliminated nodes
            if (n->entryCount < m_minNodeEntries) {
                parent->deleteEntry(parentEntry);
                eliminatedNodeIds.push(n->nodeId);
            } else {
                // CT4 [Adjust covering rectangle] If N has not been eliminated,
                // adjust EnI to tightly contain all entries in N
                if (n->mbrMinX != parent->entries[parentEntry].minX ||
                    n->mbrMinY != parent->entries[parentEntry].minY ||
                    n->mbrMaxX != parent->entries[parentEntry].maxX ||
                    n->mbrMaxY != parent->entries[parentEntry].maxY) {

                    // Use Rectangle class for calculations for clarity and potential optimizations
                    parent->entries[parentEntry].minX = n->mbrMinX;
                    parent->entries[parentEntry].minY = n->mbrMinY;
                    parent->entries[parentEntry].maxX = n->mbrMaxX;
                    parent->entries[parentEntry].maxY = n->mbrMaxY;
                    parent->recalculateMBRIfInfluencedBy(parent->entries[parentEntry].minX, parent->entries[parentEntry].minY, parent->entries[parentEntry].maxX, parent->entries[parentEntry].maxY);
                }
            }
            // CT5 [Move up one level in tree] Set N=P and repeat from CT2
            n = parent;
        }

        // CT6 [Reinsert orphaned entries] Reinsert all entries of nodes in set Q.
        // Entries from eliminated leaf nodes are reinserted in tree leaves as in
        // Insert(), but entries from higher level nodes must be placed higher in
        // the tree, so that leaves of their dependent subtrees will be on the same
        // level as leaves of the main tree
        while (!eliminatedNodeIds.empty()) {
            const auto e = getNode(eliminatedNodeIds.top());
            eliminatedNodeIds.pop();
            for (int j = 0; j < e->entryCount; j++) {
                add(e->entries[j].minX, e->entries[j].minY, e->entries[j].maxX, e->entries[j].maxY, e->ids[j], e->level);
                e->ids[j] = 0;
            }
            e->entryCount = 0;
            m_deletedNodeIds.push(e->nodeId);
        }
    }

    std::shared_ptr<Node> RTree::chooseNode(float minX, float minY, float maxX, float maxY, int level) {
        auto n = getNode(m_rootNodeId);
        m_parents = std::stack<int>();
        m_parentsEntry = std::stack<int>();

        while (true) {
            if (n == nullptr) {
                std::cerr << "Could not get root node " << m_rootNodeId << std::endl;
            }

            if (n->level == level) {
                return n;
            }

            float leastEnlargement = Rectangle::enlargement(
                n->entries[0].minX,
                n->entries[0].minY,
                n->entries[0].maxX,
                n->entries[0].maxY,
                minX,
                minY,
                maxX,
                maxY);

            int index = 0; // index of rectangle in subtree
            for (int i = 1; i < n->entryCount; i++) {
                float tempMinX = n->entries[i].minX;
                float tempMinY = n->entries[i].minY;
                float tempMaxX = n->entries[i].maxX;
                float tempMaxY = n->entries[i].maxY;
                float tempEnlargement = Rectangle::enlargement(tempMinX, tempMinY, tempMaxX, tempMaxY,
                                                              minX, minY, maxX, maxY);
                if ((tempEnlargement < leastEnlargement) ||
                    ((tempEnlargement == leastEnlargement) &&
                     (Rectangle::area(tempMinX, tempMinY, tempMaxX, tempMaxY) <
                      Rectangle::area(
                          n->entries[index].minX,
                          n->entries[index].minY,
                          n->entries[index].maxX,
                          n->entries[index].maxY))))
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

            // AT5 [Move up to next level] Set N = P and set NN = PP if a split
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

    Rectangle RTree::getBounds() {
        Rectangle bounds;

        auto n = getNode(getRootNodeId());
        if (n != nullptr && n->entryCount > 0) {
            bounds = Rectangle();
            bounds.minX = n->mbrMinX;
            bounds.minY = n->mbrMinY;
            bounds.maxX = n->mbrMaxX;
            bounds.maxY = n->mbrMaxY;
        }
        return bounds;
    }

    int RTree::numNodes() const {
        return m_nodeMap.size();
    }

    /*
     *
    *float RTree::nearest(Point& p, Node& n, float furthestDistanceSq, Collections::IntVector& nearestIds) {
        for (int i = 0; i < n.entryCount; i++) {
            float tempDistanceSq = Rectangle::distanceSq(n.entries[i].minX, n.entries[i].minY, n.entries[i].maxX, n.entries[i].maxY, p.x, p.y);
            if (n.isLeaf()) { // for leaves, the distance is an actual nearest distance
                if (tempDistanceSq < furthestDistanceSq) {
                    furthestDistanceSq = tempDistanceSq;
                    nearestIds.reset();
                }
                if (tempDistanceSq <= furthestDistanceSq) {
                    nearestIds.add(n.ids[i]);
                }
            } else { // for index nodes, only go into them if they potentially could have
                // a rectangle nearer than actualNearest
                if (tempDistanceSq <= furthestDistanceSq) {
                    // search the child node
                    furthestDistanceSq = nearest(p, getNode(n.ids[i]), furthestDistanceSq, nearestIds);
                }
            }
        }
        return furthestDistanceSq;
    }

    bool RTree::intersects(Rectangle& r, Node& n) {
        for (int i = 0; i < n.entryCount; i++) {
            if (Rectangle::intersects(
                r.minX,
                r.minY,
                r.maxX,
                r.maxY,
                n.entries[i].minX,
                n.entries[i].minY,
                n.entries[i].maxX,
                n.entries[i].maxY)) {
                if (n.isLeaf()) {
                    // TODO: Lambda function
                    //if (!v.execute(n.ids[i])) {
                      //  return false;
                    //}
                } else {
                    Node& childNode = getNode(n.ids[i]);
                    //if (!intersects(r, v, childNode)) {
                      //  return false;
                    //}
                    }
            }
        }
        return true;
    }

    Rectangle& RTree::calculateMBR(Node& n) {
        auto mbr = Rectangle();

        for (int i = 0; i < n.entryCount; i++) {
            if (n.entries[i].minX < mbr.minX) mbr.minX = n.entries[i].minX;
            if (n.entries[i].minY < mbr.minY) mbr.minY = n.entries[i].minY;
            if (n.entries[i].maxX > mbr.maxX) mbr.maxX = n.entries[i].maxX;
            if (n.entries[i].maxY > mbr.maxY) mbr.maxY = n.entries[i].maxY;
        }

        return mbr;
    }

     bool RTree::del(Rectangle& r, int id) {
        m_parents = std::stack<int>();
        m_parents.push(m_rootNodeId);

        m_parentsEntry = std::stack<int>();
        m_parentsEntry.push(-1);
        Node n;
        int foundIndex = -1;

        while (foundIndex == -1 && !m_parents.empty()) {
            n = getNode(m_parents.top());
            int startIndex = m_parentsEntry.top() + 1;

            if (!n.isLeaf()) {
                cout << "Searching node " << n.nodeId << ", from index " << startIndex << endl;
                bool contains = false;
                for (int i = startIndex; i < n.entryCount; i++) {
                    if (Rectangle::contains(n.entries[i].minX, n.entries[i].minY, n.entries[i].maxX, n.entries[i].maxY,
                     r.minX, r.minY, r.maxX, r.maxY)) {
                        m_parents.push(n.ids[i]);
                        m_parentsEntry.pop();
                        m_parentsEntry.push(i); // this becomes the start index when the child has been searched
                        m_parentsEntry.push(-1);
                        contains = true;
                        break; // ie go to next iteration of while()
                     }
                }
                if (contains) {
                    continue;
                }
            } else {
                foundIndex = n.findEntry(r.minX, r.minY, r.maxX, r.maxY, id);
            }

            m_parents.pop();
            m_parentsEntry.pop();
        }

        if (foundIndex != -1 && !n.isEmpty() ) {
            n.deleteEntry(foundIndex);
            condenseTree(n);
            size--;
        }

        Node& root = getNode(m_rootNodeId);
        while (root.entryCount == 1 && m_treeHeight > 1) {
            m_deletedNodeIds.push(m_rootNodeId);
            root.entryCount = 0;
            m_rootNodeId = root.ids[0];
            m_treeHeight--;
            root = getNode(m_rootNodeId);
        }

        // if the tree is now empty, then set the MBR of the root node back to its original state
        // (this is only needed when the tree is empty, as this is the only state where an empty node
        // is not eliminated)
        if (size == 0) {
            root.mbrMinX = std::numeric_limits<float>::max();
            root.mbrMinY = std::numeric_limits<float>::max();
            root.mbrMaxX = -std::numeric_limits<float>::max();
            root.mbrMaxY = -std::numeric_limits<float>::max();
        }
        return (foundIndex != -1);
    }

    void RTree::nearest(Point& p, float furthestDistance) {
        auto rootNode = getNode(m_rootNodeId);

        float furthestDistanceSq = furthestDistance * furthestDistance;
        auto nearestIds = Collections::IntVector();

        std::cout << "Nearest rectangles to point (" << p.x << "," << p.y << ") are: " << std::endl;

        auto printNearest = [&]() {
            for (int i = 0; i < nearestIds.size(); i++) {
                std::cout << "Rectangle with ID: " << nearestIds.get(i) << std::endl;
            }
        };

        nearest(p, rootNode, furthestDistanceSq, nearestIds);

        printNearest();

        nearestIds.reset();
    }

    void RTree::createNearestNDistanceQueue(Point& p, int count, Collections::PriorityQueue& distanceQueue, float furthestDistance) {
        //  return immediately if given an invalid "count" parameter
        if (count <= 0) {
            return;
        }

        m_parents = std::stack<int>();
        m_parents.push(m_rootNodeId);

        m_parentsEntry = std::stack<int>();
        m_parentsEntry.push(-1);

        auto savedValues = Collections::IntVector();
        float savedPriority = 0;

        float furthestDistanceSq = furthestDistance * furthestDistance;

        while (!m_parents.empty()) {
            auto n = getNode(m_parents.top());
            int startIndex = m_parentsEntry.top() + 1;

            if (!n.isLeaf()) {
                // go through every entry in the index node to check
                // if it could contain an entry closer than the farthest entry
                // currently stored.
                bool near = false;
                for (int i = startIndex; i < n.entryCount; i++) {
                    if (Rectangle::distanceSq(n.entries[i].minX, n.entries[i].minY,
                                           n.entries[i].maxX, n.entries[i].maxY,
                                           p.x, p.y) <= furthestDistanceSq) {
                        m_parents.push(n.ids[i]);
                        m_parentsEntry.pop();
                        m_parentsEntry.push(i); // this becomes the start index when the child has been searched
                        m_parentsEntry.push(-1);
                        near = true;
                        break; // ie go to next iteration of while()
                                           }
                }
                if (near) {
                    continue;
                }
            } else {
                // go through every entry in the leaf to check if
                // it is currently one of the nearest N entries.
                for (int i = 0; i < n.entryCount; i++) {
                    float entryDistanceSq = Rectangle::distanceSq(n.entries[i].minX, n.entries[i].minY, n.entries[i].maxX, n.entries[i].maxY, p.x, p.y);
                    int entryId = n.ids[i];

                    if (entryDistanceSq <= furthestDistanceSq) {
                        distanceQueue.insert(entryId, entryDistanceSq);

                        while (distanceQueue.size() > count) {
                            // normal case - we can simply remove the lowest priority (highest distance) entry
                            int value = distanceQueue.getValue();
                            float distanceSq = distanceQueue.getPriority();
                            distanceQueue.pop();

                            // rare case - multiple items of the same priority (distance)
                            if (distanceSq == distanceQueue.getPriority()) {
                                savedValues.add(value);
                                savedPriority = distanceSq;
                            } else {
                                savedValues.reset();
                            }
                        }

                        // if the saved values have the same distance as the
                        // next one in the tree, add them back in.
                        if (savedValues.size() > 0 && savedPriority == distanceQueue.getPriority()) {
                            for (int svi = 0; svi < savedValues.size(); svi++) {
                                distanceQueue.insert(savedValues.get(svi), savedPriority);
                            }
                            savedValues.reset();
                        }

                        // narrow the search, if we have already found N items
                        if (distanceQueue.getPriority() < furthestDistanceSq && distanceQueue.size() >= count) {
                            furthestDistanceSq = distanceQueue.getPriority();
                        }
                    }
                }
            }
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
        Collections::PriorityQueue distanceQueue = Collections::PriorityQueue(false);
        createNearestNDistanceQueue(p, count, distanceQueue, furthestDistance);

        auto response = [&]() {
            // Here we simply print to console instead of logging
            std::cout << "Rectangle with distance= " << distanceQueue.getValue() << std::endl;
            distanceQueue.pop();
        };

        while (distanceQueue.size() > 0) {
            response();
        }
    }

    void RTree::nearestN(Point& p, int count, float furthestDistance) {
        auto distanceQueue = Collections::PriorityQueue(false);
        createNearestNDistanceQueue(p, count, distanceQueue, furthestDistance);
        distanceQueue.setSortOrder(true);

        auto response = [&]() {
            // Here we simply print to console instead of logging
            std::cout << "Rectangle with distance= " << distanceQueue.getValue() << std::endl;
            distanceQueue.pop();
        };

        while (distanceQueue.size() > 0) {
            response();
        }
    }

    void RTree::intersects(Rectangle& r) {
        auto rootNode = getNode(m_rootNodeId);
        intersects(r, rootNode);
    }

    void RTree::contains(Rectangle& r) {
        // find all rectangles in the tree that are contained by the passed rectangle
        // written to be non-recursive (should model other searches on this?)
        m_parents = std::stack<int>();
        m_parents.push(m_rootNodeId);

        m_parentsEntry = std::stack<int>();
        m_parentsEntry.push(-1);

        auto response = [&]() {
            for (int id : m_ids) {
                std::cout << "Rectangle with ID: " << id << " was contained" << std::endl;
            }
        };

        while (!m_parents.empty()) {
            auto n = getNode(m_parents.top());
            int startIndex = m_parentsEntry.top() + 1;

            if (!n.isLeaf()) {
                // go through every entry in the index node to check
                // if it intersects the passed rectangle. If so, it
                // could contain entries that are contained.
                bool intersects = false;
                for (int i = startIndex; i < n.entryCount; i++) {
                    if (Rectangle::intersects(r.minX, r.minY, r.maxX, r.maxY,
                                             n.entries[i].minX, n.entries[i].minY, n.entries[i].maxX, n.entries[i].maxY)) {
                        m_parents.push(n.ids[i]);
                        m_parentsEntry.pop();
                        m_parentsEntry.push(i); // this becomes the start index when the child has been searched
                        m_parentsEntry.push(-1);
                        intersects = true;
                        break; // ie go to next iteration of while()
                                             }
                }
                if (intersects) {
                    continue;
                }
            } else {
                // go through every entry in the leaf to check if
                // it is contained by the passed rectangle
                for (int i = 0; i < n.entryCount; i++) {
                    if (Rectangle::contains(r.minX, r.minY, r.maxX, r.maxY,
                                           n.entries[i].minX, n.entries[i].minY, n.entries[i].maxX, n.entries[i].maxY)) {
                        m_ids.push_back(n.ids[i]);
                    }
                }
            }

            m_parents.pop();
            m_parentsEntry.pop();
        }
        response();
    }
*/
}
