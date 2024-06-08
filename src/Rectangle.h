#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <string>

#include "Point.h"

namespace SpatialIndex {

    class Rectangle {
    public:
        float minX;
        float minY;
        float maxX;
        float maxY;

        Rectangle();

        /**
       * Constructor.
           *
           * @param x1 coordinate of any corner of the rectangle
           * @param y1 (see x1)
           * @param x2 coordinate of the opposite corner
           * @param y2 (see x2)
       */
        Rectangle(float x1, float y1, float x2, float y2);

        /**
            * Sets the size of the rectangle.
            *
            * @param x1 coordinate of any corner of the rectangle
            * @param y1 (see x1)
            * @param x2 coordinate of the opposite corner
            * @param y2 (see x2)
        */
        void set(float x1, float y1, float x2, float y2);

        /**
            * Sets the size of this rectangle to equal the passed rectangle.
        */
        void set(Rectangle r);

        /**
            * Make a copy of this rectangle
            *
            * @return copy of this rectangle
        */
        Rectangle copy();

        /**
            * Determine whether an edge of this rectangle overlies the equivalent
            * edge of the passed rectangle
        */
        bool edgeOverlaps(Rectangle r);

        /**
            * Determine whether this rectangle intersects the passed rectangle
            *
            * @param r The rectangle that might intersect this rectangle
            *
            * @return true if the rectangles intersect, false if they do not intersect
        */
        bool intersects(Rectangle r);

        /**
            * Determine whether this rectangle contains the passed rectangle
            *
            * @param r The rectangle that might be contained by this rectangle
            *
            * @return true if this rectangle contains the passed rectangle, false if
            *         it does not
        */
        bool contains(Rectangle r);

        /**
            * Determine whether this rectangle is contained by the passed rectangle
            *
            * @param r The rectangle that might contain this rectangle
            *
            * @return true if the passed rectangle contains this rectangle, false if
            *         it does not
        */
        bool containedBy(Rectangle r);

        /**
            * Return the distance between this rectangle and the passed point.
            * If the rectangle contains the point, the distance is zero.
            *
            * @param p Point to find the distance to
            *
            * @return distance beween this rectangle and the passed point.
        */
        float distance(Point p);

        /**
            * Return the distance between this rectangle and the passed rectangle.
            * If the rectangles overlap, the distance is zero.
            *
            * @param r Rectangle to find the distance to
            *
            * @return distance between this rectangle and the passed rectangle
        */
        float distance(Rectangle r);

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
            * @return distance beween this rectangle and the passed point.
        */
        float distance(float minX, float minY, float maxX, float maxY, float pX, float pY);


        static float distanceSq(float minX, float minY, float maxX, float maxY, float pX, float pY);

        /**
            * Calculate the area by which this rectangle would be enlarged if
            * added to the passed rectangle. Neither rectangle is altered.
            *
            * @param r Rectangle to union with this rectangle, in order to
            *          compute the difference in area of the union and the
            *          original rectangle
            *
            * @return enlargement
        */
        float enlargement(Rectangle r);

        /**
            * Calculate the area by which a rectangle would be enlarged if
            * added to the passed rectangle..
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
        float enlargement(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                          float r2MinX, float r2MinY, float r2MaxX, float r2MaxY);

        /**
            * Compute the area of this rectangle.
            *
            * @return The area of this rectangle
        */
        float area();

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
        float area(float minX, float minY, float maxX, float maxY);

        /**
            * Computes the union of this rectangle and the passed rectangle, storing
            * the result in this rectangle.
            *
            * @param r Rectangle to add to this rectangle
        */
        void add(Rectangle r);

        /**
            * Computes the union of this rectangle and the passed point, storing
            * the result in this rectangle.
            *
            * @param p Point to add to this rectangle
        */
        void add(Point p);

        /**
            * Find the union of this rectangle and the passed rectangle.
            * Neither rectangle is altered
            *
            * @param r The rectangle to union with this rectangle
        */
        Rectangle findUnion(Rectangle r);

        union FloatIntUnion {
            float f;
            int i;
        };

        int floatToIntBits(float value);

        int hashCode();

        /**
            * Determine whether this rectangle is equal to a given object.
            * Equality is determined by the bounds of the rectangle.
            *
            * @param o The object to compare with this rectangle
        */
        bool equals(Rectangle r);

        /**
            * Determine whether this rectangle is the same as another object
            *
            * Note that two rectangles can be equal but not the same object,
            * if they both have the same bounds.
            *
            * @param o The object to compare with this rectangle.
        */
        bool sameObject(Rectangle r);

        /**
            * Return a string representation of this rectangle, in the form:
            * (1.2, 3.4), (5.6, 7.8)
            *
            * @return String String representation of this rectangle.
        */
        std::string toString();

        float width();

        float height();

        float aspectRatio();

        Point centre();
    };
}
#endif // RECTANGLE_H
