#include "Point.h"
#include <cmath>

namespace spatialindex {

    Point::Point(float x, float y) {
        this->x = x;
        this->y = y;
    }

    void Point::set(Point& other) {
        this->x = other.x;
        this->y = other.y;
    }

    int Point::xInt() {
        return static_cast<int>(std::round(this->x));
    }

    int Point::yInt() {
        return static_cast<int>(std::round(this->y));
    }
}
