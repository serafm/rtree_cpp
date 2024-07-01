//
// Created by serafm on 14/6/2024.
//

#ifndef PROCEDURE_H
#define PROCEDURE_H

#include <vector>
#include <iostream>
#include <stdexcept>

namespace Collections {
    class Procedure {
    public:

        int pos = 0;
        std::vector<float> target;

        /**
         * Executes this procedure. A false return value indicates that
         * the application executing this procedure should not invoke this
         * procedure again.
         *
         * @param value a value of type int
         * @return true if additional invocations of the procedure are
         * allowed.
         */
        bool execute(int value) {
            target[pos++] = value;
            return true;
        };

        // Virtual destructor to ensure proper cleanup of derived classes
        ~Procedure() = default;
    };
}
#endif //PROCEDURE_H
