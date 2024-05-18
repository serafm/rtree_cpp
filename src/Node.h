#ifndef NODE_H
#define NODE_H

#include <vector>
#include "Entry.h"

class Node {
public:
    bool is_leaf;
    std::vector<Entry> entries;
    std::vector<Node*> children;

    Node(bool is_leaf = true);

    ~Node();

    bool isLeaf() const;

    bool isFull(int max_entries) const;
};

#endif // NODE_H
