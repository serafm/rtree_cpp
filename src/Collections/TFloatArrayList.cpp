//
// Created by serafm on 8/6/2024.
//

#include "TFloatArrayList.h"

#include <stdexcept>

namespace Collections {

    // Creates a new TFloatArrayList instance with the default capacity.
    TFloatArrayList::TFloatArrayList() {
        data = std::vector<float>(DEFAULT_CAPACITY, 0);
    }

    // Creates a new TFloatArrayList instance with the specified capacity.
    TFloatArrayList::TFloatArrayList(int capacity) {
        data = std::vector<float>(capacity, 0);
    }

    // Adds value to the end of the list.
    bool TFloatArrayList::add(float value) {
        data.push_back(value);
        return true;
    }

    // Removes the value at the specified offset.
    float TFloatArrayList::removeAt(int offset) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        float old = get(offset);
        data.erase(data.begin() + offset);
        return old;
    }

    // Returns the value at the specified offset.
    float TFloatArrayList::get(int offset) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        return data[offset];
    }

    // Returns the number of values in the list.
    int TFloatArrayList::size() {
        return data.size();
    }

    // Sets the value at the specified offset.
    float TFloatArrayList::set(int offset, float value) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        float previous_value = data[offset];
        data[offset] = value;
        return previous_value;
    }

    void TFloatArrayList::clear() {
        clear(DEFAULT_CAPACITY);
    }

    // Flushes the internal state of the list, setting the capacity of the empty list to capacity.
    void TFloatArrayList::clear(int capacity) {
        data = std::vector<float>(capacity);
    }

    /** TODO:
    Sets the size of the list to 0, but does not change its capacity. This method can
    be used as an alternative to the clear() method if you want to recycle a
    list without allocating new backing arrays.
    **/
    void TFloatArrayList::reset() {}
    
} // DataStructures