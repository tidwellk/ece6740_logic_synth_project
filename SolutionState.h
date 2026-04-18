#ifndef SOLUTIONSTATE_H
#define SOLUTIONSTATE_H

#define NOT_ESSENTIAL false
#define ESSENTIAL true

#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <algorithm>

#include "Val.h"

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
    std::vector<Val> current_assignment;
    std::vector<Val> forced_solution;

    bool is_valid_solution;

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
    
    /// @brief print the current solution
    void printSolution();
    
    /// @brief getter for the matrix data structure
    /// @return 
    std::vector<std::vector<Val>> getMatrix();
    
    /// @brief reduces the matrix in place without making a new SolutionState object
    void reduce();
    
    /// @brief returns the cost of the current solution
    /// @return 
    int cost();
    
    /// @brief getter for the is_valid_solution private variable
    /// @return 
    bool is_valid();
    
    /// @brief getter for the current solution
    /// @return 
    std::vector<Val> getSolution();
    
    /// @brief returns true if the matrix is empty
    /// @return 
    bool isEmpty();
    
    /// @brief returns the lower bound of the current solution state
    /// @return 
    int lower_bound() const;
    
    /// @brief assign a variable a value. we take the current column number, not the actual x1, x2...
    /// @param current_column_number 
    /// @param val_to_assign 
    /// @param isForcedEssential 
    /// @return 
    bool assign_a_variable(int current_column_number, Val val_to_assign, bool isForcedEssential);

    int choose_var();
};

#endif