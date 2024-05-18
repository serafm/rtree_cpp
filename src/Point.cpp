#include "Point.h"
#include <cmath>

Point::Point(double x, double y) : x(x), y(y) {}

double Point::distanceTo(const Point& other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    return std::sqrt(dx * dx + dy * dy);
}
