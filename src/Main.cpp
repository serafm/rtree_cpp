#include <iostream>
#include "RTree.h"
#include "Point.h"

using namespace SpatialIndex;
using namespace std;

int main() {
    Rectangle rect = Rectangle(0.1, 0.2, 0.3, 0.4);
    Rectangle rect2 = Rectangle(0.41, 0.72, -0.3, -0.4);
    Rectangle rect3 = Rectangle(-0.01, 0.62, 0.13, 0.214);
    Point point = Point(0.2, 0.3);

    Collections::Procedure proc = Collections::Procedure();

    RTree rtree;
    rtree.init();
    rtree.add(rect, 1345);
    rtree.add(rect2, 1234);
    rtree.add(rect3, 5678);
    rtree.contains(rect2, proc);

    cout << "RTree size: " << rtree.treeSize() << endl;
    rtree.intersects(rect2, proc);


    return 0;
}
