#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>

// #include "matrixcover.h"

enum Val
{
    ZERO = 0,
    ONE = 1,
    DC = 2
};

char valToChar(Val v)
{
    if (v == ONE)
        return '1';
    if (v == ZERO)
        return '0';
    return '-';
}

bool parse_literal(const std::string &token, int &var_index, Val &value)
{
    if (token.empty() || token[0] != 'x')
    {
        return false;
    }

    size_t pos = 1;

    // move past the digits after x
    while (pos < token.size() && std::isdigit(static_cast<unsigned char>(token[pos])))
    {
        pos++;
    }

    if (pos == 1)
    {
        return false; // no number after x
    }

    int var_num = std::stoi(token.substr(1, pos - 1));
    var_index = var_num - 1;

    if (pos == token.size())
    {
        value = ONE; // x3
        return true;
    }

    if (pos + 1 == token.size() && token[pos] == '\'')
    {
        value = ZERO; // x3'
        return true;
    }

    return false;
}

void printMatrix(std::vector<std::vector<Val>> &matrix, std::vector<std::string> &rownames, std::vector<std::string> &colnames) // pass using the by reference operator &. this means we don't make a deep copy of the matrix just to print it
{
    if (matrix.empty())
    {
        std::cout << "Matrix is empty\n";
        return;
    }

    int num_vars = colnames.size();

    int colwidth = num_vars < 9 ? 3 : 4;

    // --- Print header ---
    std::cout << colnames[1];

    for (int i = 2; i < num_vars; i++)
    {
        std::cout << std::setw(colwidth) << colnames[i];
    }
    std::cout << "\n";

    // --- Print separator ---

    for (int i = 0; i < num_vars + 1; i++)
    {
        std::cout << "--";
    }
    std::cout << "\n";

    // --- Print rows ---
    for (size_t i = 0; i < matrix.size(); i++)
    {

        // print row values
        for (Val v : matrix[i])
        {
            std::cout << valToChar(v) << "  ";
        }

        // print row name
        std::cout << "\t" << rownames[i];
        std::cout << "\n";
    }
}

int find_essential_row_number(std::vector<std::vector<Val>> &matrix, Val &essentialVal_value, int &colnumber)
{

    for (int i = 0; i < matrix.size(); i++)
    {
        std::vector<Val> row = matrix[i];
        int howManyAssigned = 0;
        Val assignedVal;

        for (int j = 0; j < row.size(); j++)
        {
            Val currentVal = row[j];
            // for (Val currentVal : row)
            if (currentVal != DC)
            {
                assignedVal = currentVal;
                howManyAssigned++;
                colnumber = j;
            }
        }

        if (howManyAssigned == 1)
        {
            essentialVal_value = assignedVal;
            return i;
        }
    }

    return -1;
}

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
std::vector<Val> remove_essential_row(std::vector<std::vector<Val>> &matrix,
                          std::vector<std::string> &rownames,
                          std::vector<std::string> &colnames,
                          int rownumber, int colnumber,
                          std::vector<Val> solution)
{
    auto row = matrix[rownumber];
    Val curval = row[colnumber];
    solution[colnumber] = curval;
    // solution[colnumber] = matrix[rownumber][colnumber];

    return solution; // TODO
}

void reduce(std::vector<std::vector<Val>> &matrix,
            std::vector<std::string> &rownames,
            std::vector<std::string> &colnames,
            std::vector<Val> solution)
{
    std::vector<std::vector<Val>> matrix_orig;

    // auto rownames_copy = rownames;
    // auto colnames_copy = colnames;

    do
    {
        matrix_orig = matrix; // std::vector does a deep copy when you assign it like this

        // select unate variables first?

        // find essentials
        Val essentialVal_value;
        int colnumber = -1;
        int rownumber = find_essential_row_number(matrix, essentialVal_value, colnumber);
        if (rownumber >= 0 && colnumber >= 0)
        {
            remove_essential_row(matrix, rownames, colnames, rownumber, colnumber, solution);
        }

        // delete dominating rows

        // delete dominated columns

    } while (
        !matrix.empty() &&
        matrix != matrix_orig);
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

    int how_many_x_vars = 0;
    std::ifstream infile(filename);
    std::string line;

    while (std::getline(infile, line))
    {
        std::stringstream ss(line);

        std::string token;

        while (ss >> token)
        {
            int var_number = std::stoi(token.substr(1));
            if (var_number > how_many_x_vars)
            {
                how_many_x_vars = var_number;
            }
        }
    }

    // now we know how many columns to have in the matrix
    std::cout << "we have " << how_many_x_vars << " x variables\n"
              << std::endl;

    infile.clear(); // clear the EOF flag
    infile.seekg(0);

    std::vector<std::string> rownames;
    std::vector<std::string> colnames;

    std::vector<std::vector<Val>> matrix;

    while (std::getline(infile, line))
    {
        if (line.empty())
        {
            continue;
        }

        // std::cout << line << std::endl;

        rownames.push_back(line);

        std::vector<Val> row(how_many_x_vars, DC);

        std::stringstream ss(line);
        std::string token;

        int line_num = 0;
        while (ss >> token) // keep extracting words from ss into token until it fails (no more tokens). they are white space separated tokens
        {
            line_num++;

            int var_index;
            Val value;

            if (!parse_literal(token, var_index, value))
            {
                std::cerr << "Bad token \"" << token
                          << "\" on line " << line_num << "\n";
                return 1;
            }

            if (var_index < 0 || var_index >= how_many_x_vars)
            {
                std::cerr << "Variable out of range on line " << line_num << "\n";
                return 1;
            }

            row[var_index] = value;
        }

        matrix.push_back(row);
    }

    infile.close();

    // column number should match the x variable number, and we are starting count from x1
    colnames.push_back(" ");

    for (int i = 0; i < how_many_x_vars; i++)
    {
        std::string xname = "x";

        xname += std::to_string(i + 1);

        colnames.push_back(xname);
    }

    printMatrix(matrix, rownames, colnames);

    std::vector<Val> solution(how_many_x_vars, DC);

    reduce(matrix, rownames, colnames, solution);

    return 0;
}
