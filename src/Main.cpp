#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "rtree/builders/RTreeBulkLoad.h"
#include "rtree/structures/Rectangle.h"

std::vector<rtree::Rectangle> m_rectangles;
std::vector<rtree::Rectangle> rangeQueries;
std::vector<rtree::Point> nearestQueries;

struct Timer{
private:
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point start_time, stop_time;

public:
    Timer()
    {
        start();
    }

    void start()
    {
        start_time = Clock::now();
    }

    double getElapsedTimeInSeconds()
    {
        return std::chrono::duration<double>(stop_time - start_time).count();
    }

    double stop()
    {
        stop_time = Clock::now();
        return getElapsedTimeInSeconds();
    }
};

void loadData(const std::string &filepath) {
    std::cout << "----- R-Tree Spatial Index -----" << std::endl;
    m_rectangles.clear();

    std::filesystem::path pathObj(filepath);
    std::cout << "Filename: " << pathObj.filename() << std::endl;

    std::ifstream inFile(filepath);
    if (!inFile) {
        std::cerr << "Unable to open file " << filepath << std::endl;
        throw std::runtime_error("File cannot be opened");
    }

    std::string mbr_line;
    int id = 1;
    while (std::getline(inFile, mbr_line)) {
        std::istringstream iss(mbr_line);
        float x1, y1, x2, y2;

        char comma; // To ignore the commas
        if (iss >> x1 >> y1 >> comma >> x2 >> y2) {
            m_rectangles.emplace_back(x1, y1, x2, y2, id);
        } else {
            std::cerr << "Unable to parse line: " << mbr_line << std::endl;
            std::exit(0);
        }
        id++;
    }
}

void readRangeQueries(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    rangeQueries.clear();
    std::string line;

    while (std::getline(file, line) && !line.empty()) {
        std::istringstream paramStream(line);
        std::vector<float> paramRow;
        float value;
        while (paramStream >> value) {
            paramRow.push_back(value);
            if (paramStream.peek() == ',') paramStream.ignore();
        }
        rangeQueries.push_back(rtree::Rectangle{paramRow[0], paramRow[1], paramRow[2], paramRow[3]});
    }
    file.close();
}

void readNearestQueries(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    nearestQueries.clear();
    std::string line;

    while (std::getline(file, line) && !line.empty()) {
        std::istringstream paramStream(line);
        float x1, y1, x2, y2;
        char comma;

        if (paramStream >> x1 >> y1 >> comma >> x2 >> y2) {
            float centerX = (x1 + x2) / 2.0f;
            float centerY = (y1 + y2) / 2.0f;
            nearestQueries.push_back({centerX, centerY});
        } else {
            std::cerr << "Invalid line format: " << line << "\n";
        }
    }
    file.close();
}

int main() {
    Timer time;
    double buildTime = 0;
    double queryTime = 0;

    std::string filepathA = "/home/serafm/dev/rtree_final_datasets/T2_AREAWATER_continental_mbr_no_points.csv";
    //std::string filepathA = "/home/serafm/dev/rtree_final_datasets/T4_EDGES_continental_mbr_no_points.csv";
    //std::string filepathA = "/home/serafm/dev/rtree_final_datasets/T5_LINEARWATER_continental_mbr_no_points.csv";
    std::string filepathB = "/home/serafm/dev/rtree_final_datasets/T8_ROADS_continental_mbr_no_points.csv";

    loadData(filepathA);
    rtree::RTreeBulkLoad rtree(16);

    time.start();
    rtree.bulkLoad(m_rectangles);
    buildTime = time.stop();
    std::cout << "Rtree size: " << rtree.treeSize() << std::endl;
    std::cout << "Total nodes: " << rtree.numNodes() << std::endl;
    std::cout << "RTree height: " << rtree.treeHeight << std::endl;

    std::cout << "Build Time: " << buildTime << std::endl;

    // Range queries
    /*
    readRangeQueries("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c1%_n10000.qry");
    //readRangeQueries("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.5%_n10000.qry");
    //readRangeQueries("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.1%_n10000.qry");
    //readRangeQueries("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.05%_n10000.qry");
    //readRangeQueries("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.01%_n10000.qry");

    for (int i = 0; i < rangeQueries.size(); i++) {
        time.start();
        rtree.range(rangeQueries[i]);
        queryTime += time.stop();
    }

    std::cout << "Range Query Time: " << queryTime << std::endl;
    */

    // NearestN queries
    readNearestQueries("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c0.01%_n10000.qry");

    for (int i = 0; i < nearestQueries.size(); i++) {
        time.start();
        rtree.nearestN(nearestQueries[i], 5);
        break;
        queryTime += time.stop();
    }

    std::cout << "Nearest Query Time: " << queryTime << std::endl;

    //spatial_index.NearestNeighborsQuery("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c1%_n10000.qry", 5);
    //spatial_index.NearestNeighborsQuery("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c1%_n10000.qry", 10);
    //spatial_index.NearestNeighborsQuery("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c1%_n10000.qry", 20);
    //spatial_index.NearestNeighborsQuery("/home/serafm/dev/rtree_final_datasets/USA_queries/USA_c1%_n10000.qry", 30);


}
