#ifndef SOLUTIONSTATE_H
#define SOLUTIONSTATE_H

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "Val.h"

class SolutionState
{

private:
    std::vector<std::vector<Val>> matrix;

    std::vector<std::string> rownames;
    std::vector<std::string> colnames;
    std::vector<int> col_2_var_number_idx;

    int how_many_x_vars = 0;

    char valToChar(Val v);

    bool parse_literal(const std::string &token, int &var_index, Val &value);

    bool readInputFile_isOK(std::string filename);

    void populate_colnames_array();

public:
    /// @brief default constructor
    SolutionState();

    SolutionState(std::string filename);

    /// @brief prints the current matrix cover
    void printMatrix();

    /// @brief destructor
    ~SolutionState();

    /// Rule of five: copy / move
    SolutionState(const SolutionState &other);
    SolutionState(SolutionState &&other) noexcept;
    SolutionState &operator=(const SolutionState &other);
    SolutionState &operator=(SolutionState &&other) noexcept;
};

#endif