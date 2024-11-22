#include "CreateSpatialIndex.h"
using namespace spatialindex;

int main() {
    spatialindex::CreateSpatialIndex::BuildTree();
    spatialindex::CreateSpatialIndex::ReadAndExecuteQueries("/home/serafm/dev/rtee_test_data/knn_queries.csv", 0);
    spatialindex::CreateSpatialIndex::ReadAndExecuteQueries("/home/serafm/dev/rtee_test_data/range_queries.csv", 1);
    // spatialindex::CreateSpatialIndex::ReadAndExecuteQueries("/home/serafm/dev/rtee_test_data/join_queries.csv", 2);
}