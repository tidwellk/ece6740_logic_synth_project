#include "Assignment.h"

int Assignment::getVariableNameIdx()
{
    return col2var_idx;
}

Val Assignment::getVariableVal()
{
    return value;
}

Assignment::Assignment(int col2var_idx, Val value)
    : col2var_idx(col2var_idx), value(value)
{
}

Assignment::~Assignment() = default;

Assignment::Assignment(const Assignment &other)
    : col2var_idx(other.col2var_idx), value(other.value)
{
}

Assignment::Assignment(Assignment &&other) noexcept
    : col2var_idx(other.col2var_idx), value(other.value)
{
}

Assignment &Assignment::operator=(const Assignment &other)
{
    if (this != &other)
    {
        col2var_idx = other.col2var_idx;
        value = other.value;
    }

    return *this;
}

Assignment &Assignment::operator=(Assignment &&other) noexcept
{
    if (this != &other)
    {
        col2var_idx = other.col2var_idx;
        value = other.value;
    }

    return *this;
}
