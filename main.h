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
bool terminalCase(SolutionState &f);

SolutionState best_solution(SolutionState s1, SolutionState s0);

#endif
