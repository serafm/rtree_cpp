#include "CreateSpatialIndex.h"
using namespace spatialindex;

int main() {
    spatialindex::CreateSpatialIndex::Start();
    spatialindex::CreateSpatialIndex::ReadAndExecuteQueries("/home/serafm/dev/rtee_test_data/queries.csv");
}