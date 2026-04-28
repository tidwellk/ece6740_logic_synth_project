#include "main.h"

/*
When you find an essential row, what you really found is a forced variable assignment, not merely “a row to delete.”

So after adding that row’s literal or clause name to the solution, the next question is:

what rows are now satisfied by that forced assignment?

Those rows should be removed too.

For example, if the essential row forces:

x1=1

then every row containing 1 in column x1 is satisfied and can be removed, not only the single essential row.

Then there is a second effect:

rows containing the opposite polarity in that same column are not satisfied
but that literal is now impossible, so that entry should be removed from those rows

In your matrix representation, that means the entry in that column becomes -- for those remaining rows.

So the full mental model is:

find essential row
extract the forced assignment (variable, value)
add that assignment to the solution
remove all rows satisfied by that assignment
in all remaining rows, erase that variable’s column information
if any row becomes all --, that is a contradiction for that branch
continue reduction loop

So yes, you absolutely do need to check other rows with the same variable. That is the important part.

The essential row is just the clue that tells you the assignment is forced. The action applies to the whole matrix.

One subtle point: if another row has the same variable with the same polarity, it gets removed as satisfied. If it has the opposite polarity, it is not removed automatically — it just loses that literal and may become smaller, maybe even essential itself.*/

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
 * Temporary test function.
 */
void test_lower_bound(std::string filename)
{
    SolutionState a(filename);
    std::cout << "lower_bound before reduce: " << a.lower_bound() << std::endl;
    a.reduce();
    std::cout << "lower_bound after reduce:  " << a.lower_bound() << std::endl;
    a.printSolution();
}

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

/// @brief check if we have a valid solution. it is invalid if we have a conflicting assignment of essential variables
///         (this boolean is changed in the assignment function)
///         or if we have a row of all DC don't cares
/// @param f
/// @return
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

/// @brief this function reads 'filename' and runs our bcp covering algorithm with it
/// @param filename 
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

/// @brief returns the lowest cost solution, or nullopt if neither branch has a solution
/// @param s1
/// @param s0
/// @return
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
