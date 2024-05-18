#ifndef ENTRY_H
#define ENTRY_H

#include "Rectangle.h"

class Entry {
public:
    Rectangle rect;
    int data; // Assuming data is an integer for simplicity

    Entry(const Rectangle& rect, int data);
};

#endif // ENTRY_H
