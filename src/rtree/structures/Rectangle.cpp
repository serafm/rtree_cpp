#include "Rectangle.h"

namespace rtree {

    Rectangle::Rectangle(float x1, float y1, float x2, float y2, int id) {
        this->minX = x1;
        this->minY = y1;
        this->maxX = x2;
        this->maxY = y2;
        this->id = id;
    }


    Rectangle::Rectangle(float x1, float y1, float x2, float y2) {
        this->minX = x1;
        this->minY = y1;
        this->maxX = x2;
        this->maxY = y2;
    }

    bool Rectangle::edgeOverlaps(Rectangle& r) const {
        return minX == r.minX || maxX == r.maxX || minY == r.minY || maxY == r.maxY;
    }

    float Rectangle::area(float minX, float minY, float maxX, float maxY) {
        return (maxX - minX) * (maxY - minY);
    }

    void Rectangle::add(Point p) {
        if (p.x > minX && p.x < maxX && p.y > minY && p.y < maxY) {
            this->points.push_back(p);
        }
    }

    bool Rectangle::equals(Rectangle& r) const {
        return maxX == r.maxX && maxY == r.maxY && minX == r.minX && minY == r.minY;
    }

    float Rectangle::width() const {
        return maxX - minX;
    }

    float Rectangle::height() const {
        return maxY - minY;
    }

    float Rectangle::aspectRatio() const {
        return width() / height();
    }

    Point Rectangle::center() const {
        return {(minX + maxX) / 2, (minY + maxY) / 2};
    }

    std::string Rectangle::toString() const {
        return "(" + std::to_string(this->minX) + ", " + std::to_string(this->minY) + ", " + std::to_string(this->maxX) + ", " + std::to_string(this->maxY) + ")";
    }
}
