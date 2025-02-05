#pragma once
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

    /**
     * @brief The default minimum number of entries per node in the R-tree.
     *
     * Nodes must have at least this many entries, except for the root node which is
     * allowed to have fewer to accommodate varying tree sizes.
     */
    static constexpr int DEFAULT_MIN_NODE_ENTRIES = 20;
    /**
     * @brief A mapping of node IDs to their corresponding node objects.
     *
     * This hash map allows quick lookups of nodes by their ID, facilitating fast access
     * to specific nodes during insertions, deletions, queries, and tree adjustments.
     */
    std::unordered_map<int, std::shared_ptr<Node>> m_nodeMap{};

    /**
     * @brief The ID of the root node of the R-tree.
     *
     * This value identifies the current root node. As the tree grows and splits occur,
     * new root nodes may be created with different node IDs, updating this value accordingly.
     */
    int m_rootNodeId = 1;

    /**
     * @brief Tracks the highest node ID used so far.
     *
     * Whenever a new node is created, this value is updated. If deleted node IDs
     * are reused, this value ensures that newly created nodes still receive unique,
     * incrementally increasing IDs.
     */
    int m_highestUsedNodeId = m_rootNodeId;

    /**
     * @brief The current height of the R-tree.
     *
     * The root node is at level m_treeHeight, and leaves are at level 1. As nodes split
     * and a new root is created, this value may increase.
     */
    int m_treeHeight = 1;

    /**
     * @brief A pointer to the root node of the R-tree.
     *
     * The root node is the entry point for all operations on the R-tree, including
     * insertions, searches, and splits.
     */
    std::shared_ptr<Node> m_root{};

    /**
     * @brief The total number of entries (rectangles) in the R-tree.
     *
     * This count increases as entries are inserted. It can be used to determine the
     * overall size of the dataset stored in the tree.
     */
    int m_size{};

    /**
     * @brief The maximum number of entries each node can hold.
     *
     * Once a node reaches this limit, it must be split into two nodes.
     */
    int m_maxNodeEntries{};

    /**
     * @brief The minimum number of entries each node must hold.
     *
     * Except for the root, all nodes must have at least this many entries, ensuring
     * balanced distribution and preventing excessive underflow in the tree.
     */
    int m_minNodeEntries{};

    /**
     * @brief A unique identifier assigned to each new entry as it is inserted.
     *
     * This ID increments automatically with each inserted entry, ensuring every
     * rectangle in the R-tree can be uniquely identified.
     */
    int m_entryId = 1;

    /**
     * @brief A unique id for every node created in the tree.
     *
     * This ID increments automatically with each new node, ensuring every
     * node in the R-tree can be uniquely identified.
     */
    int m_nextNodeId = 0;

    /**
     * @brief Retrieves a previously unused node ID, reusing deleted node IDs if available.
     *
     * This function manages the node ID space for the R-tree. If there are any
     * previously deleted nodes whose IDs can be recycled, it returns one of those.
     * Otherwise, it increments the highest used node ID and returns a new one.
     *
     * @return The new node ID.
     */
    int getNextNodeId();

    /**
     * @brief Creates the leaf level of the R-tree from a set of rectangles.
     *
     * This function sorts the rectangles based on their spatial coordinates and
     * groups them into leaf nodes, ensuring an optimal fill ratio.
     *
     * @param rectangles A vector of rectangles to be grouped into leaf nodes.
     * @param nodeCapacity The maximum number of entries a node can hold.
     * @return A vector of shared pointers to the created leaf nodes.
     */
    std::vector<std::shared_ptr<Node>> createLeafLevel(std::vector<Rectangle>& rectangles, int nodeCapacity);

    /**
     * @brief Creates the next level of the R-tree from a given set of nodes.
     *
     * This function groups child nodes into parent nodes, constructing the tree
     * level by level until only a single root remains.
     *
     * @param nodes A vector of shared pointers to nodes that need to be grouped.
     * @param nodeCapacity The maximum number of entries a node can hold.
     * @return A vector of shared pointers to the newly created parent nodes.
     */
    std::vector<std::shared_ptr<Node>> createNextLevel(std::vector<std::shared_ptr<Node>>& nodes, int nodeCapacity);

    /**
     * @brief Creates a new internal node with given child nodes.
     *
     * This function initializes a new node at the specified level and adds entries
     * corresponding to its child nodes.
     *
     * @param children A vector of shared pointers to child nodes.
     * @param level The level of the new node in the R-tree.
     * @return A shared pointer to the newly created node.
     */
    std::shared_ptr<Node> createNode(const std::vector<std::shared_ptr<Node>>& children, int level);

    /**
     * @brief Creates a leaf node containing a subset of rectangles.
     *
     * This function initializes a leaf node and inserts rectangle entries into it.
     *
     * @param rectangles A vector of rectangles to be added to the leaf node.
     * @param start The starting index of the rectangles in the vector.
     * @param end The ending index (exclusive) of the rectangles in the vector.
     * @return A shared pointer to the newly created leaf node.
     */
    std::shared_ptr<Node> createLeafNode(const std::vector<Rectangle>& rectangles, int start, int end);

    public:

    /**
    * @brief Bulk loads a set of rectangles (a given dataset) into the R-tree.
    *
    * This function constructs the R-tree from a given set of rectangles using a
    * bottom-up approach. It first creates the leaf level, then iteratively builds
    * higher levels until a single root node remains.
    *
    * @param rectangles A vector of rectangles to be inserted into the R-tree.
    */
    void bulkLoad(std::vector<Rectangle>& rectangles);

    /**
     * @brief Default constructor for RTreeBuilder.
     *
     * Initializes an RTreeBuilder instance with default parameters and sets up the
     * initial root node of the R-tree. The default maximum and minimum node entry
     * counts are used, and internal bookkeeping data structures (e.g., status arrays,
     * node maps) are prepared for later insertions.
     */
    RTreeBuilder();

    /**
     * @brief Retrieves a node by its ID.
     *
     * This function looks up a node in the internal node map by its unique ID
     * and returns a shared pointer to that node. If the node ID does not exist,
     * an exception is thrown.
     *
     * @param id The unique node ID.
     * @return A shared pointer to the corresponding node.
     * @throws std::out_of_range If the node ID is not found in the node map.
     */
    std::shared_ptr<Node> getNode(int id);

    /**
     * @brief Returns the total number of entries (rectangles) stored in the R-tree.
     *
     * @return The number of entries inserted into the R-tree.
     */
    int treeSize() const;

    /**
     * @brief Returns the total number of nodes in the R-tree.
     *
     * This function returns the number of nodes currently present in the R-tree,
     * including internal nodes and leaf nodes.
     *
     * @return The number of nodes.
     */
    int numNodes() const;

    /**
     * @brief The default maximum number of entries allowed per node in the R-tree.
     *
     * When a node tries to accommodate more than this limit, it must be split,
     * distributing entries between the original and a newly created node.
     */
    static constexpr int DEFAULT_MAX_NODE_ENTRIES = 50;

    int getRootNodeId() const;
};

} // spatialindex

#endif //RTREEBUILDER_H
