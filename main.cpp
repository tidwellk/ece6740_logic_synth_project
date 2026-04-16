#include <iostream>

#include "SolutionState.h"

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

std::vector<Val> bcp(SolutionState a)
{
    std::vector<Val> best;

    return best;
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

    SolutionState currentCover(filename);

    currentCover.printMatrix();

    currentCover.printSolution();

    std::cout << "matrix: reduce()" << std::endl;

    currentCover.reduce();

    currentCover.printMatrix();
    currentCover.printSolution();

    // SolutionState test_mis("h.txt");

    // test_mis.printMatrix();

    // test_mis.reduce();

    // test_mis.printMatrix();

    // std::vector<Val> bestcover = bcp(currentCover);

    return 0;
}
