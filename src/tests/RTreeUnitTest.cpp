//
// Created by serafm on 2/7/2024.
//

#include "gtest/gtest.h"
#include "../RTree.h"
#include "../Rectangle.h"

using namespace SpatialIndex;

class RTreeUnitTest : public ::testing::Test {
protected:
    RTree rtree{};

    void SetUp() {
        rtree.init();
    }
};

TEST_F(RTreeUnitTest, TestAddAndFind) {
    Rectangle rect1(0, 0, 1, 1);
    Rectangle rect2(2, 2, 3, 3);
    Rectangle rect3(4, 4, 5, 5);

    rtree.add(rect1, 1);
    rtree.add(rect2, 2);
    rtree.add(rect3, 3);

    Rectangle searchRect(0, 0, 5, 5);
    bool found[4] = {false, false, false, false}; // Index 0 is unused

    Collections::Procedure procedure;// = [&](int id) {
    //found[id] = true;
    //return true;
    //};

    rtree.intersects(searchRect, procedure);

    EXPECT_TRUE(found[1]);
    EXPECT_TRUE(found[2]);
    EXPECT_TRUE(found[3]);
}

TEST_F(RTreeUnitTest, TestDelete) {
    Rectangle rect1(0, 0, 1, 1);
    Rectangle rect2(2, 2, 3, 3);
    Rectangle rect3(4, 4, 5, 5);

    rtree.add(rect1, 1);
    rtree.add(rect2, 2);
    rtree.add(rect3, 3);

    bool deleted = rtree.del(rect2, 2);
    EXPECT_TRUE(deleted);

    Rectangle searchRect(2, 2, 3, 3);
    bool found[4] = {false, false, false, false}; // Index 0 is unused

    Collections::Procedure procedure;// = [&](int id) {
        //found[id] = true;
        //return true;
    //};

    rtree.intersects(searchRect, procedure);

    EXPECT_FALSE(found[2]);
}

TEST_F(RTreeUnitTest, TestNearestNeighbor) {
    Rectangle rect1(0, 0, 1, 1);
    Rectangle rect2(2, 2, 3, 3);
    Rectangle rect3(4, 4, 5, 5);

    rtree.add(rect1, 1);
    rtree.add(rect2, 2);
    rtree.add(rect3, 3);

    Point p(2.5, 2.5);
    bool found[4] = {false, false, false, false}; // Index 0 is unused

    Collections::Procedure procedure;// = [&](int id) {
    //found[id] = true;
    //return true;
    //};

    rtree.nearest(p, procedure, 10.0);

    EXPECT_TRUE(found[2]);
    EXPECT_FALSE(found[1]);
    EXPECT_FALSE(found[3]);
}

TEST_F(RTreeUnitTest, TestTreeSize) {
    Rectangle rect1(0, 0, 1, 1);
    Rectangle rect2(2, 2, 3, 3);
    Rectangle rect3(4, 4, 5, 5);

    rtree.add(rect1, 1);
    rtree.add(rect2, 2);
    rtree.add(rect3, 3);

    EXPECT_EQ(rtree.treeSize(), 3);

    rtree.del(rect2, 2);

    EXPECT_EQ(rtree.treeSize(), 2);
}

TEST_F(RTreeUnitTest, TestBounds) {
    Rectangle rect1(0, 0, 1, 1);
    Rectangle rect2(2, 2, 3, 3);
    Rectangle rect3(4, 4, 5, 5);

    rtree.add(rect1, 1);
    rtree.add(rect2, 2);
    rtree.add(rect3, 3);

    Rectangle bounds = rtree.getBounds();
    EXPECT_EQ(bounds.minX, 0);
    EXPECT_EQ(bounds.minY, 0);
    EXPECT_EQ(bounds.maxX, 5);
    EXPECT_EQ(bounds.maxY, 5);
}
