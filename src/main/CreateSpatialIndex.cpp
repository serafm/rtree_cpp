#include "CreateSpatialIndex.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../QueryBuilder.h"


rtree::RTreeBuilder rtreeBuilder;

void CreateSpatialIndex::BuildTree(const std::string& filepath) {
    std::cout << "----- rtreeBuilder Spatial Index -----" << std::endl;

    const std::ifstream inFile(filepath);
    if (!inFile) {
        std::cerr << "Unable to open file " << filepath << std::endl;
        throw std::runtime_error("File cannot be opened");
    }

    std::cout << "\nIndexing rectangles to the tree" << std::endl;
    LoadRectanglesFromFile(filepath, rtreeBuilder);

    std::cout << "Created RTree successfully" << std::endl;
    std::cout << "RTree size: " << rtreeBuilder.treeSize() << std::endl;
    std::cout << "Number of nodes: " << rtreeBuilder.numNodes() << std::endl;
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

void CreateSpatialIndex::ReadAndExecuteQueries(const std::string& filename, int type) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    std::vector<std::vector<float>> params;
    std::string line;

    while (std::getline(file, line) && !line.empty()) {
        std::istringstream paramStream(line);
        std::vector<float> paramRow;
        float value;
        while (paramStream >> value) {
            paramRow.push_back(value);
            if (paramStream.peek() == ',') paramStream.ignore();
        }
        params.push_back(paramRow);
    }
    file.close();

    Query(type, params);
}

void CreateSpatialIndex::Query(int type, const std::vector<std::vector<float>>& params) {
    rtree::QueryBuilder queryBuilder(rtreeBuilder);

    switch (type) {
        case 0: { // Nearest
            for (const auto& point : params) {
                if (point.size() == 2) {
                    rtree::Point p{point[0], point[1]};
                    queryBuilder.GetNearestNeighbors(p, 5);
                } else {
                    std::cerr << "Invalid point for Nearest query.\n";
                }
            }
            break;
        }
        case 1: { // Contains
            for (const auto& rect : params) {
                if (rect.size() == 4) {
                    rtree::Rectangle range{rect[0], rect[1], rect[2], rect[3]};
                    queryBuilder.GetRange(range);
                } else {
                    std::cerr << "Invalid rectangle for Contains query.\n";
                }
            }
            break;
        }
        case 2: { // Intersects
            for (const auto& rect : params) {
                if (rect.size() == 4) {
                    rtree::Rectangle r{rect[0], rect[1], rect[2], rect[3]};
                    //rtree.intersects(r);
                } else {
                    std::cerr << "Invalid rectangle for Intersects query.\n";
                }
            }
            break;
        }
        default: {
            std::cerr << "\nInvalid query type. Please try again these are the options:" << std::endl;
            std::cerr << "Nearest: 0 \nContains: 1 \nIntersects: 2" << std::endl;
            break;
        }
    }
}
