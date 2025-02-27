#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <vector>

#include "../src/rtree/builders/RTreeBulkLoad.h"

enum QueryType {
    RANGE = 1,
    NEAREST,
    JOIN
};

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
    std::cout << "\n----- R-Tree Spatial Index -----" << std::endl;
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

    std::filesystem::path pathObj(filename);
    std::cout << "Filename: " << pathObj.filename() << std::endl;

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

    std::filesystem::path pathObj(filename);
    std::cout << "Filename: " << pathObj.filename() << std::endl;

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

int main(int argc, char* argv[]) {
    Timer time;
    double buildTime = 0;
    double queryTime = 0;

    std::string queryFile;
    std::string tree_path_a;
    std::string tree_path_b;
    int queryType = -1;
    int k = -1;

    // Command-line argument parsing
    char c;
    while ((c = getopt(argc, argv, "rnjk:")) != -1) {
        switch (c) {
            case 'r':
                queryType = RANGE;
                break;
            case 'n':
                queryType = NEAREST;
                break;
            case 'k':
                k = atoi(optarg);
                break;
            case 'j':
                queryType = JOIN;
                break;
            default:
                std::cerr << "Invalid arguments!\n";
                return 1;
        }
    }

    // Collect file paths
    std::vector<std::string> filepaths;
    for (int i = optind; i < argc; i++) {
        filepaths.push_back(argv[i]);
    }

    // Ensure valid file input
    if (filepaths.size() < 2) {
        std::cerr << "Error: Two file paths are required.\n";
        return 1;
    }

    tree_path_a = filepaths[0];

    if (queryType == RANGE || queryType == NEAREST) {
        queryFile = filepaths[1];
    } else {
        tree_path_b = filepaths[1];
    }

    // Load first R-tree
    loadData(tree_path_a);
    rtree::RTreeBulkLoad rtreeA(64);

    time.start();
    rtreeA.bulkLoad(m_rectangles);
    buildTime = time.stop();
    std::cout << "Build Time: " << buildTime << " sec" << std::endl;

    // Handle queries
    if (queryType == RANGE) {
        readRangeQueries(queryFile);
        for (int i = 0; i < rangeQueries.size(); i++) {
            time.start();
            rtreeA.range(rangeQueries[i]);
            queryTime += time.stop();
            //break;
        }
        std::cout << "Range Query Time: " << queryTime << " sec" << std::endl;
        std::cout << "Average Query Time: " << queryTime / (double) rangeQueries.size() << " sec" << std::endl;
    }
    else if (queryType == NEAREST) {
        if (k <= 0) {
            std::cerr << "Error: Invalid value for k\n";
            return 1;
        }
        readNearestQueries(queryFile);
        for (const auto& query : nearestQueries) {
            time.start();
            rtreeA.nearestN(query, k);
            queryTime += time.stop();
            //break;
        }
        std::cout << "Nearest Query Time: " << queryTime << " sec" << std::endl;
        std::cout << "Average Query Time: " << queryTime / (double) nearestQueries.size() << " sec" << std::endl;
    }
    else if (queryType == JOIN) {
        loadData(tree_path_b);
        rtree::RTreeBulkLoad rtreeB(64);

        time.start();
        rtreeB.bulkLoad(m_rectangles);
        buildTime = time.stop();
        std::cout << "Second RTree Build Time: " << buildTime << " sec" << std::endl;

        time.start();
        rtreeA.join(rtreeB);
        queryTime = time.stop();
        std::cout << "Join Query Time: " << queryTime << " sec" << std::endl;
    }
    else {
        std::cerr << "Invalid or missing query type.\n";
        return 1;
    }

    return 0;
}