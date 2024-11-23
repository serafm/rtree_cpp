#include "CreateSpatialIndex.h"
using namespace spatialindex;

int main() {
    CreateSpatialIndex::BuildTree("/home/serafm/dev/rtee_test_data/T4_EDGES_continental_mbr_no_points.csv");
    CreateSpatialIndex::ReadAndExecuteQueries("/home/serafm/dev/rtee_test_data/knn_queries.csv", 0);
    CreateSpatialIndex::ReadAndExecuteQueries("/home/serafm/dev/rtee_test_data/range_queries.csv", 1);
    //CreateSpatialIndex::ReadAndExecuteQueries("/home/serafm/dev/rtee_test_data/join_queries.csv", 2);
}