#include "CreateSpatialIndex.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "RTree.h"

namespace spatialindex {

    spatialindex::RTree rtree;

    void CreateSpatialIndex::BuildTree() {
        const std::string filepath = "/home/serafm/dev/rtee_test_data/T3_COUNTY_continental_mbr_no_points.csv";
        std::cout << "----- RTree Spatial Index -----" << std::endl;

        const std::ifstream inFile(filepath);
        if (!inFile) {
            std::cerr << "Unable to open file " << filepath << std::endl;
            throw std::runtime_error("File cannot be opened");
        }

        std::cout << "\nIndexing rectangles to the tree" << std::endl;
        loadRectanglesFromFile(filepath, rtree);

        std::cout << "Created RTree successfully" << std::endl;
        std::cout << "RTree size: " << rtree.treeSize() << std::endl;
        std::cout << "Number of nodes: " << rtree.numNodes() << std::endl;
    }

    void CreateSpatialIndex::Query(int type, const std::vector<std::vector<float>>& params) {
        switch (type) {
            case 0: { // Nearest
                for (const auto& point : params) {
                    if (point.size() == 2) {
                        Point p{point[0], point[1]};
                        rtree.nearest(p, 10);
                        rtree.nearestN(p, 5, 10);
                    } else {
                        std::cerr << "Invalid point for Nearest query.\n";
                    }
                }
                break;
            }
            case 1: { // Contains
                for (const auto& rect : params) {
                    if (rect.size() == 4) {
                        Rectangle r{rect[0], rect[1], rect[2], rect[3]};
                        rtree.contains(r);
                    } else {
                        std::cerr << "Invalid rectangle for Contains query.\n";
                    }
                }
                break;
            }
            case 2: { // Intersects
                for (const auto& rect : params) {
                    if (rect.size() == 4) {
                        Rectangle r{rect[0], rect[1], rect[2], rect[3]};
                        rtree.intersects(r);
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
            double value;
            while (paramStream >> value) {
                paramRow.push_back(value);
                if (paramStream.peek() == ',') paramStream.ignore();
            }
            params.push_back(paramRow);
        }
        file.close();

        Query(type, params);
    }


    void CreateSpatialIndex::loadRectanglesFromFile(const std::string& filepath, spatialindex::RTree & rtree) {
        std::ifstream inFile(filepath);
        std::string mbr_line;
        int id = 1;

        while (std::getline(inFile, mbr_line)) {
            std::vector<float> mbr = parseMBRLine(mbr_line);
            if (mbr.size() == 4) {
                spatialindex::Rectangle rect{mbr[0], mbr[1], mbr[2], mbr[3]};
                rtree.add(rect, id);
                id+=1;
            }
        }
    }

    std::vector<float> CreateSpatialIndex::parseMBRLine(const std::string& line) {
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
}
