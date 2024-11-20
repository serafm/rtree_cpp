#include "Rectangle.h"
#include <algorithm>
#include <cmath>

namespace spatialindex {

    Rectangle::Rectangle() {
        this->minX = std::numeric_limits<float>::max();
        this->minY = std::numeric_limits<float>::max();
        this->maxX = std::numeric_limits<float>::min();
        this->maxY = std::numeric_limits<float>::min();
    }

    Rectangle::Rectangle(float x1, float y1, float x2, float y2) {
        this->minX = std::min(x1, x2);
        this->minY = std::min(y1, y2);
        this->maxX = std::max(x1, x2);
        this->maxY = std::max(y1, y2);
    }

    Rectangle Rectangle::copy() const {
        return {minX, minY, maxX, maxY};
    }

    bool Rectangle::isEmpty() const {
        return minX == 0 && maxX == 0 && minY == 0 && maxY == 0;
    }

    bool Rectangle::edgeOverlaps(Rectangle& r) const {
        return minX == r.minX || maxX == r.maxX || minY == r.minY || maxY == r.maxY;
    }

    bool Rectangle::intersects(Rectangle& r) const {
        return maxX >= r.minX && minX <= r.maxX && maxY >= r.minY && minY <= r.maxY;
    }

    bool Rectangle::intersects(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                                float r2MinX, float r2MinY, float r2MaxX, float r2MaxY) {
        return r1MaxX >= r2MinX && r1MinX <= r2MaxX && r1MaxY >= r2MinY && r1MinY <= r2MaxY;
    }

    bool Rectangle::contains(Rectangle& r) const {
        return maxX >= r.maxX && minX <= r.minX && maxY >= r.maxY && minY <= r.minY;
    }

    bool Rectangle::contains(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY,
                                 float r2MinX, float r2MinY, float r2MaxX, float r2MaxY) {
        return r1MaxX >= r2MaxX && r1MinX <= r2MinX && r1MaxY >= r2MaxY && r1MinY <= r2MinY;
    }

    bool Rectangle::containedBy(Rectangle& r) const {
        return r.maxX >= maxX && r.minX <= minX && r.maxY >= maxY && r.minY <= minY;
    }

    float Rectangle::distance(Point& p) const {
        float distanceSquared = 0;

        // Calculate the distance in the X direction.
        float temp = minX - p.x;

        // If temp is negative, the point is to the right of minX.
        // Adjust temp to be the distance from p.x to maxX.
        if (temp < 0) {
            temp = p.x - maxX;
        }

        // If temp is still positive, the point is outside the rectangle in the X direction.
        // Add the square of the X distance to the total distance squared.
        if (temp > 0) {
            distanceSquared += (temp * temp);
        }

        // Calculate the distance in the Y direction.
        temp = minY - p.y;

        // If temp is negative, the point is above minY.
        // Adjust temp to be the distance from p.y to maxY.
        if (temp < 0) {
            temp = p.y - maxY;
        }

        // If temp is still positive, the point is outside the rectangle in the Y direction.
        // Add the square of the Y distance to the total distance squared.
        if (temp > 0) {
            distanceSquared += (temp * temp);
        }

        return std::sqrt(distanceSquared);
    }

    /*float Rectangle::distance(float minX, float minY, float maxX, float maxY, float pX, float pY) {
        return std::sqrt(distanceSq(minX, minY, maxX, maxY, pX, pY));
    }*/

    float Rectangle::distanceSq(float minX, float minY, float maxX, float maxY, float pX, float pY) {
        float distanceX = 0;
        float distanceY = 0;

        if (minX > pX) {
            distanceX = minX - pX;
        } else if (pX > maxX) {
            distanceX = pX - maxX;
        }

        if (minY > pY) {
            distanceY = minY - pY;
        } else if (pY > maxY) {
            distanceY = pY - maxY;
        }

        return std::sqrt(distanceX * distanceX + distanceY * distanceY);
    }

    /*float Rectangle::distance(Rectangle r) const {
        float distanceSquared = 0;
        float greatestMin = std::max(minX, r.minX);
        float leastMax = std::min(maxX, r.maxX);
        if (greatestMin > leastMax) {
            distanceSquared += (greatestMin - leastMax) * (greatestMin - leastMax);
        }
        greatestMin = std::max(minY, r.minY);
        leastMax = std::min(maxY, r.maxY);
        if (greatestMin > leastMax) {
            distanceSquared += (greatestMin - leastMax) * (greatestMin - leastMax);
        }
        return std::sqrt(distanceSquared);
    }*/

    float Rectangle::enlargement(Rectangle& r) {
        float enlargedArea = ( std::max(maxX, r.maxX) - std::min(minX, r.minX) ) *
                             ( std::max(maxY, r.maxY) - std::min(minY, r.minY) );
        return enlargedArea - area();
    }

    float Rectangle::enlargement(float r1MinX, float r1MinY, float r1MaxX, float r1MaxY, float r2MinX, float r2MinY, float r2MaxX, float r2MaxY) {
        float r1Area = (r1MaxX - r1MinX) * (r1MaxY - r1MinY);

        if (r1Area == std::numeric_limits<float>::max()) {
            return 0;
        }

        if (r2MinX < r1MinX) r1MinX = r2MinX;
        if (r2MinY < r1MinY) r1MinY = r2MinY;
        if (r2MaxX > r1MaxX) r1MaxX = r2MaxX;
        if (r2MaxY > r1MaxY) r1MaxY = r2MaxY;

        float r1r2UnionArea = (r1MaxX - r1MinX) * (r1MaxY - r1MinY);

        if (r1r2UnionArea == std::numeric_limits<float>::max()) {
            return std::numeric_limits<float>::max();
        }

        return r1r2UnionArea - r1Area;
    }

    float Rectangle::area() const {
        return (maxX - minX) * (maxY - minY);
    }

    float Rectangle::area(float minX, float minY, float maxX, float maxY) {
        return (maxX - minX) * (maxY - minY);
    }

    void Rectangle::add(Rectangle r) {
        if (r.minX < minX) minX = r.minX;
        if (r.maxX > maxX) maxX = r.maxX;
        if (r.minY < minY) minY = r.minY;
        if (r.maxY > maxY) maxY = r.maxY;
    }

    void Rectangle::add(Point p) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    Rectangle Rectangle::findUnion(Rectangle& r) const {
        Rectangle union_ = this->copy();
        union_.add(r);
        return union_;
    }

    int Rectangle::floatToIntBits(float value) {
        FloatIntUnion u{};
        u.f = value;
        return u.i;
    }

    int Rectangle::hashCode() const {
        static int prime = 31;
        int result = 1;
        result = prime * result + floatToIntBits(maxX);
        result = prime * result + floatToIntBits(maxY);
        result = prime * result + floatToIntBits(minX);
        result = prime * result + floatToIntBits(minY);
        return result;
    }

    bool Rectangle::equals(Rectangle& r) const {
        return maxX == r.maxX && maxY == r.maxY && minX == r.minX && minY == r.minY;
    }

    bool Rectangle::sameObject(Rectangle& r) {
        // TODO: The object to compare with this rectangle.
        return true;
    }

    std::string Rectangle::toString() {
        return "";
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

    // Adjusts the rectangle to include the area specified by the provided coordinates
    static void adjustMBR(Rectangle& rect, float childMinX, float childMinY, float childMaxX, float childMaxY) {
        rect.minX = std::min(rect.minX, childMinX);
        rect.minY = std::min(rect.minY, childMinY);
        rect.maxX = std::max(rect.maxX, childMaxX);
        rect.maxY = std::max(rect.maxY, childMaxY);
    }
}
