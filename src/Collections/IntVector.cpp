#include "IntVector.h"
#include <vector>
#include <stdexcept>

namespace Collections {

    // Creates a new TIntArrayList instance with the default capacity.
    IntVector::IntVector() {
        data = std::vector<int>(DEFAULT_CAPACITY, 0);
    }

    // Creates a new TIntArrayList instance with the specified capacity.
    IntVector::IntVector(int capacity) {
        data = std::vector<int>(capacity, 0);
        position = 0;
    }

    // Adds value to the end of the list.
    bool IntVector::add(uint32_t value) {
        data.insert(data.begin(), value);
        return true;
    }

    // Removes the value at the specified offset.
    int IntVector::removeAt(int offset) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        int old = get(offset);
        data.erase(data.begin() + offset);
        return old;
    }

    // Returns the value at the specified offset.
    int IntVector::get(int offset) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        return data[offset];
    }

    // Returns the number of values in the list.
    int IntVector::size() {
        return data.size();
    }

    // Sets the value at the specified offset.
    int IntVector::set(int offset, int value) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        int previous_value = data[offset];
        data[offset] = value;
        return previous_value;
    }

    void IntVector::clear() {
        clear(DEFAULT_CAPACITY);
    }

    // Flushes the internal state of the list, setting the capacity of the empty list to capacity.
    void IntVector::clear(int capacity) {
        data = std::vector<int>(capacity);
    }

    /** TODO:
    Sets the size of the list to 0, but does not change its capacity. This method can
    be used as an alternative to the clear() method if you want to recycle a
    list without allocating new backing arrays.
    **/
    void IntVector::reset() {
        position = 0;
        data.clear();
    }

}