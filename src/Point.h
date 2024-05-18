#ifndef POINT_H
#define POINT_H

class Point {
public:
    double x, y;

    Point(double x = 0, double y = 0);

    double distanceTo(const Point& other) const;
};

#endif // POINT_H
