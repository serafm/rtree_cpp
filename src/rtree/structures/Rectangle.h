#pragma once

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <string>
#include "Point.h"

namespace rtree {

    class Rectangle {
    public:
        float minX{};
        float minY{};
        float maxX{};
        float maxY{};
        int id{};

        /**
       * Constructor.
           *
           * @param x1 coordinate of any corner of the rectangle
           * @param y1 (see x1)
           * @param x2 coordinate of the opposite corner
           * @param y2 (see x2)
       */
        Rectangle(float x1, float y1, float x2, float y2, int id);

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

        /*static bool intersects(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                                float r2MinX, float r2MinY, float r2MaxX, float r2MaxY) {
            return r1MaxX >= r2MinX && r1MinX <= r2MaxX && r1MaxY >= r2MinY && r1MinY <= r2MaxY;
        }
        */
        static inline bool contains(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                                     float r2MinX, float r2MinY, float r2MaxX, float r2MaxY) {
            return r1MaxX >= r2MaxX && r1MinX <= r2MinX && r1MaxY >= r2MaxY && r1MinY <= r2MinY;
        }


        /**
         * Determine whether if r1 rectangle contains r2 rectangle
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
         * @return true if r1 contains r2, false otherwise.
         */
        /*static bool contains(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                                 float r2MinX, float r2MinY, float r2MaxX, float r2MaxY);*/

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
        static inline float distanceSq(const float minX, const float minY,
                                        const float maxX, const float maxY,
                                        const float x, const float y) {
            float dx = (x < minX) ? (minX - x) : ((x > maxX) ? (x - maxX) : 0.0f);
            float dy = (y < minY) ? (minY - y) : ((y > maxY) ? (y - maxY) : 0.0f);
            return dx * dx + dy * dy;
        }

        /**
            * Calculate the area by which a rectangle would be enlarged if
            * added to the passed rectangle.
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
            * @return enlargement
        */
        static float enlargement(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                          float r2MinX, float r2MinY, float r2MaxX, float r2MaxY);

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
            * Computes the union of this rectangle and the passed point, storing
            * the result in this rectangle.
            *
            * @param p Point to add to this rectangle
        */
        void add(Point p);

        /**
            * Determine whether this rectangle is equal to a given object.
            * Equality is determined by the bounds of the rectangle.
            *
            * @param r The object to compare with this rectangle
        */
        bool equals(Rectangle& r) const;

        float width() const;

        float height() const;

        float aspectRatio() const;

        Point centre() const;

        std::string toString() const;

    };
}
#endif // RECTANGLE_H
