#include "RTree.h"
#include <algorithm>
#include <limits>
#include <queue>
#include <vector>

void RTree::chooseLeaf(const Entry& entry, Node*& leaf) {
    Node* node = root;
    while (!node->isLeaf()) {
        double min_enlargement = std::numeric_limits<double>::max();
        Node* next_node = nullptr;
        for (auto& child : node->children) {
            double enlargement = child->entries[0].rect.enlargement(entry.rect);
            if (enlargement < min_enlargement) {
                min_enlargement = enlargement;
                next_node = child;
            }
        }
        node = next_node;
    }
    leaf = node;
}

void RTree::adjustTree(Node* node1, Node* node2) {
    if (node1 == root) {
        if (node2) {
            Node* new_root = new Node(false);
            new_root->entries.push_back(Entry(node1->entries[0].rect, 0));
            new_root->children.push_back(node1);
            new_root->entries.push_back(Entry(node2->entries[0].rect, 0));
            new_root->children.push_back(node2);
            root = new_root;
        }
        return;
    }

    Node* parent = nullptr;
    for (Node* current = root; !current->isLeaf(); ) {
        for (auto& child : current->children) {
            if (child == node1 || (node2 && child == node2)) {
                parent = current;
                break;
            }
        }
        if (parent) break;
        double min_enlargement = std::numeric_limits<double>::max();
        Node* next_node = nullptr;
        for (auto& child : current->children) {
            double enlargement = child->entries[0].rect.enlargement(node1->entries[0].rect);
            if (enlargement < min_enlargement) {
                min_enlargement = enlargement;
                next_node = child;
            }
        }
        current = next_node;
    }

    if (parent) {
        for (auto& entry : parent->entries) {
            if (node1->entries[0].rect.contains(entry.rect)) {
                entry.rect = node1->entries[0].rect;
                break;
            }
        }
        if (node2) {
            parent->entries.push_back(Entry(node2->entries[0].rect, 0));
            parent->children.push_back(node2);
            if (parent->isFull(max_entries)) {
                Node* new_node = nullptr;
                splitNode(parent, new_node);
                adjustTree(parent, new_node);
            } else {
                adjustTree(parent, nullptr);
            }
        } else {
            adjustTree(parent, nullptr);
        }
    }
}

void RTree::splitNode(Node* node, Node*& new_node) {
    int seed1, seed2;
    pickSeeds(node, seed1, seed2);
    Node* new_node1 = new Node(node->isLeaf());
    Node* new_node2 = new Node(node->isLeaf());

    new_node1->entries.push_back(node->entries[seed1]);
    new_node2->entries.push_back(node->entries[seed2]);

    node->entries.erase(node->entries.begin() + seed2);
    node->entries.erase(node->entries.begin() + seed1);

    while (!node->entries.empty()) {
        if (new_node1->entries.size() + node->entries.size() == min_entries) {
            new_node1->entries.insert(new_node1->entries.end(),
                                      node->entries.begin(), node->entries.end());
            node->entries.clear();
            break;
        } else if (new_node2->entries.size() + node->entries.size() == min_entries) {
            new_node2->entries.insert(new_node2->entries.end(),
                                      node->entries.begin(), node->entries.end());
            node->entries.clear();
            break;
        }

        double min_enlargement1 = std::numeric_limits<double>::max();
        double min_enlargement2 = std::numeric_limits<double>::max();
        size_t next_entry_index = 0;

        for (size_t i = 0; i < node->entries.size(); ++i) {
            double enlargement1 = new_node1->entries[0].rect.enlargement(node->entries[i].rect);
            double enlargement2 = new_node2->entries[0].rect.enlargement(node->entries[i].rect);
            if (enlargement1 < min_enlargement1) {
                min_enlargement1 = enlargement1;
                next_entry_index = i;
            }
            if (enlargement2 < min_enlargement2) {
                min_enlargement2 = enlargement2;
                next_entry_index = i;
            }
        }

        if (min_enlargement1 < min_enlargement2) {
            new_node1->entries.push_back(node->entries[next_entry_index]);
        } else {
            new_node2->entries.push_back(node->entries[next_entry_index]);
        }
        node->entries.erase(node->entries.begin() + next_entry_index);
    }

    node->entries = new_node1->entries;
    new_node = new_node2;
}

void RTree::pickSeeds(Node* node, int& seed1, int& seed2) {
    double max_waste = -1;
    for (size_t i = 0; i < node->entries.size() - 1; ++i) {
        for (size_t j = i + 1; j < node->entries.size(); ++j) {
            double waste = node->entries[i].rect.enlargement(node->entries[j].rect);
            if (waste > max_waste) {
                max_waste = waste;
                seed1 = i;
                seed2 = j;
            }
        }
    }
}

void RTree::insert(const Entry& entry) {
    Node* leaf = nullptr;
    chooseLeaf(entry, leaf);

    leaf->entries.push_back(entry);
    if (leaf->isFull(max_entries)) {
        Node* new_node = nullptr;
        splitNode(leaf, new_node);
        adjustTree(leaf, new_node);
    } else {
        adjustTree(leaf, nullptr);
    }
}

std::vector<int> RTree::search(const Rectangle& rect) {
    std::vector<int> results;
    std::vector<Node*> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        Node* node = stack.back();
        stack.pop_back();

        if (node->isLeaf()) {
            for (const auto& entry : node->entries) {
                if (rect.intersects(entry.rect)) {
                    results.push_back(entry.data);
                }
            }
        } else {
            for (const auto& child : node->children) {
                if (rect.intersects(child->entries[0].rect)) {
                    stack.push_back(child);
                }
            }
        }
    }

    return results;
}

int RTree::nearestNeighbor(const Point& p) {
    std::priority_queue<std::pair<double, Node*>, std::vector<std::pair<double, Node*>>, std::greater<>> queue;
    queue.push({0, root});
    int nearest_data = -1;
    double min_distance = std::numeric_limits<double>::max();

    while (!queue.empty()) {
        auto [distance, node] = queue.top();
        queue.pop();

        if (node->isLeaf()) {
            for (const auto& entry : node->entries) {
                double dist = entry.rect.distanceTo(p);
                if (dist < min_distance) {
                    min_distance = dist;
                    nearest_data = entry.data;
                }
            }
        } else {
            for (const auto& child : node->children) {
                double dist = child->entries[0].rect.distanceTo(p);
                if (dist < min_distance) {
                    queue.push({dist, child});
                }
            }
        }
    }

    return nearest_data;
}

std::vector<int> RTree::nearestNeighbors(const Point& p, int k) {
    std::priority_queue<std::pair<double, Node*>, std::vector<std::pair<double, Node*>>, std::greater<>> queue;
    queue.push({0, root});
    std::priority_queue<std::pair<double, int>> results;

    while (!queue.empty()) {
        auto [distance, node] = queue.top();
        queue.pop();

        if (node->isLeaf()) {
            for (const auto& entry : node->entries) {
                double dist = entry.rect.distanceTo(p);
                if (results.size() < k) {
                    results.push({dist, entry.data});
                } else if (dist < results.top().first) {
                    results.pop();
                    results.push({dist, entry.data});
                }
            }
        } else {
            for (const auto& child : node->children) {
                double dist = child->entries[0].rect.distanceTo(p);
                if (results.size() < k || dist < results.top().first) {
                    queue.push({dist, child});
                }
            }
        }
    }

    std::vector<int> nearest;
    while (!results.empty()) {
        nearest.push_back(results.top().second);
        results.pop();
    }

    std::reverse(nearest.begin(), nearest.end());
    return nearest;
}
