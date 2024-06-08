#include <iostream>
#include "RTree.h"
#include "Point.h"

using namespace SpatialIndex;

int main() {
    RTree rtree;

    // Insert some entries
    rtree.insert(Entry(Rectangle(0, 0, 1, 1), 1));
    rtree.insert(Entry(Rectangle(1, 1, 2, 2), 2));
    rtree.insert(Entry(Rectangle(2, 2, 3, 3), 3));
    rtree.insert(Entry(Rectangle(3, 3, 4, 4), 4));

    // Search for entries intersecting with a given rectangle
    Rectangle search_rect(1, 1, 3, 3);
    std::vector<int> results = rtree.search(search_rect);

    // Print search results
    std::cout << "Entries intersecting with rectangle (" << search_rect.xmin << ", " << search_rect.ymin
              << ", " << search_rect.xmax << ", " << search_rect.ymax << "):" << std::endl;
    for (int data : results) {
        std::cout << "Data: " << data << std::endl;
    }

    // Find the nearest neighbor to a point
    Point p(1.5, 1.5);
    int nearest = rtree.nearestNeighbor(p);
    std::cout << "Nearest neighbor to point (" << p.x << ", " << p.y << "): " << nearest << std::endl;

    // Find the k nearest neighbors to a point
    int k = 2;
    std::vector<int> k_nearest = rtree.nearestNeighbors(p, k);
    std::cout << "The " << k << " nearest neighbors to point (" << p.x << ", " << p.y << "):" << std::endl;
    for (int data : k_nearest) {
        std::cout << "Data: " << data << std::endl;
    }

    // Perform a range query
    Rectangle range(0.5, 0.5, 2.5, 2.5);
    std::vector<int> range_results = rtree.rangeQuery(range);
    std::cout << "Entries within range (" << range.xmin << ", " << range.ymin
              << ", " << range.xmax << ", " << range.ymax << "):" << std::endl;
    for (int data : range_results) {
        std::cout << "Data: " << data << std::endl;
    }

    // Perform a spatial join with another RTree
    RTree other_rtree;
    other_rtree.insert(Entry(Rectangle(1, 1, 2, 2), 5));
    other_rtree.insert(Entry(Rectangle(2, 2, 3, 3), 6));
    other_rtree.insert(Entry(Rectangle(3, 3, 4, 4), 7));

    std::vector<std::pair<int, int>> join_results = rtree.spatialJoin(other_rtree);
    std::cout << "Spatial join results:" << std::endl;
    for (const auto& pair : join_results) {
        std::cout << "Data1: " << pair.first << ", Data2: " << pair.second << std::endl;
    }

    return 0;
}
