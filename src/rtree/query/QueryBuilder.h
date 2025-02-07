#pragma once

#ifndef RTREE_H
#define RTREE_H

#include <map>
#include <queue>
#include <stack>
#include <vector>

#include "../core/Point.h"
#include "../RTreeBulkLoadBuilder.h"
#include "../core/Rectangle.h"

namespace rtree {

    class QueryBuilder {

        using RTreePtr = RTreeBulkLoadBuilder;
        RTreePtr m_rtreeA;
        RTreePtr m_rtreeB;
        std::vector<int> m_ids;
        std::map<int, std::vector<int>> m_joinRectangles;
        std::priority_queue<std::pair<float, int>> m_distanceQueue;
        std::stack<int> m_parents;
        std::stack<int> m_parentsEntry;

        /**
         * @brief Finds the nearest N entries in the R-tree to a given point using a best-first search strategy.
         *
         * This method leverages a priority queue (min-heap) of candidate nodes and entries keyed by their
         * minimum bounding rectangle distance to the query point. Unlike a simple depth-first approach,
         * we always expand the most promising (closest) node first. As soon as we find at least N neighbors,
         * we establish a maximum distance threshold, allowing us to prune entire subtrees that cannot yield
         * closer results. This prioritization and pruning significantly improves the efficiency of retrieving
         * the top N nearest neighbors to the specified point.
         *
         * @param p The query point for which nearest neighbors are sought.
         * @param count The number of nearest neighbors to retrieve.
         */
        void nearestN(const Point& p, int count);

        /**
         * @brief Searches the R-tree for entries contained within or intersecting a given rectangle.
         *
         * By leveraging the hierarchical bounding rectangles stored within the R-tree,
         * this function prunes branches that cannot possibly contain results. It only
         * descends into nodes whose bounding rectangles intersect with the query range,
         * ensuring that the search is focused and efficient. All leaf entries that lie
         * fully or partially within the specified rectangle are returned.
         *
         * @param range The query rectangle defining the spatial search region.
         */
        void contains(Rectangle& range);

        /**
         * @brief Performs a spatial join between two R-trees.
         *
         * This function recursively traverses both R-trees, identifying entries whose
         * bounding rectangles intersect. By leveraging the hierarchical structure of
         * the R-trees, it prunes large portions of the search space. Only nodes whose
         * minimum bounding rectangles overlap are further explored, ensuring a more
         * efficient join operation than a brute-force approach.
         *
         */
        void join();

        /**
         * @brief Outputs the nearest neighbor search results for a given point.
         *
         * This function extracts all the nearest neighbors from the provided priority queue,
         * sorts them by their distance from the query point, and prints them in ascending order.
         * If only one neighbor is requested, it prints that neighbor directly without sorting.
         *
         * @param p The query point for which neighbors were found.
         * @param queue A priority queue holding pairs of (distance, entry ID) representing nearest neighbors.
         */
        void printNearestNeighbors(Point& p, std::priority_queue<std::pair<float, int>>& queue);

        /**
         * @brief Prints the results of a range query.
         *
         * Given a query rectangle and a list of IDs whose bounding rectangles are contained
         * within or intersect the query rectangle, this function displays each qualifying ID.
         *
         * @param range The query rectangle used to filter the entries.
         * @param ids A list of entry IDs that satisfy the range query conditions.
         */
        void printRangeQuery(Rectangle& range, const std::vector<int>& ids);

        /**
         * @brief Prints the results of a spatial join query.
         *
         * After performing a join between two R-trees, this function lists each entry from
         * the first R-tree alongside the IDs of entries in the second R-tree that intersect
         * with it.
         */
        void printJoinQuery();

        public:

        QueryBuilder(RTreePtr& rtreeA, RTreePtr& rtreeB);
        QueryBuilder(RTreePtr& rtreeA);
        void Range(Rectangle& range);
        void NearestNeighbors(Point& p, int count);
        void Join();

    };
}
#endif // RTREE_H
