#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <string>
#include <vector>
#include <optional>

#include "SolutionState.h"
#include "Val.h"

std::optional<SolutionState> bcp(SolutionState a, int upperbound);

void test_reduce(std::string filename);

void test_choose_var(std::string filename, int expected_column);

bool isInfeasible(SolutionState &f);

void test(std::string filename);

std::optional<SolutionState> best_solution(std::optional<SolutionState> s1, std::optional<SolutionState> s0);

#endif
