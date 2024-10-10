#pragma once

#ifndef POINT_H
#define POINT_H
#include <string>

namespace spatialindex {

    class Point {
    public:

        /**
        * The (x, y) coordinates of the point.
        */
        float x;
        float y;

        /**
        * Constructor.
        *
        * @param x The x coordinate of the point
        * @param y The y coordinate of the point
        */
        Point(float x, float y);

        /**
        * Copy from another point into this one
        */
        void set(Point& other);

        /**
        * Print as a string in format "(x, y)"
        */
        std::string toString();

        /**
        * @return X coordinate rounded to an int
        */
        int xInt();

        /**
        * @return Y coordinate rounded to an int
        */
        int yInt();
    };
}
#endif // POINT_H
