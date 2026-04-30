#ifndef SOLUTIONSTATE_H
#define SOLUTIONSTATE_H

// Passed to assign_a_variable to indicate whether the assignment is forced by an
// essential row (updates forced_solution) or inferred by column dominance (does not).
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

// Represents one node in the branch-and-bound search tree for the Boolean covering problem.
// The matrix holds the remaining uncovered clauses; columns are unassigned variables.
// Two solution vectors track assignments: forced_solution (from essential rows / column
// dominance) and current_assignment (all assignments including branch decisions).
class SolutionState
{

private:
    std::vector<std::vector<Val>> matrix;

    std::vector<std::string> rownames;
    std::vector<std::string> colnames;
    // Maps the current (possibly shrunk) column index back to the original variable index
    // in colnames / forced_solution / current_assignment.
    std::vector<int> current_column_to_colnames_idx;

    int how_many_x_vars = 0;

    std::vector<Val> current_assignment;
    std::vector<Val> forced_solution;

    bool is_valid_solution;

    // Initializes forced_solution and current_assignment to UNASSIGNED for each variable.
    void populate_solutions_array();

    char valToChar(Val v);

    // Parses a literal token of the form "xN" (positive) or "xN'" (negative).
    // Sets var_index = N-1 and value = ONE or ZERO. Returns false on malformed input.
    bool parse_literal(const std::string &token, int &var_index, Val &value);

    // Reads the input file, determines how_many_x_vars, and populates matrix and rownames.
    // Returns false if the file cannot be opened or contains a malformed literal.
    bool readInputFile_isOK(std::string filename);

    // Fills colnames with "x1", "x2", ... and initializes current_column_to_colnames_idx.
    void populate_colnames_array();

    // Repeatedly calls find_essential_row() until no essential rows remain.
    void remove_essential_rows();

    // Finds one essential row (exactly one non-DC entry), forces that variable assignment,
    // and removes all rows now covered by the assignment.
    // Returns true if an essential row was found and processed, false otherwise.
    bool find_essential_row();

    // Removes dominating rows from the matrix.
    // Row i dominates row j when every non-DC entry in i also appears in j, meaning i is
    // strictly harder to cover and can be dropped without affecting solution validity.
    void remove_dominating_rows();

    // Assigns 0 to any column dominated by another and removes it from the matrix.
    // Column a dominates column b when setting a=1 covers every row that b=1 would cover,
    // so b can be permanently set to 0 (NOT_ESSENTIAL) without losing optimality.
    void remove_dominated_columns();

    // Removes all rows where matrix[row][column_to_check] == value (those rows are now covered).
    void remove_rows_with_same_val(int column_to_check, Val value);
    void remove_row_number(int rownum);

    // Erases column_number from every row vector and updates current_column_to_colnames_idx.
    void remove_column(int column_number);

    public:
    /// @brief Constructs an empty solution state.
    SolutionState();

    /// @brief Constructs a solution state by reading a covering problem from a file.
    /// @param filename Input covering problem file.
    SolutionState(std::string filename);

    /// @brief Destroys the solution state.
    ~SolutionState();
    
    /// @brief Copy-constructs a solution state.
    /// @param other Source state to copy.
    SolutionState(const SolutionState &other);

    /// @brief Move-constructs a solution state.
    /// @param other Source state to move from.
    SolutionState(SolutionState &&other) noexcept;

    /// @brief Copy-assigns a solution state.
    /// @param other Source state to copy.
    /// @return Reference to this object after assignment.
    SolutionState &operator=(const SolutionState &other);

    /// @brief Move-assigns a solution state.
    /// @param other Source state to move from.
    /// @return Reference to this object after assignment.
    SolutionState &operator=(SolutionState &&other) noexcept;
    
    /// @brief Compares two states for matrix equality.
    /// @param other State to compare against.
    /// @return True if the matrices are identical.
    bool operator==(const SolutionState &other) const;
    
    /// @brief Compares two states for matrix inequality.
    /// @param other State to compare against.
    /// @return True if the matrices differ.
    bool operator!=(const SolutionState &other) const;
    
    /// @brief Prints the current matrix cover in tabular form.
    void printMatrix();
    
    /// @brief Prints the current forced and branch assignments.
    void printSolution();
    
    /// @brief Returns the current matrix representation.
    /// @return Copy of the current matrix.
    std::vector<std::vector<Val>> getMatrix();
    
    /// @brief Applies essential-row, dominating-row, and dominated-column reductions
    ///        to convergence. Mutates this state in place.
    void reduce();

    /// @brief Returns the number of ONE entries in forced_solution.
    int cost() const;

    /// @brief Returns false if a contradictory assignment was recorded via assign_a_variable.
    bool is_valid();

    /// @brief Returns current_assignment (includes both forced and branch assignments).
    std::vector<Val> getSolution();

    /// @brief Returns the total number of x variables (including already-assigned ones).
    int getHowManyXVars() const;

    /// @brief Returns true when all clauses have been covered (matrix has no rows).
    bool isEmpty();

    /// @brief Computes a greedy lower bound on the minimum additional cost to cover the matrix.
    /// @return cost() + greedy_lb if feasible; -1 if the remaining matrix contains
    ///         an infeasible row (all DC entries, meaning a clause can never be satisfied).
    int lower_bound() const;

    /// @brief Assigns val_to_assign to the variable at current_column_number, then removes
    ///        covered rows and the column.
    /// @param current_column_number Column index in the current reduced matrix.
    /// @param val_to_assign Value to assign (ONE or ZERO).
    /// @param isForcedEssential True when the assignment comes from an essential row.
    /// Pre:  0 <= current_column_number < current matrix column count.
    ///       val_to_assign is ONE or ZERO.
    /// Post: Column is erased from the matrix. Rows where that column == val_to_assign are removed.
    ///       If isForcedEssential, forced_solution is updated; current_assignment is always updated.
    /// @return false (and sets is_valid_solution=false) on a contradictory forced assignment; true otherwise.
    bool assign_a_variable(int current_column_number, Val val_to_assign, bool isForcedEssential);

    /// @brief Selects the branching variable using a short-row / binate heuristic.
    /// @return Index into the current (reduced) column list, or -1 if the matrix is empty.
    int choose_var();
};

#endif
