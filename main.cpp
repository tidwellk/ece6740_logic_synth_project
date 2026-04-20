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
    /* pseudocode from textbook page 336

    BCP(F, U, currentSol)
    {
1   (F, currentSol) = REDUCE(F, currentSol)
    if (terminalCase(F))
    {
        if (F != empty and COST( currentSol) < U)
        {
            U = COST( currentSol)
2           return ( currentSol)
        }
3       else return ("no solution")
    }

4   L = LOWER_BOUND{F, currentSol)
      if (L >= U) return ("no solution")

5   Xi = CHOOSE_VAR(F)

6   S 1 = BCP(Fx, U, currentSol UNION {xi})

7   if (cost(S1) = L) return (S1)
    S 0 = BCP(Fxi, U, currentSol)

8   return BEST_SOLUTION(S1, SO)
    }
    */

    f.reduce();

    if (terminalCase(f))
    {
        if (!f.isEmpty() &&
            f.cost() < upperbound)
        {
            upperbound = f.cost();
        }
        else
        {
            // return "no solution"
            return std::nullopt;
        }
    }

    int lowerbound = f.lower_bound();

    if (lowerbound >= upperbound)
    {
        // return no solution
        return std::nullopt;
    }

    // at this point we need to branch
    /*
    // TODO
    the pseudocode says xi = choose_var(f). maybe choose_var could be a public function in the class because
        it really depends on the current state.
    */
    int chosen_column = f.choose_var();
    /*
     // TODO
     then we could make a copy of f, so S1 = f AND assign_var(xi = 1).
     S1 = f    uses the copy constructor to do a deep copy
     S1.assign_var()
     */
    SolutionState s1 = f;
    s1.assign_a_variable(chosen_column, ONE, ESSENTIAL);

    // do we need to call reduce here? im not sure

    if (s1.cost() == lowerbound)
    {

        return s1;
    }

    //  S0 = f AND assign_var(xi = 0)
    SolutionState s0 = f;
    s0.assign_a_variable(chosen_column, ZERO, ESSENTIAL);

    // might need to s0.reduce() ? i don't know

    return best_solution(s1, s0);
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

int main(int argc, char **argv)
{
    std::cout << "main()" << std::endl;

    if (argc < 2)
    {
        std::cout << "You need to run the command with a filename: ./main.out f.txt" << std::endl;
        std::cout << "f.txt is one clause per line, each literal separated by spaces" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    test_reduce(filename);

    test_reduce("h.txt");

    // std::vector<Val> bestcover = bcp(test_mis);

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

bool terminalCase(SolutionState &f)
{
    if (f.isEmpty())
    {
        return true;
    }

    if (isInfeasible(f))
    {
        return true;
    }
    return false;
}

/// @brief returns the lowest cost solution
/// @param s1 
/// @param s0 
/// @return 
SolutionState best_solution(SolutionState s1, SolutionState s0)
{
    if (s1.cost() < s0.cost())
    {
        return s1;
    }
    return s0;
}
