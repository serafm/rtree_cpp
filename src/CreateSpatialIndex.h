#ifndef CREATESPATIALINDEX_H
#define CREATESPATIALINDEX_H
#include <string>
#include <vector>

#include "RTree.h"

namespace spatialindex {
    class CreateSpatialIndex {

    private:

        // Function to load rectangles from a file into the RTree
        static void loadRectanglesFromFile(const std::string& filepath, spatialindex::RTree & rtree);

        // Function to parse a line of MBR data and return a vector of floats
        static std::vector<float> parseMBRLine(const std::string& line);

    public:
        static void Start();
        static void Query(int type);
    };
}


#endif //CREATESPATIALINDEX_H
