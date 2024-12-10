#pragma once

#ifndef CREATESPATIALINDEX_H
#define CREATESPATIALINDEX_H

#include <string>
#include <vector>
#include "../rtree/RTreeBuilder.h"

class CreateSpatialIndex {

    // Instance members
    std::vector<std::vector<float>> m_params;
    rtree::RTreeBuilder m_rtreeA;
    rtree::RTreeBuilder m_rtreeB;

    /**
     * @brief Loads rectangle entries from the given file into the specified R-tree.
     *
     * This function reads each line of the input file, parses it as a minimum bounding rectangle (MBR),
     * and inserts the rectangle into the R-tree. Only lines with exactly four floating-point values
     * (minX, minY, maxX, maxY) are considered valid entries.
     *
     * @param filepath The path to the file containing rectangle coordinates.
     * @param rtree A reference to the RTreeBuilder object into which the rectangles will be inserted.
     */
    void LoadRectanglesFromFile(const std::string& filepath, rtree::RTreeBuilder& rtree);

    /**
     * @brief Parses a single line of text into a vector of floating-point values representing an MBR.
     *
     * This function replaces commas with spaces, then tokenizes the line into floating-point numbers.
     * If the returned vector contains exactly four values, it represents a valid rectangle (minX, minY, maxX, maxY).
     *
     * @param line A string containing comma- or space-separated values representing rectangle coordinates.
     * @return A vector of floats extracted from the input line.
     */
    std::vector<float> ParseMBRLine(const std::string& line);

    /**
     * @brief Reads query parameters (points or rectangles) from a file into an internal structure.
     *
     * This function opens the specified file, parses each line into a vector of floating-point values,
     * and stores them for later use in queries. Each non-empty line is expected to contain between two
     * and four floating-point values, depending on whether the query is for points or rectangles.
     *
     * @param filename The path to the file containing query parameters.
     */
    void ReadQueryFile(const std::string& filename);

    /**
     * @brief Constructs an R-tree from the rectangles found in the provided file.
     *
     * This function reads spatial data (rectangles) from the given file, inserts them into an R-tree,
     * and returns the fully constructed R-tree builder object. It prints diagnostics such as tree size
     * and number of nodes to the console.
     *
     * @param filepath The path to the file containing rectangle coordinates.
     * @return A fully constructed RTreeBuilder instance.
     * @throws std::runtime_error If the file cannot be opened.
     */
    rtree::RTreeBuilder BuildTree(const std::string& filepath);

public:
    CreateSpatialIndex() = default;

    struct RTreeParams {
        std::string filepathA;
        std::string filepathB;
        std::string nearestQueryFilepath;
        std::string rangeQueryFilepath;
    };

    /**
     * @brief Initializes and builds two R-trees from the specified input files.
     *
     * Using the provided parameters, this method reads the input files and constructs two
     * corresponding R-trees that can be used for spatial queries such as nearest neighbor,
     * range, and join queries.
     *
     * @param params A structure containing the filepaths and other parameters used to build the R-trees.
     */
    void Start(const std::string& filepathA, const std::string& filepathB);
    void Start(const std::string& filepath);

    void NearestNeighborsQuery(const std::string& filename, int n);
    void RangeQuery(const std::string& filename);
    void JoinQuery();
};

#endif // CREATESPATIALINDEX_H
