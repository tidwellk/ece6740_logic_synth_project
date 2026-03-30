#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

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

void printMatrix(std::vector<std::vector<Val>> &matrix, std::vector<std::string> &rownames) // pass using the by reference operator &. this means we don't make a deep copy of the matrix just to print it
{
    if (matrix.empty())
    {
        std::cout << "Matrix is empty\n";
        return;
    }

    int num_vars = matrix[0].size();

    // --- Print header ---

    for (int i = 0; i < num_vars; i++)
    {
        std::cout << "x" << (i + 1) << " ";
    }
    std::cout << "\n";

    // --- Print separator ---

    for (int i = 0; i < num_vars; i++)
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
    std::cout << "we have " << how_many_x_vars << " x variables\n" << std::endl;

    infile.clear(); // clear the EOF flag
    infile.seekg(0);

    std::vector<std::string> rownames;
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

    printMatrix(matrix, rownames);

    return 0;
}
