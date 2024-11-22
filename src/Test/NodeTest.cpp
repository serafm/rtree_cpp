#include <gtest/gtest.h>
#include "../CreateSpatialIndex.h"
#include "../Node.h"

using namespace spatialindex;

/*
TEST(NodeTest, AddEntry) {
    Node node(1, 2); // Level 1, max 2 entries
    node.addEntry(0, 0, 10, 10, 100);
    EXPECT_EQ(node.entryCount, 1);

    node.addEntry(5, 5, 15, 15, 200);
    EXPECT_EQ(node.entryCount, 2);

    // Adding beyond max should fail (depends on implementation)
    EXPECT_THROW(node.addEntry(20, 20, 30, 30, 300), std::runtime_error);
}

TEST(NodeTest, DeleteEntry) {
    Node node(1, 3);
    node.addEntry(0, 0, 10, 10, 100);
    node.addEntry(5, 5, 15, 15, 200);

    node.deleteEntry(1); // Delete second entry
    EXPECT_EQ(node.entryCount, 1);

    EXPECT_THROW(node.deleteEntry(1), std::out_of_range); // Invalid index
}
*/