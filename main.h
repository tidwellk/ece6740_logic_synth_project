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

bool isInfeasible(SolutionState &f);
// bool terminalCase(SolutionState &f);

std::optional<SolutionState> best_solution(std::optional<SolutionState> s1, std::optional<SolutionState> s0);

#endif
