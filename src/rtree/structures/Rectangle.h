#pragma once

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <string>
#include <vector>
#include "Point.h"

namespace rtree {

class Rectangle {
public:

    float minX = 0;
    float minY = 0;
    float maxX = 0;
    float maxY = 0;
    int id = 0;

    std::vector<Point> points;

    /**
    * Constructor.
    *
    * @param x1 value of minX
    * @param y1 value of minY
    * @param x2 value of maxX
    * @param y2 value of maxY
   */
    Rectangle(float x1, float y1, float x2, float y2, int id);

    /**
    * Constructor without id.
    *
    * @param x1 value of minX
    * @param y1 value of minY
    * @param x2 value of maxX
    * @param y2 value of maxY
    */
    Rectangle(float x1, float y1, float x2, float y2);

    /**
    * Make a copy of this rectangle
    *
    * @return copy of this rectangle
    */
    Rectangle copy() const;

    /**
    * Determine whether the edge of this rectangle overlies the equivalent
    * edge of the passed rectangle
    */
    bool edgeOverlaps(Rectangle& r) const;

    /**
    * Determine whether two rectangles intersect
    *
    * @param r1MinX minimum X coordinate of rectangle 1
    * @param r1MinY minimum Y coordinate of rectangle 1
    * @param r1MaxX maximum X coordinate of rectangle 1
    * @param r1MaxY maximum Y coordinate of rectangle 1
    * @param r2MinX minimum X coordinate of rectangle 2
    * @param r2MinY minimum Y coordinate of rectangle 2
    * @param r2MaxX maximum X coordinate of rectangle 2
    * @param r2MaxY maximum Y coordinate of rectangle 2
    *
    * @return true if rectangle intersects range, false otherwise.
    */
    [[nodiscard]] bool intersects(float rangeMinX, float rangeMinY, float rangeMaxX, float rangeMaxY) const {
        return maxX >= rangeMinX && minX <= rangeMaxX && maxY >= rangeMinY && minY <= rangeMaxY;
    }

    static inline bool contains(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                                 float r2MinX, float r2MinY, float r2MaxX, float r2MaxY) {
        return r1MaxX >= r2MaxX && r1MinX <= r2MinX && r1MaxY >= r2MaxY && r1MinY <= r2MinY;
    }

    /**
    * Return the distance between a rectangle and a point.
    * If the rectangle contains the point, the distance is zero.
    *
    * @param minX minimum X coordinate of rectangle
    * @param minY minimum Y coordinate of rectangle
    * @param maxX maximum X coordinate of rectangle
    * @param maxY maximum Y coordinate of rectangle
    * @param pX X coordinate of point
    * @param pY Y coordinate of point
    *
    * @return distance between this rectangle and the passed point.
    */
    static inline float distance(float minX, float minY,
                           float maxX, float maxY,
                           float x, float y) {
        float dx = 0.0f, dy = 0.0f;

        // Compute horizontal distance
        if (x < minX) {
            dx = minX - x;
        } else if (x > maxX) {
            dx = x - maxX;
        }

        // Compute vertical distance
        if (y < minY) {
            dy = minY - y;
        } else if (y > maxY) {
            dy = y - maxY;
        }

        return (dx * dx) + (dy * dy);
    }

    /**
    * Compute the area of a rectangle.
    *
    * @param minX the minimum X coordinate of the rectangle
    * @param minY the minimum Y coordinate of the rectangle
    * @param maxX the maximum X coordinate of the rectangle
    * @param maxY the maximum Y coordinate of the rectangle
    *
    * @return The area of the rectangle
    */
    static float area(float minX, float minY, float maxX, float maxY);

    /**
    * Adds a new point in this rectangle.
    *
    * @param p Point to add to this rectangle
    */
    void add(Point p);

    /**
    * Determine whether this rectangle is equal to a given rectangle.
    * Equality is determined by the bounds of the rectangle.
    *
    * @param r The rectangle to compare with this rectangle
    */
    bool equals(Rectangle& r) const;

    /**
     * @brief Computes the width of the rectangle.
     *
     * @return The width of the rectangle (maxX - minX).
     */
    [[nodiscard]] float width() const;

    /**
     * @brief Computes the height of the rectangle.
     *
     * @return The height of the rectangle (maxY - minY).
     */
    [[nodiscard]] float height() const;

    /**
     * @brief Computes the aspect ratio of the rectangle.
     *
     * @return The aspect ratio (width / height).
     */
    [[nodiscard]] float aspectRatio() const;

    /**
     * @brief Computes the center point of the rectangle.
     *
     * @return A Point representing the center of the rectangle.
     */
    [[nodiscard]] Point center() const;

    /**
     * @brief Generates a string representation of the rectangle.
     *
     * @return Coordinates of a rectangle.
     */
    [[nodiscard]] std::string toString() const;


};
}
#endif // RECTANGLE_H
