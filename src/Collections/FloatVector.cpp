#include "FloatVector.h"
#include <stdexcept>

namespace Collections {

    // Creates a new FloatVector instance with the default capacity.
    FloatVector::FloatVector() {
        data = std::vector<float>(DEFAULT_CAPACITY, 0);
    }

    // Creates a new TFloatArrayList instance with the specified capacity.
    FloatVector::FloatVector(int capacity) {
        data = std::vector<float>(capacity, 0);
    }

    // Adds value to the end of the list.
    bool FloatVector::add(float value) {
        data.push_back(value);
        return true;
    }

    // Removes the value at the specified offset.
    float FloatVector::removeAt(int offset) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        float old = get(offset);
        data.erase(data.begin() + offset);
        return old;
    }

    // Returns the value at the specified offset.
    float FloatVector::get(int offset) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        return data[offset];
    }

    // Returns the number of values in the list.
    int FloatVector::size() const {
        return data.size();
    }

    // Sets the value at the specified offset.
    float FloatVector::set(int offset, float value) {
        if (offset >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        float previous_value = data[offset];
        data[offset] = value;
        return previous_value;
    }

    void FloatVector::clear() {
        clear(DEFAULT_CAPACITY);
    }

    // Flushes the internal state of the list, setting the capacity of the empty list to capacity.
    void FloatVector::clear(int capacity) {
        data = std::vector<float>(capacity);
    }

    /** TODO:
    Sets the size of the list to 0, but does not change its capacity. This method can
    be used as an alternative to the clear() method if you want to recycle a
    list without allocating new backing arrays.
    **/
    void FloatVector::reset() {}
    
} // DataStructures