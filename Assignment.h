#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include <iostream>

#include "Val.h"

class Assignment
{

private:

    int col2var_idx;
    Val value;

public:

    int getVariableNameIdx();
    Val getVariableVal();

    /// @brief default constructor
    Assignment(int col2var_idx, Val value);

    /// @brief destructor
    ~Assignment();

    /// Rule of five: copy / move
    Assignment(const Assignment &other);
    Assignment(Assignment &&other) noexcept;
    Assignment &operator=(const Assignment &other);
    Assignment &operator=(Assignment &&other) noexcept;

};

#endif