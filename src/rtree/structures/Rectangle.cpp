#include "Rectangle.h"
#include <cmath>

namespace rtree {

    Rectangle::Rectangle(float x1, float y1, float x2, float y2, int id) {
        this->minX = x1;
        this->minY = y1;
        this->maxX = x2;
        this->maxY = y2;
        this->id = id;
    }

    Rectangle::Rectangle(float x1, float y1, float x2, float y2) {
        this->minX = x1;
        this->minY = y1;
        this->maxX = x2;
        this->maxY = y2;
    }

    bool Rectangle::edgeOverlaps(Rectangle& r) const {
        return minX == r.minX || maxX == r.maxX || minY == r.minY || maxY == r.maxY;
    }

   /* bool Rectangle::intersects(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                                 float r2MinX, float r2MinY, float r2MaxX, float r2MaxY) {
        return r1MaxX >= r2MinX && r1MinX <= r2MaxX && r1MaxY >= r2MinY && r1MinY <= r2MaxY;
    }

    bool Rectangle::contains(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                                 float r2MinX, float r2MinY, float r2MaxX, float r2MaxY) {
        return r1MaxX >= r2MaxX && r1MinX <= r2MinX && r1MaxY >= r2MaxY && r1MinY <= r2MinY;
    }*/

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
