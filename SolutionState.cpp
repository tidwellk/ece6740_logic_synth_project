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

	populate_solutions_array();
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

void SolutionState::populate_solutions_array()
{
	for (int i = 0; i < how_many_x_vars; i++)
	{
		solution.push_back(UNASSIGNED);
	}
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

	for (uint i = 0; i < rownames.size(); i++)
	{
		// current_row_to_rownames_idx.push_back(i);
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
		current_column_to_colnames_idx.push_back(i);
	}
}

void SolutionState::remove_essential_rows()
{
	while (find_essential_row() == true)
	{
		std::cout << "found essential row" << std::endl;
		printSolution();
		printMatrix();
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

	how_many_x_vars = (int)current_column_to_colnames_idx.size();

	int colwidth = how_many_x_vars < 9 ? 3 : 4;

	// --- Print header ---
	std::cout << "\n" << colnames[current_column_to_colnames_idx[0]];

	for (int i = 1; i < how_many_x_vars; i++)
	{
		std::cout << std::setw(colwidth) << colnames[current_column_to_colnames_idx[i]];
	}
	std::cout << std::endl;

	// --- Print separator ---
	for (int i = 0; i < how_many_x_vars + 2; i++)
	{
		std::cout << "--";
	}
	std::cout << std::endl;

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
		std::cout << std::endl;
	}
}

void SolutionState::printSolution()
{
	std::cout << "Solution\n"
			  << "------------" << std::endl;
	for (uint i = 0; i < solution.size(); i++)
	{
		std::cout << colnames[i] << "\t" << valToChar(solution[i]) << std::endl;
	}
}

/// @brief this function mutates the state in place. it will apply these three things:
///		finding essential rows
///		delete dominating rows
///		delete dominated columns
void SolutionState::reduce()
{
	remove_essential_rows();

	remove_dominated_rows();
	printMatrix();
	remove_dominated_columns();
}

/// @brief pairwise comparison of rows
/// i think i wrote this backwards though, the one that gets removed is the dominator i guess.
/// it matches what is in the UCP-BCP.pdf slides and behaves accordingly even though i named
/// the boolean backwards
void SolutionState::remove_dominated_rows()
{
	// this makes a set which iterates in descending order
	std::set<int, std::greater<int>> rows_to_remove;

	// need to compare row by row
	// outer loop takes one row and inner loop changes which one the outer one is being compared to
	for (uint outer = 0; outer < matrix.size(); outer++)
	{
		std::vector<Val> row_i = matrix[outer];

		for (uint inner = 0; inner < matrix.size(); inner++)
		{
			if (outer == inner)
			{
				continue;
			}

			std::vector<Val> row_j = matrix[inner];

			bool i_dominates_j = true;

			// ok now we have two rows to compare, row_i and row_j
			// need to compare them elementwise
			for (uint elementwise = 0; elementwise < row_i.size(); elementwise++)
			{
				// we want to check if i dominates j, if so we remove j
				Val row_i_el = row_i[elementwise];

				// if the element in i is dont care, then we skip checking j
				if (row_i_el == DC)
				{
					continue;
				}

				Val row_j_el = row_j[elementwise];

				if (row_i_el != row_j_el)
				{
					i_dominates_j = false;
					break;
				}
			}

			// elementwise check is done.
			if (i_dominates_j)
			{
				rows_to_remove.emplace(inner);
			}
		}
	}

	// remove each row, starting from the bottom and working up. the set is iterated in reverse order
	for (const int pending_row_del : rows_to_remove)
	{
		remove_row_number(pending_row_del);
	}
}

/// @brief pairwise checks of every column
void SolutionState::remove_dominated_columns()
{
	std::set<int, std::greater<int>> cols_to_remove;

	uint rowCount = matrix.size();
	uint colCount = matrix[0].size();

	for (uint colA = 0; colA < colCount; colA++)
	{
		for (uint colB = 0; colB < colCount; colB++)
		{
			if (colA == colB)
			{
				continue;
			}

			bool a_dominates_b = true;

			// need to check every row from colA and colB
			/*
			the bad combinations are:

			a=−, b=1
			a=0, b=1
			a=0, b=−

			If any row has one of those, then a does not dominate b
			*/
			for (uint rowIdx = 0; rowIdx < rowCount; rowIdx++)
			{
				Val row_a_el = matrix[rowIdx][colA];
				Val row_b_el = matrix[rowIdx][colB];

				if (row_a_el == DC &&
					row_b_el == ONE)
				{
					a_dominates_b = false;
					break;
				}

				if (row_a_el == ZERO &&
					row_b_el == ONE)
				{
					a_dominates_b = false;
					break;
				}

				if (row_a_el == ZERO &&
					row_b_el == DC)
				{
					a_dominates_b = false;
					break;
				}
			}

			if (a_dominates_b)
			{
				cols_to_remove.emplace(colB);

				// TODO check weights
			}
		}
	}

	// now we have a set of columns to remove
	// each column we remove gets assigned that variable to 0
	for (int column_to_del : cols_to_remove)
	{
		assign_a_variable(column_to_del, ZERO);
	}
}

bool SolutionState::find_essential_row()
{
	for (uint i = 0; i < matrix.size(); i++)
	{
		std::vector<Val> row = matrix[i];

		int howManyAssigned = 0;
		int colNumber = -1;
		Val assignedVal = DC;
		for (uint j = 0; j < row.size(); j++)
		{
			Val currentVal = row[j];

			if (currentVal != DC)
			{
				assignedVal = currentVal;
				howManyAssigned++;
				colNumber = j;
			}
		}

		if (howManyAssigned == 1)
		{
			assign_a_variable(colNumber, assignedVal);

			return true;
		}
	}
	return false;
}

/// @brief this is called when find_essential_row has found one.
///		   it needs to remove any row that has the same x value assigned
///		   to it because it has been covered
/// @param column_to_check
/// @param value
void SolutionState::remove_rows_with_same_val(int column_to_check, Val value)
{
	for (int rownum = (int)matrix.size() - 1; rownum >= 0; rownum--)
	{
		Val testval = matrix[rownum][column_to_check];
		if (testval == value)
		{
			remove_row_number(rownum);
		}
	}
}

void SolutionState::remove_row_number(int rownum)
{
	// remove the row from rownames
	rownames.erase(rownames.begin() + rownum);

	// remove the row from matrix
	matrix.erase(matrix.begin() + rownum);
}

/// @brief this is called when a variable has been assigned, we need to remove the column from
///        each row vector
/// @param column_number
void SolutionState::remove_column(int column_number)
{
	for (auto &row : matrix)
	{
		row.erase(row.begin() + column_number);
	}

	// we need to update the current_column translation array
	current_column_to_colnames_idx.erase(
		current_column_to_colnames_idx.begin() + column_number);
}

bool SolutionState::assign_a_variable(int current_column_number, Val val_to_assign)
{
	int actual_var_column = current_column_to_colnames_idx[current_column_number];

	// check for duplicates first
	// solution.push_back(Assignment(assigned_var_column, assignedVal));
	/*
	A row ai of A is essential when there exists exactly one j such that aij is
	not equal to ’-’.
	This cooresponds to clause consisting of a single literal.
	If the literal is xj (i.e., aij = 1), the variable is essential.
	If the literal is xj' (i.e., aij = 0), the variable is unacceptable.
	The matrix A is reduced with respect to the essential literal.
	This variable is set to value of literal, column is removed, and any row
	where variable has same value is removed.
	*/
	if (solution[actual_var_column] == UNASSIGNED ||
		solution[actual_var_column] == DC ||
		solution[actual_var_column] == val_to_assign)
	{
		solution[actual_var_column] = val_to_assign;
	}
	else
	{
		std::cout << "contradiction in solution" << std::endl;
		exit(1);
	}

	remove_rows_with_same_val(current_column_number, val_to_assign);

	remove_column(current_column_number);

	printMatrix();

	return true;
}
