#include <chrono>
#include <iostream>


#include "../rtree/RTreeBuilder.h"
#include "CreateSpatialIndex.h"
using namespace rtree;

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    CreateSpatialIndex::BuildTree("/home/serafm/dev/rtee_test_data/T3_COUNTY_continental_mbr_no_points.csv");

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Execution time: " << duration.count() / 1e6 << " milliseconds\n";
    std::cout << "Execution time: " << duration.count() / 1e9 << " seconds\n";

    CreateSpatialIndex::ReadAndExecuteQueries("/home/serafm/dev/rtee_test_data/knn_queries.csv", 0);
    CreateSpatialIndex::ReadAndExecuteQueries("/home/serafm/dev/rtee_test_data/range_queries.csv", 1);
    //CreateSpatialIndex::ReadAndExecuteQueries("/home/serafm/dev/rtee_test_data/join_queries.csv", 2);
}