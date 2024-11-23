#include <gtest/gtest.h>
#include "../RTree.h"

using namespace spatialindex;

/*
TEST(RTreeTest, AddAndSearch) {
    RTree tree{};
    Rectangle rect1{0, 0, 10, 10};
    Rectangle rect2{0, 0, 5, 5};
    tree.add(rect1, 1);
    tree.add(rect2, 1);

    Rectangle searchRect(5, 5, 20, 20);
    tree.intersects(searchRect);

    EXPECT_EQ(results.size(), 2); // Both rectangles intersect
    EXPECT_TRUE(results.count(1));
    EXPECT_TRUE(results.count(2));

}

TEST(RTreeTest, Delete) {
}

TEST(RTreeTest, NearestNeighbor) {
}
*/