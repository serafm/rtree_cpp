#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#include <stack>
#include <fstream>
#include <map>

#include "../structures/Node.h"
#include "../structures/Rectangle.h"

#ifndef RTREEBULKLOAD_H
#define RTREEBULKLOAD_H

namespace rtree {

class RTreeBulkLoad {

    /**
     * @brief The ID of the root node of the R-tree.
     *
     * This value identifies the current root node. As the tree grows and splits occur,
     * new root nodes may be created with different node IDs, updating this value accordingly.
     */
    int m_rootNodeId{};

    /**
     * @brief The total number of entries (rectangles) in the R-tree.
     *
     * This count increases as entries are inserted. It can be used to determine the
     * overall size of the dataset stored in the tree.
     */
    int m_totalRectangles{};

    /**
     * @brief A unique id for every node created in the tree.
     *
     * This ID increments automatically with each new node, ensuring every
     * node in the R-tree can be uniquely identified.
     */
    int m_nextNodeId{};

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
    std::vector<Node*> createLeafLevel(std::vector<Rectangle>& rectangles, int nodeCapacity);

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
    std::vector<Node*> createNextLevel(std::vector<Node*>& nodes, int nodeCapacity);

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
    Node* createNode(std::vector<Node*>& children, int level);

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
    Node* createLeafNode(const std::vector<Rectangle>& rectangles, int start, int end);

    /**
     * @brief Recursively retrieves all leaf entry IDs from the given node and its children.
     *
     * If the node is a leaf node, its entry IDs are added to the vector.
     * Otherwise, the method recurses into all children.
     *
     * @param node The starting node.
     * @param leafs Vector to store the collected leaf entry IDs.
     */
    void getLeafs(Node* node, std::vector<int>& leafs);

    /**
     * @brief Sweeps through a small batch of leaf rectangles and finds those intersecting the query range.
     *
     * This method assumes a small window (batch) of leafs and performs a vertical sweep
     * to collect intersecting rectangles into the results vector.
     *
     * @param leaf A single rectangle to check for intersection.
     * @param rangeQ The query rectangle.
     * @param start The starting index in the batch.
     * @param size The size of the batch (1).
     * @param results Vector to collect matching entry IDs.
     * @param res_size Current size of the results vector (used for resizing).
     */
    void sweepLeafs(Rectangle& leaf, const Rectangle& rangeQ, int start, int size, std::vector<int>& results, uint32_t& res_size);

    /**
     * @brief Sweeps through a larger batch of leaf rectangles, identifying those intersecting the query range.
     *
     * This method handles multiple leaf rectangles and checks them against the query rectangle.
     * Results are collected into the provided results vector.
     *
     * @param rangeQ The query rectangle.
     * @param leafs The vector of leaf rectangles to check.
     * @param start The starting index within the leafs vector.
     * @param size The number of rectangles to check.
     * @param results Vector to collect matching entry IDs.
     * @param res_size Current size of the results vector (used for resizing).
     */
    void sweepLeafsNext(const Rectangle& rangeQ, std::vector<Rectangle>& leafs, int start, int size, std::vector<int>& results, uint32_t& res_size);

    /**
     * @brief Checks if two rectangles (range and entry) intersect.
     *
     * @param rangeMinX Minimum X coordinate of the range rectangle.
     * @param rangeMinY Minimum Y coordinate of the range rectangle.
     * @param rangeMaxX Maximum X coordinate of the range rectangle.
     * @param rangeMaxY Maximum Y coordinate of the range rectangle.
     * @param rectMinX Minimum X coordinate of the entry rectangle.
     * @param rectMinY Minimum Y coordinate of the entry rectangle.
     * @param rectMaxX Maximum X coordinate of the entry rectangle.
     * @param rectMaxY Maximum Y coordinate of the entry rectangle.
     * @return True if the rectangles intersect, false otherwise.
     */
    static inline bool intersects(float rangeMinX, float rangeMinY, float rangeMaxX, float rangeMaxY,
                                  float rectMinX, float rectMinY, float rectMaxX, float rectMaxY) {
        return !(rectMaxX < rangeMinX || rectMinX > rangeMaxX ||
                 rectMaxY < rangeMinY || rectMinY > rangeMaxY);
    }

    /**
     * @brief A pointer to the root node of the R-tree.
    */
    Node* m_root;

    /**
     * @return the total number of leafs stored in the R-tree.
     */
    int getLeafsSize() const;

    /**
     * @brief The current height of the R-tree.
     *
     * The root node is at level m_treeHeight, and leaves are at level 1. As nodes split
     * and a new root is created, this value may increase.
     */
    int treeHeight{};

    /**
     * @brief The default maximum number of entries allowed per node in the R-tree.
     *
     * When a node tries to accommodate more than this limit, it must be split,
     * distributing entries between the original and a newly created node.
     */
    const int m_capacity{};

public:

    /**
     * @brief Constructor for RTreeBulkLoad.
     *
     * Initializes an RTreeBulkLoad instance with the specified capacity, which defines
     * the maximum number of entries each node can hold.
     *
     * @param capacity Maximum number of entries per node.
     */
    explicit RTreeBulkLoad(int capacity);

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
     * @brief Performs a spatial join between two R-trees.
     *
     * This function recursively traverses both R-trees, identifying entries whose
     * leafs (rectangles) intersect. Only nodes whose
     * minimum bounding rectangles overlap are further explored, ensuring a more
     * efficient join operation.
     *
     * @param rtreeB The second R-tree instance to join.
     */
    void join(RTreeBulkLoad& rtreeB);

    /**
     * @brief Performs a range query on the R-tree.
     *
     * Searches for all leaf entries (rectangles) that are contained or intersect with the given range.
     * Results are collected into an internal vector (m_ids).
     *
     * @param range The query range.
     */
    void range(const Rectangle& range);

    /**
     * @brief Performs a k-nearest neighbors (kNN) search on the R-tree.
     *
     * Finds the `k` nearest leaf entries (rectangles) to the given query point.
     * This uses a priority queue for best-first search, prioritizing nodes/rectangles
     * based on their distance to the query point.
     *
     * @param p The query point.
     * @param k The number of nearest neighbors to find.
     */
    void nearestN(const Point& p, int k);
};

} // rtree

#endif //RTREEBULKLOAD_H
