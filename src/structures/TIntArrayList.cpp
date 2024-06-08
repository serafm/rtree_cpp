//
// Created by serafm on 7/6/2024.
//
#include "TIntArrayList.h"
#include <vector>
#include <stdexcept>

namespace DataStructures {

    // Creates a new TIntArrayList instance with the default capacity.
    TIntArrayList::TIntArrayList() {
        data = std::vector<int>(DEFAULT_CAPACITY, 0);
    }

    // Creates a new TIntArrayList instance with the specified capacity.
    TIntArrayList::TIntArrayList(int capacity) {
        data = std::vector<int>(capacity, 0);
    }

    // Adds value to the end of the list.
    bool TIntArrayList::add(int value) {
        data.push_back(value);
        return true;
    }

    // Removes the value at the specified offset.
    int TIntArrayList::removeAt(int offset) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        int old = get(offset);
        data.erase(data.begin() + offset);
        return old;
    }

    // Returns the value at the specified offset.
    int TIntArrayList::get(int offset) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        return data[offset];
    }

    // Returns the number of values in the list.
    int TIntArrayList::size() {
        return data.size();
    }

    // Sets the value at the specified offset.
    int TIntArrayList::set(int offset, int value) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        int previous_value = data[offset];
        data[offset] = value;
        return previous_value;
    }

    // Flushes the internal state of the list, setting the capacity of the empty list to capacity.
    void TIntArrayList::clear(int capacity) {
        data = std::vector<int>(capacity);
    }

    /** TODO:
    Sets the size of the list to 0, but does not change its capacity. This method can
    be used as an alternative to the clear() method if you want to recycle a
    list without allocating new backing arrays.
    **/
    void TIntArrayList::reset() {}
}