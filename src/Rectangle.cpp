#include "Rectangle.h"
#include <algorithm>
#include <cmath>

Rectangle::Rectangle(double xmin, double ymin, double xmax, double ymax)
    : xmin(xmin), ymin(ymin), xmax(xmax), ymax(ymax) {}

bool Rectangle::intersects(const Rectangle& other) const {
    return !(xmin > other.xmax || xmax < other.xmin ||
             ymin > other.ymax || ymax < other.ymin);
}

bool Rectangle::contains(const Rectangle& other) const {
    return (xmin <= other.xmin && xmax >= other.xmax &&
            ymin <= other.ymin && ymax >= other.ymax);
}

double Rectangle::area() const {
    return (xmax - xmin) * (ymax - ymin);
}

double Rectangle::enlargement(const Rectangle& other) const {
    double new_xmin = std::min(xmin, other.xmin);
    double new_ymin = std::min(ymin, other.ymin);
    double new_xmax = std::max(xmax, other.xmax);
    double new_ymax = std::max(ymax, other.ymax);
    Rectangle new_rect(new_xmin, new_ymin, new_xmax, new_ymax);
    return new_rect.area() - this->area();
}

double Rectangle::distanceTo(const Point& p) const {
    double dx = std::max({xmin - p.x, 0.0, p.x - xmax});
    double dy = std::max({ymin - p.y, 0.0, p.y - ymax});
    return std::sqrt(dx * dx + dy * dy);
}
