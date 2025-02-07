#include "Point.h"

namespace rtree {

    Point::Point(float x, float y) {
        this->x = x;
        this->y = y;
    }

    void Point::set(const Point & other) {
        this->x = other.x;
        this->y = other.y;
    }

    std::string Point::toString() const {
        return "(" + std::to_string(this->x) + "," + std::to_string(this->y) + ")";
    }
}
