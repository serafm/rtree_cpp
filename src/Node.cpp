#include "Node.h"

Node::Node(bool is_leaf) : is_leaf(is_leaf) {}

Node::~Node() {
    for (auto child : children) {
        delete child;
    }
}

bool Node::isLeaf() const {
    return is_leaf;
}

bool Node::isFull(int max_entries) const {
    return entries.size() >= max_entries;
}
