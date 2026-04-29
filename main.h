#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <string>
#include <vector>
#include <optional>

#include "SolutionState.h"
#include "Val.h"

/// @brief Branch-and-bound Boolean covering solver.
/// Pre:  upperbound > 0. Typically initialized to (num_vars + 1) so any valid solution qualifies.
/// Post: If non-nullopt, the returned state satisfies isEmpty() and cost() < upperbound.
///       If nullopt, no solution with cost < upperbound exists under state f.
std::optional<SolutionState> bcp(SolutionState a, int upperbound);

/// @brief Returns true when state f cannot lead to a valid solution.
/// Two conditions trigger infeasibility:
///   1. A contradictory forced assignment was recorded (is_valid() == false).
///   2. A clause row contains only DC entries — no literal can satisfy it.
bool isInfeasible(SolutionState &f);

/// @brief Returns the lower-cost solution of s1 and s0, or nullopt if both are absent.
std::optional<SolutionState> best_solution(std::optional<SolutionState> s1, std::optional<SolutionState> s0);

/// @brief Runs the full BCP solver on filename and prints the result.
void test(std::string filename);

void test_reduce(std::string filename);

void test_choose_var(std::string filename, int expected_column);

#endif
