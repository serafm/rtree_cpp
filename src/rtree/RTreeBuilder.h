#pragma once
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

#include "../rtree/core/Node.h"
#include "../rtree/core/Rectangle.h"

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
     * @brief Status value indicating that an entry has been assigned to a node.
     *
     * During node splits, entries are either marked as assigned to one of the two new nodes
     * or remain unassigned until a decision is made.
     */
    static constexpr int ENTRY_STATUS_ASSIGNED = 0;

    /**
     * @brief Status value indicating that an entry has not yet been assigned to a node.
     *
     * Used during node splitting operations to track entries that need to be distributed
     * between the original and the newly created node.
     */
    static constexpr int ENTRY_STATUS_UNASSIGNED = 1;

    /**
     * @brief A mapping of node IDs to their corresponding node objects.
     *
     * This hash map allows quick lookups of nodes by their ID, facilitating fast access
     * to specific nodes during insertions, deletions, queries, and tree adjustments.
     */
    std::unordered_map<int, std::shared_ptr<Node>> m_nodeMap{};

    /**
     * @brief A vector tracking the assignment status of each entry during node splits.
     *
     * Each element corresponds to an entry and stores whether it's assigned or unassigned,
     * guiding the process of distributing entries between two nodes during a split.
     */
    std::vector<int8_t> m_entryStatus{};

    /**
     * @brief A vector holding the initial status of entries when a node is split.
     *
     * This vector stores the default or initial status of entries before any assignment
     * decisions have been made during the splitting process.
     */
    std::vector<int8_t> m_initialEntryStatus{};

    /**
     * @brief A stack of parent node IDs encountered while traversing down the tree.
     *
     * When searching for the insertion node, the path is recorded so that, after
     * insertion at the leaf level, the tree can be adjusted from the bottom up.
     */
    std::stack<int> m_parents{};

    /**
     * @brief A stack of parent entry indexes encountered while traversing down the tree.
     *
     * For each parent node in the tree traversal path, this stack stores the index of the
     * entry that points to the child node we descended into. This enables adjustments
     * to be made correctly when propagating changes up the tree.
     */
    std::stack<int> m_parentsEntry{};

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
     * @brief A stack of node IDs that have been deleted and can be reused.
     *
     * Instead of continually incrementing node IDs, deleted node IDs are stored here
     * for recycling. This reduces the overall range of node IDs used.
     */
    std::stack<int> m_deletedNodeIds{};

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
     * @brief Inserts a rectangle into the R-tree at a specified level.
     *
     * This function performs the insertion logic, locating the appropriate node
     * for the rectangle and, if necessary, splitting nodes that exceed the maximum
     * entry limit. After insertion, the tree is adjusted to maintain the R-tree
     * properties. If the insertion at the root level causes a split, a new root
     * is created to accommodate the increased height.
     *
     * @param minX The minimum X-coordinate of the rectangle.
     * @param minY The minimum Y-coordinate of the rectangle.
     * @param maxX The maximum X-coordinate of the rectangle.
     * @param maxY The maximum Y-coordinate of the rectangle.
     * @param id The entry ID associated with this rectangle.
     * @param level The tree level at which to insert the rectangle (1 for leaves).
     */
    void add(float minX, float minY, float maxX, float maxY, int id, int level);

    /**
     * @brief Adjusts the R-tree after an insertion or split to maintain its properties.
     *
     * After inserting an entry and possibly splitting a leaf node, this function
     * propagates changes up the tree. It may reposition entries in parent nodes
     * to accurately reflect new bounding rectangles or handle higher-level splits.
     * If a new node is created at the root level, a new root node is formed.
     *
     * @param n The node that was modified (leaf or internal).
     * @param nn An optional newly created node from a split operation.
     * @return A pointer to the new node if the root was split; otherwise, nullptr.
     */
    std::shared_ptr<Node> adjustTree(std::shared_ptr<Node> n, std::shared_ptr<Node> nn);

    /**
     * @brief Chooses the appropriate node level at which to insert a new entry.
     *
     * This function navigates down the R-tree to find the correct node into which
     * the given rectangle should be inserted. At each level, it selects the subtree
     * that will require the least enlargement to include the new rectangle. The
     * process stops when it reaches the specified target level.
     *
     * @param minX The minimum X-coordinate of the rectangle.
     * @param minY The minimum Y-coordinate of the rectangle.
     * @param maxX The maximum X-coordinate of the rectangle.
     * @param maxY The maximum Y-coordinate of the rectangle.
     * @param level The level at which to insert the new entry.
     * @return A pointer to the node at which the rectangle should be inserted.
     */
    std::shared_ptr<Node> chooseNode(float minX, float minY, float maxX, float maxY, int level);

    /**
     * @brief Chooses initial seed entries for a node split.
     *
     * When splitting a node, this method identifies "seed" entries to serve as the
     * first elements in the original and the new node. The chosen seeds aim to maximize
     * the normalized separation along the X or Y axes, thereby improving the distribution
     * of entries and reducing overlap.
     *
     * @param n The node being split.
     * @param newRectMinX The minimum X-coordinate of the new rectangle.
     * @param newRectMinY The minimum Y-coordinate of the new rectangle.
     * @param newRectMaxX The maximum X-coordinate of the new rectangle.
     * @param newRectMaxY The maximum Y-coordinate of the new rectangle.
     * @param newId The entry ID of the new rectangle.
     * @param newNode The newly created node for the split.
     */
    void pickSeeds(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId, const std::shared_ptr<Node>& newNode);

    /**
     * @brief Selects the next entry to assign during a node split.
     *
     * After choosing the seed entries, this function determines the best next entry
     * to place in either the original or the new node. It measures how much each
     * entry would enlarge the bounding rectangles of both nodes, choosing the entry
     * that creates the largest difference. This strategy ensures a balanced and
     * efficient split.
     *
     * @param n The original node being split.
     * @param newNode The newly created node receiving part of the entries.
     * @return The index of the chosen entry to distribute.
     */
    int pickNext(const std::shared_ptr<Node>& n, const std::shared_ptr<Node>& newNode);

    /**
     * @brief Splits a node that has exceeded its maximum capacity.
     *
     * Given a node that cannot accommodate a new entry, this function partitions
     * its entries between the original node and a newly created node. The goal of
     * splitting is to minimize overlap and coverage, thus maintaining good query
     * performance. It uses the "PickSeeds" and "PickNext" heuristics to determine
     * how entries are distributed.
     *
     * @param n The node that needs splitting.
     * @param newRectMinX The minimum X-coordinate of the new rectangle.
     * @param newRectMinY The minimum Y-coordinate of the new rectangle.
     * @param newRectMaxX The maximum X-coordinate of the new rectangle.
     * @param newRectMaxY The maximum Y-coordinate of the new rectangle.
     * @param newId The entry ID of the new rectangle.
     * @return A pointer to the newly created node resulting from the split.
     */
    std::shared_ptr<Node> splitNode(const std::shared_ptr<Node>& n, float newRectMinX, float newRectMinY, float newRectMaxX, float newRectMaxY, int newId);

    public:

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
     * @brief Adds a new entry (rectangle) to the R-tree.
     *
     * This function inserts a rectangle into the R-tree, adjusting internal nodes
     * and performing splits as needed. The rectangle is represented by its minimum
     * and maximum coordinates and is assigned a unique entry ID.
     *
     * @param r The rectangle to be inserted.
     */
    void addEntry(const Rectangle & r);

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

} // rtree

#endif //RTREEBUILDER_H