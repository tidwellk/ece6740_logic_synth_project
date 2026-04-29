#ifndef VAL_H
#define VAL_H

#include <iostream>

// Truth value of a literal in a Boolean covering problem.
// DC (don't-care) means the variable is absent from that clause row.
// UNASSIGNED is the initial state of a solution variable before branching.
enum Val
{
    ZERO = 0,
    ONE = 1,
    DC = 2,
    UNASSIGNED = 3
};

#endif