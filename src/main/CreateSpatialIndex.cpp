#include "CreateSpatialIndex.h"
#include "../rtree/builders/QueryBuilder.h"

#include <fstream>
#include <future>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <filesystem>

void CreateSpatialIndex::Start(const std::string& filepathA, const std::string& filepathB) {
    m_rtreeA = BuildTree(filepathA);
    m_rtreeB = BuildTree(filepathB);
}

void CreateSpatialIndex::Start(const std::string& filepathA) {
    m_rtreeA = BuildTree(filepathA);
}

void CreateSpatialIndex::LoadData(const std::string &filepath, std::vector<rtree::Rectangle>& rectangles) {
    std::cout << "\n ----- R-Tree Spatial Index -----" << std::endl;
    rectangles.clear();

    std::filesystem::path pathObj(filepath);
    std::cout << "\nFilename: " << pathObj.filename() << std::endl;

    std::ifstream inFile(filepath);
    if (!inFile) {
        std::cerr << "Unable to open file " << filepath << std::endl;
        throw std::runtime_error("File cannot be opened");
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "\nLoading rectangles from file..." << std::endl;
    std::string mbr_line;
    while (std::getline(inFile, mbr_line)) {
        std::istringstream iss(mbr_line);
        float x1, y1, x2, y2;

        char comma; // To ignore the commas
        if (iss >> x1 >> y1 >> comma >> x2 >> y2) {
            rectangles.emplace_back(x1, y1, x2, y2);
        } else {
            std::cerr << "Unable to parse line: " << mbr_line << std::endl;
            std::exit(0);
        }
    }

    std::cout << "Dataset loaded!" << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Time: " << duration.count() / 1e9 << " seconds\n";
}

rtree::RTreeBulkLoad CreateSpatialIndex::BuildTree(const std::string& filepath) {
    rtree::RTreeBulkLoad rtree;
    std::vector<rtree::Rectangle> rectangles;

    LoadData(filepath, rectangles);

    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "\nBuilding R-tree using STR bulk loading" << std::endl;
    rtree.bulkLoad(rectangles);

    std::cout << "Created RTree successfully" << std::endl;
    std::cout << "RTree size: " << rtree.treeSize() << std::endl;
    std::cout << "Number of nodes: " << rtree.numNodes() << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Time: " << duration.count() / 1e9 << " seconds\n";

    return rtree;
}

void CreateSpatialIndex::NearestNeighborsQuery(const std::string& filename, int n) {
    rtree::QueryBuilder queryBuilder(m_rtreeA);

    ReadKNNQueryFile(filename);

    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "\n" << n << " Nearest Neighbors Query" << std::endl;
    std::filesystem::path pathObj(filename);
    std::cout << "Filename: " << pathObj.filename() << std::endl;

    for (const auto& point : m_params) {
        if (point.size() == 2) {
            rtree::Point p{point[0], point[1]};
            queryBuilder.NearestNeighbors(p, n);
        } else {
            std::cerr << "Invalid point for Nearest neighbors query.\n";
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Time: " << duration.count() / 1e9 << " seconds\n";
}

void CreateSpatialIndex::RangeQuery(const std::string& filename) {
    rtree::QueryBuilder queryBuilder(m_rtreeA);

    ReadRangeQueryFile(filename);

    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "\nRange Query" << std::endl;
    std::filesystem::path pathObj(filename);
    std::cout << "Filename: " << pathObj.filename() << std::endl;

    for (const auto& rect : m_params) {
        if (rect.size() == 4) {
            rtree::Rectangle range{rect[0], rect[1], rect[2], rect[3]};
            queryBuilder.Range(range);
        } else {
            std::cerr << "Invalid rectangle for Range query.\n";
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Time: " << duration.count() / 1e9 << " seconds\n";
}

void CreateSpatialIndex::JoinQuery() {
    rtree::QueryBuilder queryBuilder(m_rtreeA, m_rtreeB);

    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "\nJoin Query" << std::endl;

    queryBuilder.Join();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Time: " << duration.count() / 1e9 << " seconds\n";
}

void CreateSpatialIndex::ReadRangeQueryFile(const std::string& filename) {
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

void CreateSpatialIndex::ReadKNNQueryFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    m_params.clear();
    std::string line;

    while (std::getline(file, line) && !line.empty()) {
        std::istringstream paramStream(line);
        float x1, y1, x2, y2;
        char comma;

        if (paramStream >> x1 >> y1 >> comma >> x2 >> y2) {
            float centerX = (x1 + x2) / 2.0f;
            float centerY = (y1 + y2) / 2.0f;
            m_params.push_back({centerX, centerY});
        } else {
            std::cerr << "Invalid line format: " << line << "\n";
        }
    }
    file.close();
}
