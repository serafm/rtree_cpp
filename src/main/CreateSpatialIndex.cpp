#include "CreateSpatialIndex.h"

#include <algorithm>
#include <fstream>
#include <future>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../rtree/QueryBuilder.h"

void CreateSpatialIndex::Start(const RTreeParams& params) {
    m_rtreeA = BuildTree(params.filepathA);
    m_rtreeB = BuildTree(params.filepathB);
}

rtree::RTreeBuilder CreateSpatialIndex::BuildTree(const std::string& filepath) {
    std::cout << "----- rtreeBuilder Spatial Index -----" << std::endl;

    rtree::RTreeBuilder rtree;

    std::ifstream inFile(filepath);
    if (!inFile) {
        std::cerr << "Unable to open file " << filepath << std::endl;
        throw std::runtime_error("File cannot be opened");
    }

    std::cout << "\nIndexing rectangles to the tree" << std::endl;
    LoadRectanglesFromFile(filepath, rtree);

    std::cout << "Created RTree successfully" << std::endl;
    std::cout << "RTree size: " << rtree.treeSize() << std::endl;
    std::cout << "Number of nodes: " << rtree.numNodes() << std::endl;

    return rtree;
}

void CreateSpatialIndex::LoadRectanglesFromFile(const std::string& filepath, rtree::RTreeBuilder& rtree) {
    std::ifstream inFile(filepath);
    std::string mbr_line;

    while (std::getline(inFile, mbr_line)) {
        std::vector<float> mbr = ParseMBRLine(mbr_line);
        if (mbr.size() == 4) {
            rtree::Rectangle rect{mbr[0], mbr[1], mbr[2], mbr[3]};
            rtree.addEntry(rect);
        }
    }
}

std::vector<float> CreateSpatialIndex::ParseMBRLine(const std::string& line) {
    std::string modified_line = line;
    std::replace(modified_line.begin(), modified_line.end(), ',', ' ');
    std::istringstream iss(modified_line);
    std::vector<float> mbr;
    std::string num;

    while (iss >> num) {
        mbr.push_back(stof(num));
    }

    return mbr;
}

void CreateSpatialIndex::NearestNeighborsQuery(int n, const std::string& filename) {
    rtree::QueryBuilder queryBuilder(m_rtreeA);
    ReadQueryFile(filename);
    for (const auto& point : m_params) {
        if (point.size() == 2) {
            rtree::Point p{point[0], point[1]};
            queryBuilder.NearestNeighbors(p, n);
        } else {
            std::cerr << "Invalid point for Nearest neighbors query.\n";
        }
    }
}

void CreateSpatialIndex::RangeQuery(const std::string& filename) {
    rtree::QueryBuilder queryBuilder(m_rtreeA);
    ReadQueryFile(filename);
    for (const auto& rect : m_params) {
        if (rect.size() == 4) {
            rtree::Rectangle range{rect[0], rect[1], rect[2], rect[3]};
            queryBuilder.Range(range);
        } else {
            std::cerr << "Invalid rectangle for Range query.\n";
        }
    }
}

void CreateSpatialIndex::JoinQuery() {
    rtree::QueryBuilder queryBuilder(m_rtreeA, m_rtreeB);
    queryBuilder.Join();

}

void CreateSpatialIndex::ReadQueryFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    m_params.clear();
    std::string line;

    while (std::getline(file, line) && !line.empty()) {
        std::istringstream paramStream(line);
        std::vector<float> paramRow;
        float value;
        while (paramStream >> value) {
            paramRow.push_back(value);
            if (paramStream.peek() == ',') paramStream.ignore();
        }
        m_params.push_back(paramRow);
    }
    file.close();
}
