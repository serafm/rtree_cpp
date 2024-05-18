#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "Point.h"

class Rectangle {
public:
    double xmin, ymin, xmax, ymax;

    Rectangle(double xmin = 0, double ymin = 0, double xmax = 0, double ymax = 0);

    bool intersects(const Rectangle& other) const;

    bool contains(const Rectangle& other) const;

    double area() const;

    double enlargement(const Rectangle& other) const;

    double distanceTo(const Point& p) const;
};

#endif // RECTANGLE_H
