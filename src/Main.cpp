#include "RTree.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm> // For std::replace

// Assume these classes and functions are defined
// class RTree { /* RTree class definition */ };
// class Rectangle { /* Rectangle class definition */ };

// Function prototypes
void loadRectanglesFromFile(const std::string& filepath, spatialindex::RTree & rtree);
std::vector<float> parseMBRLine(const std::string& line);

int main() {
    const std::string filepath = "/home/serafm/dev/rtee_test_data/T3_COUNTY_continental_mbr_no_points.csv";
    std::cout << "----- RTree Spatial Index -----" << std::endl;

    const std::ifstream inFile(filepath);
    if (!inFile) {
        std::cerr << "Unable to open file " << filepath << std::endl;
        throw std::runtime_error("File cannot be opened");
    }

    spatialindex::RTree rtree;

    std::cout << "----- Indexing rectangles to the tree -----" << std::endl;
    loadRectanglesFromFile(filepath, rtree);

    std::cout << "Created RTree successfully" << std::endl;
    std::cout << "RTree size: " << rtree.treeSize() << std::endl;
    std::cout << "Number of nodes: " << rtree.numNodes() << std::endl;

    spatialindex::Point p{-91.0804, 30.5159};
    rtree.nearest(p, 10);
    rtree.nearestN(p, 5, 10);

    return 0;
}

// Function to load rectangles from a file into the RTree
void loadRectanglesFromFile(const std::string& filepath, spatialindex::RTree & rtree) {
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

// Function to parse a line of MBR data and return a vector of floats
std::vector<float> parseMBRLine(const std::string& line) {
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
