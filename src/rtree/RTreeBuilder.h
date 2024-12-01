#pragma once
#include <map>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

#include "Node.h"
#include "Rectangle.h"

#ifndef RTREEBUILDER_H
#define RTREEBUILDER_H

namespace rtree {

class RTreeBuilder: public std::enable_shared_from_this<RTreeBuilder>{

    static const int DEFAULT_MIN_NODE_ENTRIES = 20;
    static const int ENTRY_STATUS_ASSIGNED = 0;
    static const int ENTRY_STATUS_UNASSIGNED = 1;

    // List of all nodes in the tree
    std::unordered_map<int, std::shared_ptr<Node>> m_nodeMap{};
    std::vector<int8_t> m_entryStatus{};
    std::vector<int8_t> m_initialEntryStatus{};
    std::stack<int> m_parents{};
    std::stack<int> m_parentsEntry{};
    int m_treeHeight = 1;
    std::shared_ptr<Node> m_root{};
    int m_size = 0;
    std::stack<int> m_deletedNodeIds{};
    int m_maxNodeEntries{};
    int m_minNodeEntries{};
    int m_entryId = 1;

    // Adds a new rectangle to the R-tree
    void add(float minX, float minY, float maxX, float maxY, int id, int level);

    // Adjusts the tree after an insertion or a split
    std::shared_ptr<Node> adjustTree(std::shared_ptr<Node> n, std::shared_ptr<Node> nn);

    // Selects the most appropriate leaf node for inserting a new rectangle
    std::shared_ptr<Node> chooseNode(float minX, float minY, float maxX, float maxY, int level);

    // Generates and returns an id for a new node in the tree
    int getNextNodeId();

    // Selects two initial entries (seeds) to start the node splitting process when a node overflows
    void pickSeeds(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId, const std::shared_ptr<Node>& newNode);

    // During node splitting, this function selects the next entry to assign to one of the two groups (original node n or new node newNode).
    // It chooses the entry that causes the greatest difference in area enlargement between the two groups
    int pickNext(const std::shared_ptr<Node>& n, const std::shared_ptr<Node>& newNode);

    // Splits a full node n into two nodes after inserting a new rectangle defined by the provided coordinates and newId.
    // It redistributes the entries between the original node and a new node, and returns the new node created
    std::shared_ptr<Node> splitNode(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId);

    public:

    RTreeBuilder();

    // Retrieves a reference to the node with the specified id
    std::shared_ptr<Node> getNode(int id);

    void addEntry(const Rectangle & r);

    static const int DEFAULT_MAX_NODE_ENTRIES = 50;
    int m_rootNodeId = 1;
    int m_highestUsedNodeId = m_rootNodeId;

    // Return R-tree size
    int treeSize() const;

    // Return R-tree number of nodes
    int numNodes() const;
};

} // spatialindex

#endif //RTREEBUILDER_H
