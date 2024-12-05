#ifndef CREATESPATIALINDEX_H
#define CREATESPATIALINDEX_H

#include <string>
#include <vector>
#include "../rtree/RTreeBuilder.h"

class CreateSpatialIndex {
private:
    // Instance members
    std::vector<std::vector<float>> m_params;
    rtree::RTreeBuilder m_rtreeA;
    rtree::RTreeBuilder m_rtreeB;

    // Instance methods
    void LoadRectanglesFromFile(const std::string& filepath, rtree::RTreeBuilder& rtree);
    std::vector<float> ParseMBRLine(const std::string& line);
    void ReadQueryFile(const std::string& filename);
    rtree::RTreeBuilder BuildTree(const std::string& filepath);

public:
    struct RTreeParams {
        std::string filepathA;
        std::string filepathB;
        std::string nearestQueryFilepath;
        std::string rangeQueryFilepath;
    };

    // Public methods
    void Start(const RTreeParams& params);
    void NearestNeighborsQuery(int n, const std::string& filename);
    void RangeQuery(const std::string& filename);
    void JoinQuery();
    CreateSpatialIndex() = default;
};

#endif // CREATESPATIALINDEX_H
