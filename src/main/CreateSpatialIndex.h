#ifndef CREATESPATIALINDEX_H
#define CREATESPATIALINDEX_H
#include <string>
#include <vector>

#include "../rtree/RTreeBuilder.h"


class CreateSpatialIndex {

// Function to load rectangles from a file into the RTree
static void LoadRectanglesFromFile(const std::string& filepath, rtree::RTreeBuilder& rtree);

// Function to parse a line of MBR data and return a vector of floats
static std::vector<float> ParseMBRLine(const std::string& line);

static void Query(int type, const std::vector<std::vector<float>>& params);

public:
    static void BuildTree(const std::string& filepath);
    static void ReadAndExecuteQueries(const std::string& filename, int type);
};

#endif //CREATESPATIALINDEX_H
