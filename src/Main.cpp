#include <iostream>
#include "RTree.h"
#include "Point.h"

using namespace SpatialIndex;
using namespace std;

int main() {
    Rectangle rect = Rectangle(0.1, 0.2, 0.3, 0.4);
    Node node = Node(3445, 1, 50);
    int siz = node.getEntryCount();
    std::cout << "Node size: " << siz << std::endl;
    node.addEntry(rect.minX, rect.minY, rect.maxX, rect.maxY, 1);
    int id = node.entries[0].id;
    std::cout << "Node ID: " << id << std::endl;

    int result = node.findEntry(rect.minX, rect.minY, rect.maxX, rect.maxY, 1);
    std::cout << result << std::endl;
    //Collections::Procedure proc = Collections::Procedure();

    //RTree rtree;
    //rtree.init();
    //rtree.add(rect, 1345);
    //rtree.contains(rect, proc);

    //cout << "RTree size: " << rtree.treeSize() << endl;
    //cout << "Root node ID: " << rtree.getRootNodeId() << endl;



    return 0;
}
