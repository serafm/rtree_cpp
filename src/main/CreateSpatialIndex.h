#pragma once

#ifndef CREATESPATIALINDEX_H
#define CREATESPATIALINDEX_H

#include <string>
#include <vector>
#include "../rtree/builders/RTreeBulkLoad.h"
#include "../rtree/structures/Rectangle.h"

class CreateSpatialIndex {

    // Instance members
    std::vector<std::vector<float>> m_params;
    rtree::RTreeBulkLoad m_rtreeA;
    rtree::RTreeBulkLoad m_rtreeB;

    /**
     * @brief Parses a single line of text into a vector of floating-point values representing an MBR.
     *
     * This function replaces commas with spaces, then tokenizes the line into floating-point numbers.
     * If the returned vector contains exactly four values, it represents a valid rectangle (minX, minY, maxX, maxY).
     *
     * @param line A string containing comma- or space-separated values representing rectangle coordinates.
     * @return A vector of floats extracted from the input line.
     */
    static std::vector<float> ParseMBRLine(const std::string& line);

    /**
     * @brief Reads query parameters (points or rectangles) from a file into an internal structure.
     *
     * This function opens the specified file, parses each line into a vector of floating-point values,
     * and stores them for later use in queries. Each non-empty line is expected to contain between two
     * and four floating-point values, depending on whether the query is for points or rectangles.
     *
     * @param filename The path to the file containing query parameters.
     */
    void ReadRangeQueryFile(const std::string& filename);

    /**
     * @brief Reads K-nearest neighbors (KNN) query parameters from a file into an internal structure.
     *
     * This function opens the specified file, parses each line into a pair of floating-point values
     * representing the center of a query region, and stores them for use in KNN queries. Each line is
     * expected to contain four floating-point values separated by a comma, defining the bounding box of
     * a query region. The function computes the center of this region and stores it as the query point.
     *
     * @param filename The path to the file containing KNN query parameters.
     */
    void ReadKNNQueryFile(const std::string& filename);

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
    rtree::RTreeBulkLoad BuildTree(const std::string& filepath);

    /**
     * @brief Loads spatial data from a file into a vector of rectangles.
     *
     * This function reads a file containing rectangle coordinates and populates a vector with
     * `rtree::Rectangle` objects. Each line in the file is expected to contain four floating-point
     * values separated by a comma, representing the coordinates (x1, y1, x2, y2) of a rectangle.
     * If a line cannot be parsed correctly, an error message is displayed, and the program exits.
     *
     * @param filepath The path to the file containing rectangle data.
     * @param rectangles A reference to a vector that will be populated with the loaded rectangles.
     * @throws std::runtime_error If the file cannot be opened.
     */
    void LoadData(const std::string &filepath, std::vector<rtree::Rectangle>& rectangles);

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
     * @param filepathA A filepath for a tree dataset (coordinates)
     * @param filepathB A filepath for a tree dataset (coordinates)
     */
    void Start(const std::string& filepathA, const std::string& filepathB);
    void Start(const std::string& filepath);

    void NearestNeighborsQuery(const std::string& filename, int n);
    void RangeQuery(const std::string& filename);
    void JoinQuery();
};

#endif // CREATESPATIALINDEX_H
