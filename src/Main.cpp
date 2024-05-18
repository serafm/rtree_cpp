#include <iostream>
#include "RTree.h"
#include "Point.h"

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

    return 0;
}
