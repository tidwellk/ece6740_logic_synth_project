#include "SolutionState.h"

/// @brief default constructor
SolutionState::SolutionState()
{
	std::cout << "SolutionState: default constructor" << std::endl;
}

/// @brief This constructor reads a text file into the object.
/// @param filename text file to read
SolutionState::SolutionState(std::string filename)
{
	if (!readInputFile_isOK(filename))
	{
		std::cout << "could not read input file" << std::endl;
		exit(1);
	}

	// now we know how many columns to have in the matrix
	std::cout << "we have " << how_many_x_vars << " x variables\n"
			  << std::endl;

	populate_colnames_array();
}

/// @brief destructor
SolutionState::~SolutionState()
{
	std::cout << "SolutionState: destructor" << std::endl;
}

/// @brief rule of 5 stuff auto generated from gpt
/// @param other 
SolutionState::SolutionState(const SolutionState &other)
	: matrix(other.matrix)
{
	std::cout << "SolutionState: copy constructor" << std::endl;
}

SolutionState::SolutionState(SolutionState &&other) noexcept
	: matrix(std::move(other.matrix))
{
	std::cout << "SolutionState: move constructor" << std::endl;
}

SolutionState &SolutionState::operator=(const SolutionState &other)
{
	if (this != &other)
		matrix = other.matrix;
	std::cout << "SolutionState: copy assignment" << std::endl;
	return *this;
}

SolutionState &SolutionState::operator=(SolutionState &&other) noexcept
{
	if (this != &other)
		matrix = std::move(other.matrix);
	std::cout << "SolutionState: move assignment" << std::endl;
	return *this;
}

char SolutionState::valToChar(Val v)
{
	if (v == ONE)
		return '1';
	if (v == ZERO)
		return '0';
	return '-';
}

bool SolutionState::parse_literal(const std::string &token, int &var_index, Val &value)
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

bool SolutionState::readInputFile_isOK(std::string filename)
{
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

	infile.clear(); // clear the EOF flag
	infile.seekg(0);

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
				return false;
			}

			if (var_index < 0 || var_index >= how_many_x_vars)
			{
				std::cerr << "Variable out of range on line " << line_num << "\n";
				return false;
			}

			row[var_index] = value;
		}

		matrix.push_back(row);
	}

	infile.close();

	return true;
}

/// @brief this function fills out the colnames vector with x1, x2, x3, etc
void SolutionState::populate_colnames_array()
{
	for (int i = 0; i < how_many_x_vars; i++)
	{
		colnames.push_back("x" + std::to_string(i + 1));
	}
}

/// @brief print the matrix cover
void SolutionState::printMatrix() // pass using the by reference operator &. this means we don't make a deep copy of the matrix just to print it
{
	if (matrix.empty())
	{
		std::cout << "Matrix is empty\n";
		return;
	}

	int colwidth = how_many_x_vars < 9 ? 3 : 4;

	// --- Print header ---
	std::cout << colnames[1];

	for (int i = 1; i < how_many_x_vars; i++)
	{
		std::cout << std::setw(colwidth) << colnames[i];
	}
	std::cout << "\n";

	// --- Print separator ---

	for (int i = 0; i < how_many_x_vars + 1; i++)
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
