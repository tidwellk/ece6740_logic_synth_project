#ifndef SOLUTIONSTATE_H
#define SOLUTIONSTATE_H

#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <algorithm>

#include "Val.h"
#include "Assignment.h"

class SolutionState
{

private:
    std::vector<std::vector<Val>> matrix;

    std::vector<std::string> rownames;
    std::vector<std::string> colnames;
    std::vector<int> current_column_to_colnames_idx;
    // std::vector<int> current_row_to_rownames_idx;

    int how_many_x_vars = 0;

    // std::vector<Assignment> solutionObj;
    std::vector<Val> solution;

    void populate_solutions_array();

    char valToChar(Val v);

    bool parse_literal(const std::string &token, int &var_index, Val &value);

    bool readInputFile_isOK(std::string filename);

    void populate_colnames_array();

    void remove_essential_rows();
    bool find_essential_row();

    void remove_dominating_rows();
    void remove_dominated_columns();

    void remove_rows_with_same_val(int column_to_check, Val value);
    void remove_row_number(int rownum);
    void remove_column(int column_number);

    bool assign_a_variable(int current_column_number, Val val_to_assign);

public:
    /// @brief default constructor
    SolutionState();
    SolutionState(std::string filename);
    /// @brief destructor
    ~SolutionState();

    /// Rule of five: copy / move
    SolutionState(const SolutionState &other);
    SolutionState(SolutionState &&other) noexcept;
    SolutionState &operator=(const SolutionState &other);
    SolutionState &operator=(SolutionState &&other) noexcept;

    /// override == operator
    bool operator==(const SolutionState &other) const;

    bool operator!=(const SolutionState &other) const;

    /// @brief prints the current matrix cover
    void printMatrix();

    void printSolution();

    std::vector<std::vector<Val>> getMatrix();

    /// @brief reduces the matrix in place without making a copy of the object
    void reduce();
};

#endif