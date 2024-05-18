#ifndef RTREE_H
#define RTREE_H

#include "Node.h"
#include <vector>
#include <limits>
#include <queue>
#include "Point.h"

class RTree {
private:
    Node* root;
    int max_entries;
    int min_entries;

    void chooseLeaf(const Entry& entry, Node*& leaf);
    void adjustTree(Node* node1, Node* node2);
    void splitNode(Node* node, Node*& new_node);
    void pickSeeds(Node* node, int& seed1, int& seed2);

public:
    RTree(int max_entries = 4, int min_entries = 2)
        : max_entries(max_entries), min_entries(min_entries) {
        root = new Node();
    }

    ~RTree() {
        delete root;
    }

    void insert(const Entry& entry);
    std::vector<int> search(const Rectangle& rect);
    int nearestNeighbor(const Point& p);
    std::vector<int> nearestNeighbors(const Point& p, int k);
};

#endif // RTREE_H
