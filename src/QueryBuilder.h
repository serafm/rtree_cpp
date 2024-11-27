#pragma once

#ifndef RTREE_H
#define RTREE_H

#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <vector>

#include "rtree/Node.h"
#include "rtree/Point.h"
#include "rtree/RTreeBuilder.h"
#include "rtree/Rectangle.h"

namespace rtree {

    class QueryBuilder {

        // RTreeBuilder instances
        RTreeBuilder m_rtreeA;
        RTreeBuilder m_rtreeB;

        std::vector<int> m_ids;
        std::priority_queue<std::pair<float, int>> m_distanceQueue;
        std::stack<int> m_parents;
        std::stack<int> m_parentsEntry;

        // Builds a priority queue containing the nearest N rectangles to a given point p
        void nearestN(const Point & p, int count);

        // Find all rectangles in the tree that are contained by the passed rectangle
        void contains(Rectangle& r);

        void intersects(Rectangle& r);

        // Finds all rectangles in the subtree rooted at node n that intersect with a given rectangle r
        std::set<int> intersects(Rectangle &r, std::shared_ptr<Node>& n);

        // Prints the Nearest Neighbors.
        void printNearestNeighbors(Point& p, std::priority_queue<std::pair<float, int>>& queue);

        // Prints the rectangle ids contained by a range query
        void printRangeQuery(Rectangle& range, const std::vector<int>& ids);

        // Prints the rectangle ids intersecting with a given query
        void printIntersectedRectangles(const std::set<int>& ids);

        public:

        QueryBuilder(RTreeBuilder& rtreeA, RTreeBuilder& rtreeB);
        explicit QueryBuilder(RTreeBuilder& rtreeA);

        // Retrieve the rectangles contained in the given range
        void GetRange(Rectangle& range);

        // Retrieve the N nearest rectangles to a point with sorted results
        void GetNearestNeighbors(Point& p, int count);

    };
}
#endif // RTREE_H
