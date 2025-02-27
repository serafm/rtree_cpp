# C++ Spatial Index (RTree)

The management and indexing of geospatial data is a crucial research topic for computer science.
One of the most important and well-known data structures
for storing and searching geospatial data is R-tree.
This thesis implements an efficient and effective
version of an R-tree using C++ programming language.
The R-tree structure allows efficient search and processing of multidimensional
data based on the MBR (Minimum Bounding Rectangle) overlay.
The main goal of the thesis is to index the data using the R-tree data structure.
Subsequently, indexing is used for efficient information retrieval in geospatial
queries such as join, range and nearest neighbor queries.
The creation of the new index aims to improve the index creation and query performance.
Finally, experiments are presented to demonstrate practically the improvement and
performance of an R-tree.

# Building and Running the R-tree C++ Project

## Prerequisites
Make sure you have the following dependencies installed on your system:

- **CMake** (version 3.x or higher)
- **GCC/Clang** (with C++17 support)
- **Make** (or Ninja if preferred)

## Building the Project
Follow these steps to compile the project:

1. Open a terminal and navigate to the root directory of the project:
   ```sh
   cd /path/to/rtree_cpp
   ```
2. Create a `build` directory and navigate into it:
   ```sh
   mkdir -p build && cd build
   ```
3. Run CMake to configure the project:
   ```sh
   cmake ..
   ```
4. Compile the project:
   ```sh
   make
   ```
   This will generate the executable `rtree_cpp` inside the `build` directory.

## Running the Executable
The compiled executable `rtree_cpp` supports multiple query types. Below are examples of how to run different queries.

### 1. Range Query
To perform a range query, run the following command:
```sh
./rtree_cpp -r "filepath_of_dataset" "filepath_of_query_dataset"
```
Example:
```sh
./rtree_cpp -r ./data/spatial_data.txt ./queries/range_query.txt
```

### 2. k-Nearest Neighbors (kNN) Query
To perform a k-NN query, use the `-n` flag followed by `-k <number_of_neighbors>`:
```sh
./rtree_cpp -n -k 5 "filepath_of_dataset" "filepath_of_query_dataset"
```
Example:
```sh
./rtree_cpp -n -k 5 ./data/spatial_data.txt ./queries/knn_query.txt
```

### 3. Spatial Join Query
To perform a spatial join query, use the `-j` flag:
```sh
./rtree_cpp -j "filepath_of_dataset1" "filepath_of_dataset2"
```
Example:
```sh
./rtree_cpp -j ./data/dataset1.txt ./data/dataset2.txt
```

## Cleaning the Build
To remove all generated build files and clean the project, run:
```sh
rm -rf build
```
This will delete the `build` directory and all compiled binaries.

## Notes
- Ensure that the dataset and query files are correctly formatted before running the queries.
- If using large datasets, consider optimizing the build with `-O2` or `-O3` flags by modifying the CMake configuration.

---
This guide provides the necessary steps to build and execute your R-tree C++ implementation from the terminal. If you encounter any issues, check the error logs and dependencies. Happy coding!
