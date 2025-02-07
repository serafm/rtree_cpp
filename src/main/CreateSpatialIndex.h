#pragma once

#ifndef CREATESPATIALINDEX_H
#define CREATESPATIALINDEX_H

#include <string>
#include <vector>
#include "../rtree/RTreeBulkLoadBuilder.h"
#include "../rtree/RTreeBuilder.h"
#include "../rtree/query/QueryBuilder.h"

class CreateSpatialIndex {

    using RTreePtr = rtree::RTreeBulkLoadBuilder;
    std::vector<std::vector<float>> m_params;
    RTreePtr m_rtreeA;
    RTreePtr m_rtreeB;
    //rtree::RTreeBuilder m_rtreeA;
    //rtree::RTreeBuilder m_rtreeB;
    std::vector<rtree::Rectangle> m_rectangles;

    /**
     * @brief Reads knn query parameters (rectangles) from a file into an internal structure.
     *
     * This function opens the specified file, parses each line into a vector of floating-point values,
     * and stores them for later use in queries. Each non-empty line is expected to contain between two
     * and four floating-point values, depending on whether the query is for points or rectangles.
     *
     * @param filename The path to the file containing query parameters.
     */
    void ReadKNNQueryFile(const std::string& filename);

    /**
     * @brief Reads range query parameters (rectangles) from a file into an internal structure.
     *
     * This function opens the specified file, parses each line into a vector of floating-point values,
     * and stores them for later use in queries. Each non-empty line is expected to contain between two
     * and four floating-point values, depending on whether the query is for points or rectangles.
     *
     * @param filename The path to the file containing query parameters.
     */
    void ReadRangeQueryFile(const std::string& filename);

    /**
     * @brief Constructs an R-tree from the rectangles found in the provided file.
     *
     * This function reads spatial data (rectangles) from memory,
     * and returns the fully constructed R-tree builder object using the STR bulk load approach.
     * It prints tree size and number of nodes to the console.
     *
     * @return A fully constructed RTreeBulkLoadBuilder instance.
     * @throws std::runtime_error If the file cannot be opened.
     */
    rtree::RTreeBulkLoadBuilder BulkLoadTree();

    /**
     * @brief Constructs an R-tree from the rectangles found in the provided file.
     *
     * This function reads spatial data (rectangles) from the given file, inserts them into an R-tree,
     * and returns the fully constructed R-tree builder object.
     * It prints diagnostics such as tree size and number of nodes to the console.
     *
     * @return A fully constructed RTreeBuilder instance.
     * @throws std::runtime_error If the file cannot be opened.
     */
    rtree::RTreeBuilder BuildTree(const std::string& filepath);

public:
    explicit CreateSpatialIndex() = default;

    struct RTreeParams {
        std::string filepathA;
        std::string filepathB;
        std::string nearestQueryFilepath;
        std::string rangeQuery1;
        std::string rangeQuery2;
        std::string rangeQuery3;
        std::string rangeQuery4;
        std::string rangeQuery5;
    };

    /**
     * @brief Initializes and builds two R-trees from the specified input files.
     *
     * Using the provided parameters, this method reads the input files and constructs two
     * corresponding R-trees that can be used for spatial queries such as nearest neighbor,
     * range, and join queries.
     *
     * @param filepathA A filepath for a tree dataset (coordinates)
     * @param filepathB A filepath for a tree dataset (coordinates)
     */
    void StartBulkLoad(const std::string& filepathA, const std::string& filepathB);
    void StartBulkLoad(const std::string &filepath, RTreePtr& rtree);

    void Start(const std::string& filepathA, const std::string& filepathB);
    void Start(const std::string &filepath);

    void LoadData(const std::string &filepath);

    void NearestNeighborsQuery(const std::string& filename, int n);
    void RangeQuery(const std::string& filename);
    void JoinQuery();
};

#endif // CREATESPATIALINDEX_H
