#include "Rectangle.h"
#include <algorithm>
#include <cmath>

namespace rtree {

    Rectangle::Rectangle()
      : minX(MAXFLOAT),
        minY(MAXFLOAT),
        maxX(-MAXFLOAT),
        maxY(-MAXFLOAT) {}

    Rectangle::Rectangle(float x1, float y1, float x2, float y2) {
        this->minX = std::min(x1, x2);
        this->minY = std::min(y1, y2);
        this->maxX = std::max(x1, x2);
        this->maxY = std::max(y1, y2);
    }

    Rectangle Rectangle::copy() const {
        return {minX, minY, maxX, maxY};
    }

    bool Rectangle::edgeOverlaps(Rectangle& r) const {
        return minX == r.minX || maxX == r.maxX || minY == r.minY || maxY == r.maxY;
    }

    bool Rectangle::intersects(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                                 float r2MinX, float r2MinY, float r2MaxX, float r2MaxY) {
        return r1MaxX >= r2MinX && r1MinX <= r2MaxX && r1MaxY >= r2MinY && r1MinY <= r2MaxY;
    }

    bool Rectangle::contains(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                                 float r2MinX, float r2MinY, float r2MaxX, float r2MaxY) {
        return r1MaxX >= r2MaxX && r1MinX <= r2MinX && r1MaxY >= r2MaxY && r1MinY <= r2MinY;
    }

    float Rectangle::distanceSq(float minX, float minY, float maxX, float maxY, float pX, float pY) {
        // Check if the point is inside the rectangle
        if (pX >= minX && pX <= maxX && pY >= minY && pY <= maxY) {
            return 0.0f; // Distance is zero if the point is inside the rectangle
        }

        // Calculate distances to each edge
        float distanceLeft = std::max(0.0f, minX - pX);   // Distance to left edge
        float distanceRight = std::max(0.0f, pX - maxX);  // Distance to right edge
        float distanceBottom = std::max(0.0f, minY - pY); // Distance to bottom edge
        float distanceTop = std::max(0.0f, pY - maxY);    // Distance to top edge

        // Horizontal and vertical distances (min distance to horizontal and vertical edges)
        float distanceX = std::max(distanceLeft, distanceRight);
        float distanceY = std::max(distanceBottom, distanceTop);

        // Return the Euclidean distance
        return std::sqrt(distanceX * distanceX + distanceY * distanceY);
    }


    float Rectangle::enlargement(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY, float r2MinX, float r2MinY, float r2MaxX, float r2MaxY) {
        // Calculate the area of r1
        float r1Area = (r1MaxX - r1MinX) * (r1MaxY - r1MinY);

        if (r1Area == MAXFLOAT) {
            return 0; // cannot enlarge an infinite rectangle...
        }

        // Update the bounding box of r1 to include r2
        if (r2MinX < r1MinX) r1MinX = r2MinX;
        if (r2MinY < r1MinY) r1MinY = r2MinY;
        if (r2MaxX > r1MaxX) r1MaxX = r2MaxX;
        if (r2MaxY > r1MaxY) r1MaxY = r2MaxY;

        // Calculate the area of the union of r1 and r2
        float r1r2UnionArea = (r1MaxX - r1MinX) * (r1MaxY - r1MinY);

        if (r1r2UnionArea == MAXFLOAT) {
            // Handle large area overflow
            return MAXFLOAT;
        }

        // Return the enlargement of r1 to fit r2
        return r1r2UnionArea - r1Area;
    }

    float Rectangle::area(float minX, float minY, float maxX, float maxY) {
        return (maxX - minX) * (maxY - minY);
    }

    void Rectangle::add(Point p) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    bool Rectangle::equals(Rectangle& r) const {
        return maxX == r.maxX && maxY == r.maxY && minX == r.minX && minY == r.minY;
    }

    float Rectangle::width() const {
        return maxX - minX;
    }

    float Rectangle::height() const {
        return maxY - minY;
    }

    float Rectangle::aspectRatio() const {
        return width() / height();
    }

    Point Rectangle::centre() const {
        return {(minX + maxX) / 2, (minY + maxY) / 2};
    }

    std::string Rectangle::toString() const {
        return "(" + std::to_string(this->minX) + ", " + std::to_string(this->minY) + ", " + std::to_string(this->maxX) + ", " + std::to_string(this->maxY) + ")";
    }
}
