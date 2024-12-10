#include <chrono>
#include <iostream>

#include "CreateSpatialIndex.h"

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    CreateSpatialIndex spatial_index = CreateSpatialIndex();

    CreateSpatialIndex::RTreeParams params
    {
        "/home/serafm/dev/rtee_test_data/T1_AREALM_continental_mbr_no_points.csv",
        "/home/serafm/dev/rtee_test_data/T3_COUNTY_continental_mbr_no_points.csv",
        "/home/serafm/dev/rtee_test_data/knn_queries.csv",
        "/home/serafm/dev/rtee_test_data/range_queries.csv",
    };

    // Build 1 Rtree
    //spatial_index.Start(params.filepathA);

    // Build 2 Rtrees
    spatial_index.Start(params.filepathA, params.filepathB);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "RTree build time taken: " << duration.count() / 1e6 << " milliseconds\n";
    std::cout << "RTree build time taken: " << duration.count() / 1e9 << " seconds\n";


    start = std::chrono::high_resolution_clock::now();
    // Execute NearestN query
    spatial_index.NearestNeighborsQuery(params.nearestQueryFilepath, 5);

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Nearest query time taken: " << duration.count() / 1e6 << " milliseconds\n";

    start = std::chrono::high_resolution_clock::now();
    // Execute Range query
    spatial_index.RangeQuery(params.rangeQueryFilepath);

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Range query time taken: " << duration.count() / 1e6 << " milliseconds\n";

    start = std::chrono::high_resolution_clock::now();
    // Execute Join query
    spatial_index.JoinQuery();

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Join query time taken: " << duration.count() / 1e6 << " milliseconds\n";
    std::cout << "Join query time taken: " << duration.count() / 1e9 << " seconds\n";

}