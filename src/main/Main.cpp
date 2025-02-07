#include "CreateSpatialIndex.h"

int main() {

    CreateSpatialIndex spatial_index = CreateSpatialIndex();

    CreateSpatialIndex::RTreeParams params
    {
        "/home/serafm/dev/rtree_final_datasets/T2_AREAWATER_continental_mbr_no_points.csv",
        "/home/serafm/dev/rtree_final_datasets/T2_AREAWATER_continental_mbr_no_points.csv",
        "/home/serafm/dev/rtee_test_data/knn_queries.csv",
        "/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.01%_n10000.qry",
        "/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.05%_n10000.qry",
        "/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.1%_n10000.qry",
        "/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.5%_n10000.qry",
        "/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c1%_n10000.qry",
    };

    // Bulk load
    spatial_index.StartBulkLoad(params.filepathA, "");

    // NearestN queries
    spatial_index.NearestNeighborsQuery(params.nearestQueryFilepath, 5);
    //spatial_index.NearestNeighborsQuery(params.nearestQueryFilepath, 10);
    //spatial_index.NearestNeighborsQuery(params.nearestQueryFilepath, 20);
    //spatial_index.NearestNeighborsQuery(params.nearestQueryFilepath, 30);

    // Range queries
    //spatial_index.RangeQuery(params.rangeQuery1);
    //spatial_index.RangeQuery(params.rangeQuery2);
    //spatial_index.RangeQuery(params.rangeQuery3);
    //spatial_index.RangeQuery(params.rangeQuery4);
    //spatial_index.RangeQuery(params.rangeQuery5);

    // Join query
    //spatial_index.JoinQuery();
}