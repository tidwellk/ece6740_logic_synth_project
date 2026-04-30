#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <string>
#include <vector>
#include <optional>

#include "SolutionState.h"
#include "Val.h"

/// @brief Solves a covering problem state using recursive branch-and-bound.
/// @param f Current covering problem state.
/// @param upperbound Cost threshold used for pruning.
/// @return The best solution below upperbound, or nullopt if none exists.
std::optional<SolutionState> bcp(SolutionState f, int upperbound);

/// @brief Returns true if the current state cannot lead to a valid solution.
/// @param f Current covering problem state.
/// @return True when assignments conflict or a row contains only don't-cares.
bool isInfeasible(SolutionState &f);

/// @brief Chooses the lower-cost solution from two recursive branch results.
/// @param s1 Solution from the x=1 branch, if one exists.
/// @param s0 Solution from the x=0 branch, if one exists.
/// @return The lower-cost solution, or nullopt if neither branch succeeds.
std::optional<SolutionState> best_solution(std::optional<SolutionState> s1, std::optional<SolutionState> s0);

/// @brief Runs the full BCP solver on a file and prints the result.
/// @param filename Input covering problem file.
void test(std::string filename);

/// @brief Debug helper that prints a matrix before and after one full reduce() call.
/// @param filename Input covering problem file.
void test_reduce(std::string filename);

/// @brief Debug helper that prints the lower bound before and after reduction.
/// @param filename Input covering problem file.
void test_lower_bound(std::string filename);

/// @brief Debug helper for validating choose_var() on a file with a known expected column.
/// @param filename Input covering problem file.
/// @param expected_column Expected reduced-matrix column index.
void test_choose_var(std::string filename, int expected_column);

#endif
