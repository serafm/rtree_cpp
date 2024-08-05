# RTree algorithm steps

### Insert Operation

##### 1. Find Position for New Record (Insert Algorithm)
    1.1 Invoke ChooseLeaf: Select a leaf node L in which to place the new entry E.
    1.2 Add Record to Leaf Node:
        - If L has room, add E.
        - If L is full, invoke SplitNode to split L into L and LL and distribute entries, including E, between them.
    1.3 Propagate Changes Upward: Invoke AdjustTree, passing L and LL (if a split occurred).
    1.4 Grow Tree Taller: If the root was split, create a new root with L and LL as its children.

##### 2. ChooseLeaf Algorithm
    2.1 Initialize: Set N to the root node.
    2.2 Leaf Check: If N is a leaf, return N.
    2.3 Choose Subtree: Select the entry F in N whose rectangle F.I needs the least enlargement to include E.I. Resolve ties by choosing the smallest area rectangle.
    2.4 Descenf Until Leaf: Set N to the child node pointed to by F.p and repeat from step 2.2.

##### 3. AdjustTree Algorithm
    3.1 Initialize: Set N to L. If L was split, set NN to the resulting second node.
    3.2 Check if Done: If N is the root, stop.
    3.3 Adjust Parent Entry's Rectangle: Adjust EN.I to tightly enclose all entry rectangles in N.
    3.4 Propagate Node Split Upward: If N has a partner NN, create a new entry ENN and add it to P if there is room. Otherwise, split P.
    3.5 Move Up One Level: Set N to P and repeat from step 3.2.

##### 4. SplitNode Algorithm
    4.1 Quadratic Split Algorithm is used to divide a node N containing M+1 entries into two groups.
