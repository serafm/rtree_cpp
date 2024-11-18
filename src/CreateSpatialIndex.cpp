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

    void CreateSpatialIndex::Start() {
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

    void CreateSpatialIndex::Query(int type) {

        switch (type) {
            case 0: { // Nearest
                Point p{-91.0804, 30.5159};
                rtree.nearest(p, 10);
                rtree.nearestN(p, 5, 10);
                break;
            }
            case 1: { // Contains
                Rectangle rect{-100,50,100, -50};
                rtree.contains(rect);
                break;
            }
            case 2: { // Intersects
                auto rect = Rectangle{-100,50,100, -50};
                rtree.intersects(rect);
                break;
            }
            default: {
                std::cerr << "\nInvalid query type. Please try again these are the options:" << std::endl;
                std::cerr << "Nearest: 0 \nContains: 1 \nIntersects: 2" << std::endl;
                break;
            }
        }
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
