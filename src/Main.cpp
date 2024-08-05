#include <algorithm>
#include <fstream>
#include <iostream>
#include "RTree.h"
#include "sstream"

using namespace rtree;
using namespace std;

int main() {

    string filepath = "/home/serafm/Documents/rtree-cpp/data/T3_COUNTY_continental_mbr_no_points_small.csv";
    std::cout << "----- Create RTree Spatial Index -----" << endl;
    //std::cout << "Enter file path of MBRs: " << std::endl;
    //std::cin >> filepath;

    ifstream inFile(filepath);
    if (!inFile) {
        cerr << "Unable to open file " << filepath << endl;
        throw runtime_error("File cannot be opened");
    }

    // Initialize RTree Spatial Index
    RTree rtree;

    string mbr_line;
    uint32_t id = 2;
    // Read the file and add the rectangles into the tree
    std::cout << "----- Reading and adding MBRs to RTree -----" << std::endl;
    while (getline(inFile, mbr_line)) {
        replace(mbr_line.begin(), mbr_line.end(), ',', ' ');
        std::istringstream iss(mbr_line);
        string num;
        std::vector<float> mbr;

        while (iss >> num) {
            mbr.push_back(stof(num));
        }
        Rectangle rect{mbr[0], mbr[1], mbr[2], mbr[3]};
        rtree.add(rect, id);
        id+=1;
        //std::cout << rect.minX << ", " << rect.minY << ", " << rect.maxX << ", " << rect.maxY << std::endl;
    }

    std::cout << "Created RTree succesfully" << std::endl;
    std::cout << "RTree size: " << rtree.treeSize() << std::endl;
    std::cout << "Number of nodes: " << rtree.numNodes() << std::endl;
    //rtree.getNode(1).getNodeEntries();
    //Rectangle bounds = rtree.getBounds();
    //std::cout << "Bounds: " <<  std::endl;
    //std::cout << bounds.minX << ", " << bounds.minY << ", " << bounds.maxX << ", " << bounds.maxY << std::endl;
    //rtree.nearest(Point{-86.3077, 38.6887} ,1);

    return 0;
}
