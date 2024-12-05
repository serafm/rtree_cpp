#include <chrono>
#include <iostream>

#include "CreateSpatialIndex.h"

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    CreateSpatialIndex spatial_index = CreateSpatialIndex();

    CreateSpatialIndex::RTreeParams params
    {
        "/home/serafm/dev/rtee_test_data/T3_COUNTY_continental_mbr_no_points.csv",
        "/home/serafm/dev/rtee_test_data/T1_AREALM_continental_mbr_no_points.csv",
        "/home/serafm/dev/rtee_test_data/knn_queries.csv",
        "/home/serafm/dev/rtee_test_data/range_queries.csv",
    };

    spatial_index.Start(params);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Execution time: " << duration.count() / 1e6 << " milliseconds\n";
    std::cout << "Execution time: " << duration.count() / 1e9 << " seconds\n";


    spatial_index.NearestNeighborsQuery(5, params.nearestQueryFilepath);
    spatial_index.RangeQuery(params.rangeQueryFilepath);
    spatial_index.JoinQuery();
}