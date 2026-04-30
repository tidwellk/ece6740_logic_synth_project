#include "main.h"

/**
 * @brief Solves a covering problem state using recursive branch-and-bound.
 * @param f Current covering problem state.
 * @param upperbound Cost threshold used for pruning.
 * @return The best solution below upperbound, or nullopt if none exists.
 */
std::optional<SolutionState> bcp(SolutionState f, int upperbound)
{
    f.reduce();

    // RECURSIVE BASE CASES
    if (f.isEmpty())
    {
        if (f.cost() < upperbound)
        {
            // upperbound = f.cost();
            return f;
        }
        else
        {
            return std::nullopt;
        }
    }

    if (isInfeasible(f))
    {
        return std::nullopt;
    }

    // RECURSIVE CALLS
    int lowerbound = f.lower_bound();

    if (lowerbound >= upperbound)
    {
        // return no solution
        return std::nullopt;
    }

    int chosen_column = f.choose_var();
    if (chosen_column < 0)
    {
        return std::nullopt;
    }

    SolutionState s1 = f;

    std::optional<SolutionState> s1opt;
    if (!s1.assign_a_variable(chosen_column, ONE, ESSENTIAL))
    {
        // if assign a variable created a contradiction, then we have no solution here
        s1opt = std::nullopt;
    }
    else
    {
        s1opt = bcp(s1, upperbound);
        if (s1opt)
        {
            if (s1opt->cost() == lowerbound)
            {
                return s1opt;
            }
            upperbound = std::min(upperbound, s1opt->cost());
        }
    }

    //  S0 = f AND assign_var(xi = 0)
    SolutionState s0 = f;
    std::optional<SolutionState> s0opt;

    if (!s0.assign_a_variable(chosen_column, ZERO, ESSENTIAL))
    {
        // if assign the variable fails, we have no solution
        s0opt = std::nullopt;
    }
    else
    {
        s0opt = bcp(s0, upperbound);
    }


    return best_solution(s1opt, s0opt);
}

/// @brief Debug helper that prints a matrix before and after one full reduce() call.
/// @param filename Input covering problem file.
void test_reduce(std::string filename)
{
    SolutionState a(filename);
    a.printMatrix();
    a.printSolution();
    std::cout << "matrix: reduce()" << std::endl;
    a.reduce();
    a.printMatrix();
    a.printSolution();
}

/**
 * @brief Debug helper that prints the lower bound before and after reduction.
 * @param filename Input covering problem file.
 */
void test_lower_bound(std::string filename)
{
    SolutionState a(filename);
    std::cout << "lower_bound before reduce: " << a.lower_bound() << std::endl;
    a.reduce();
    std::cout << "lower_bound after reduce:  " << a.lower_bound() << std::endl;
    a.printSolution();
}

/// @brief Debug helper for validating choose_var() on a file with a known expected column.
/// @param filename Input covering problem file.
/// @param expected_column Expected reduced-matrix column index.
void test_choose_var(std::string filename, int expected_column)
{
    SolutionState f(filename);

    std::cout << "Testing choose_var() with file: " << filename << std::endl;
    f.printMatrix();

    int chosen_column = f.choose_var();

    std::cout << "Expected column index: " << expected_column << std::endl;
    std::cout << "Actual column index:   " << chosen_column << std::endl;

    if (chosen_column == expected_column)
    {
        std::cout << "PASS: choose_var() returned the expected column." << std::endl;
    }
    else
    {
        std::cout << "FAIL: choose_var() did not return the expected column." << std::endl;
    }
}

/// @brief Program entry point for running the solver on a single input file.
/// @param argc Number of command-line arguments.
/// @param argv Command-line argument array. argv[1] is the input filename.
/// @return 0 on success, or 1 when no input file is provided.
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "You need to run the command with a filename, for example:" << std::endl;
        std::cout << "./main.out examples/f.txt" << std::endl;
        std::cout << "Each input file should contain one clause per line with space-separated literals." << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    // Default submission behavior: run the full BCP solver on the provided input file.
    // Temporary helper tests such as test_reduce(), test_lower_bound(), and
    // test_choose_var() are kept in this file to match the team's current
    // development workflow and can be called here when debugging individual steps.
    test(filename);

    return 0;
}

/// @brief Returns true if the current state cannot lead to a valid solution.
/// @param f Current covering problem state.
/// @return True when assignments conflict or a row contains only don't-cares.
bool isInfeasible(SolutionState &f)
{
    // check for conflicting assignments in the solution
    if (f.is_valid() == false)
    {
        return true;
    }

    // check for no remaining literals for a clause
    for (auto &row : f.getMatrix())
    {
        bool all_dont_cares = true;
        for (Val element : row)
        {
            if (element != DC)
            {
                all_dont_cares = false;
                break;
            }
        }

        if (all_dont_cares)
        {
            return true;
        }
    }
    return false;
}

/// @brief Runs the full BCP solver on a file and prints the result.
/// @param filename Input covering problem file.
void test(std::string filename)
{
    std::cout << std::endl;

    SolutionState f(filename);

    std::optional<SolutionState> f_solution = bcp(f, f.getHowManyXVars() + 1);

    std::cout << filename << "\t";

    if (f_solution)
    {
        std::cout << "solution exists" << std::endl;
        f_solution->printSolution();
    }
    else
    {
        std::cout << "no solution" << std::endl;
    }
}

/// @brief Chooses the lower-cost solution from two recursive branch results.
/// @param s1 Solution from the x=1 branch, if one exists.
/// @param s0 Solution from the x=0 branch, if one exists.
/// @return The lower-cost solution, or nullopt if neither branch succeeds.
std::optional<SolutionState> best_solution(std::optional<SolutionState> s1, std::optional<SolutionState> s0)
{
    if (!s1 && !s0)
    {
        return std::nullopt;
    }

    if (!s1)
    {
        return s0;
    }

    if (!s0)
    {
        return s1;
    }

    if (s1->cost() < s0->cost())
    {
        return s1;
    }
    
    return s0;
}
