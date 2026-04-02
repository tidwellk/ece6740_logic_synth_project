#ifndef VAL_H
#define VAL_H

#include <iostream>

enum Val
{
    ZERO = 0,
    ONE = 1,
    DC = 2
};

struct Assignment {
    std::string name;
    Val value;
};

#endif