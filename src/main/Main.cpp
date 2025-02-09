#include <chrono>
#include <iostream>

#include "CreateSpatialIndex.h"

int main() {
    CreateSpatialIndex spatial_index = CreateSpatialIndex();

    CreateSpatialIndex::RTreeParams params
    {
        "/home/serafm/dev/rtree_final_datasets/T5_LINEARWATER_continental_mbr_no_points.csv",
        "/home/serafm/dev/rtree_final_datasets/T8_ROADS_continental_mbr_no_points.csv",
        "/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c1%_n10000.qry",
        "/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c1%_n10000.qry",
    };

    // Build 1 Rtree
    spatial_index.Start(params.filepathA);

    // Build 2 Rtrees
    //spatial_index.Start(params.filepathA, params.filepathB);

    // Execute NearestN query
    spatial_index.NearestNeighborsQuery(params.nearestQueryFilepath, 5);
    //spatial_index.NearestNeighborsQuery(params.nearestQueryFilepath, 10);
    //spatial_index.NearestNeighborsQuery(params.nearestQueryFilepath, 20);
    //spatial_index.NearestNeighborsQuery(params.nearestQueryFilepath, 30);

    // Execute Range query
    //spatial_index.RangeQuery(params.rangeQueryFilepath);
    //spatial_index.RangeQuery("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.5%_n10000.qry");
    //spatial_index.RangeQuery("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.1%_n10000.qry");
    //spatial_index.RangeQuery("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.05%_n10000.qry");
    //spatial_index.RangeQuery("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.01%_n10000.qry");

    // Execute Join query
    //spatial_index.JoinQuery();
}