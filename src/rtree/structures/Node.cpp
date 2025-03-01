#include "Node.h"

namespace rtree {

    Node::Node(int id, int level, int capacity)
    : nodeId(id),
    level(level)
    {
        children.reserve(capacity);
        leafs.reserve(capacity);
    }

    Node::Node(int capacity) {
        children.reserve(capacity);
        leafs.reserve(capacity);
    }

    Node::~Node() = default;

    void Node::addChildEntry(Node* n) {
        children.push_back(n);
        ids.push_back(n->nodeId);

        if (n->mbrMinX < mbrMinX) mbrMinX = n->mbrMinX;
        if (n->mbrMinY < mbrMinY) mbrMinY = n->mbrMinY;
        if (n->mbrMaxX > mbrMaxX) mbrMaxX = n->mbrMaxX;
        if (n->mbrMaxY > mbrMaxY) mbrMaxY = n->mbrMaxY;
    }

    void Node::addLeafEntry(const Rectangle& rect) {
        leafs.push_back(rect);
        ids.push_back(rect.id);

        if (rect.minX < mbrMinX) mbrMinX = rect.minX;
        if (rect.minY < mbrMinY) mbrMinY = rect.minY;
        if (rect.maxX > mbrMaxX) mbrMaxX = rect.maxX;
        if (rect.maxY > mbrMaxY) mbrMaxY = rect.maxY;
    }
    void Node::sortChildrenByMinX() {
        std::sort(children.begin(), children.end(),
            [](const Node* a, const Node* b) {
                return a->mbrMinX < b->mbrMinX;
            });
    }

    void Node::sortLeafsByMinX() {
        std::sort(leafs.begin(), leafs.end(),
            [](const Rectangle& a, const Rectangle& b) {
                return a.minX < b.minX;
            });
    }

    void Node::deleteEntry(int index) {
        if (isLeaf()) {
            // Delete from leaf entries
            leafs.erase(leafs.begin() + index);
        } else {
            // Delete from children
            children.erase(children.begin() + index);
        }
    
        // Remove ID from ids vector
        ids.erase(ids.begin() + index);
    
        // Recalculate MBR after deletion
        recalculateMBR();
    }
    
    void Node::recalculateMBR() {
        if (isLeaf()) {
            // Recalculate MBR based on leaf rectangles
            if (leafs.empty()) {
                // Reset MBR to initial state if no entries left
                mbrMinX = MAXFLOAT;
                mbrMinY = MAXFLOAT;
                mbrMaxX = -MAXFLOAT;
                mbrMaxY = -MAXFLOAT;
                return;
            }
    
            mbrMinX = mbrMinY = MAXFLOAT;
            mbrMaxX = mbrMaxY = -MAXFLOAT;
    
            for (const auto& rect : leafs) {
                if (rect.minX < mbrMinX) mbrMinX = rect.minX;
                if (rect.minY < mbrMinY) mbrMinY = rect.minY;
                if (rect.maxX > mbrMaxX) mbrMaxX = rect.maxX;
                if (rect.maxY > mbrMaxY) mbrMaxY = rect.maxY;
            }
        } else {
            // Recalculate MBR based on child nodes
            if (children.empty()) {
                // Reset MBR to initial state if no entries left
                mbrMinX = MAXFLOAT;
                mbrMinY = MAXFLOAT;
                mbrMaxX = -MAXFLOAT;
                mbrMaxY = -MAXFLOAT;
                return;
            }
    
            mbrMinX = mbrMinY = MAXFLOAT;
            mbrMaxX = mbrMaxY = -MAXFLOAT;
    
            for (const auto* child : children) {
                if (child->mbrMinX < mbrMinX) mbrMinX = child->mbrMinX;
                if (child->mbrMinY < mbrMinY) mbrMinY = child->mbrMinY;
                if (child->mbrMaxX > mbrMaxX) mbrMaxX = child->mbrMaxX;
                if (child->mbrMaxY > mbrMaxY) mbrMaxY = child->mbrMaxY;
            }
        }
    }

    int Node::getEntryCount() const {
        return entryCount;
    }

    int Node::getNodeId() const {
        return nodeId;
    }

    bool Node::isLeaf() const {
        return level == 1;
    }

    int Node::getLevel() const {
        return level;
    }

    bool Node::isEmpty() const {
        return this->children.empty() || this->leafs.empty();
    }

}
