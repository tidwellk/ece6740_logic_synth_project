#include "SolutionState.h"

/// @brief default constructor
SolutionState::SolutionState()
	: is_valid_solution(true)
{
	// std::cout << "SolutionState: default constructor" << std::endl;
}

/// @brief This constructor reads a text file into the object.
/// @param filename text file to read
SolutionState::SolutionState(std::string filename)
	: is_valid_solution(true)
{
	if (!readInputFile_isOK(filename))
	{
		std::cout << "could not read input file" << std::endl;
		exit(1);
	}

	// now we know how many columns to have in the matrix
	// std::cout << "we have " << how_many_x_vars << " x variables\n"
			//   << std::endl;

	populate_colnames_array();

	populate_solutions_array();
}

/// @brief destructor
SolutionState::~SolutionState()
{ 
	// std::cout << "SolutionState: destructor" << std::endl;
}

/// @brief rule of 5 stuff auto generated from gpt
/// @param other
SolutionState::SolutionState(const SolutionState &other)
	: matrix(other.matrix),
	  rownames(other.rownames),
	  colnames(other.colnames),
	  current_column_to_colnames_idx(other.current_column_to_colnames_idx),
	  how_many_x_vars(other.how_many_x_vars),
	  current_assignment(other.current_assignment),
	  forced_solution(other.forced_solution),
	  is_valid_solution(other.is_valid_solution)
{
	// std::cout << "SolutionState: copy constructor" << std::endl;
}

SolutionState::SolutionState(SolutionState &&other) noexcept
	: matrix(std::move(other.matrix)),
	  rownames(std::move(other.rownames)),
	  colnames(std::move(other.colnames)),
	  current_column_to_colnames_idx(std::move(other.current_column_to_colnames_idx)),
	  how_many_x_vars(other.how_many_x_vars),
	  current_assignment(std::move(other.current_assignment)),
	  forced_solution(std::move(other.forced_solution)),
	  is_valid_solution(other.is_valid_solution)
{
	// std::cout << "SolutionState: move constructor" << std::endl;
}

SolutionState &SolutionState::operator=(const SolutionState &other)
{
	if (this != &other)
	{
		matrix = other.matrix;
		rownames = other.rownames;
		colnames = other.colnames;
		current_column_to_colnames_idx = other.current_column_to_colnames_idx;
		how_many_x_vars = other.how_many_x_vars;
		forced_solution = other.forced_solution;
		current_assignment = other.current_assignment;
		is_valid_solution = other.is_valid_solution;
	}
	// std::cout << "SolutionState: copy assignment" << std::endl;
	return *this;
}

SolutionState &SolutionState::operator=(SolutionState &&other) noexcept
{
	if (this != &other)
	{
		matrix = std::move(other.matrix);
		rownames = std::move(other.rownames);
		colnames = std::move(other.colnames);
		current_column_to_colnames_idx = std::move(other.current_column_to_colnames_idx);
		how_many_x_vars = other.how_many_x_vars;
		forced_solution = std::move(other.forced_solution);
		current_assignment = std::move(other.current_assignment);
		is_valid_solution = other.is_valid_solution;
	}
	std::cout << "SolutionState: move assignment" << std::endl;
	return *this;
}

void SolutionState::populate_solutions_array()
{
	for (int i = 0; i < how_many_x_vars; i++)
	{
		forced_solution.push_back(UNASSIGNED);
		current_assignment.push_back(UNASSIGNED);
	}
}

char SolutionState::valToChar(Val v)
{
	if (v == ONE)
		return '1';
	if (v == ZERO)
		return '0';
	if (v == UNASSIGNED)
		return ' ';
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

	if (!infile.is_open())
	{
		std::cerr << "Could not open input file: " << filename << "\n";
		return false;
	}

	while (std::getline(infile, line))
	{
		std::stringstream ss(line);

		std::string token;

		while (ss >> token)
		{
			int var_index;
			Val value;

			if (!parse_literal(token, var_index, value))
			{
				std::cerr << "Bad token \"" << token << "\" while scanning " << filename << "\n";
				return false;
			}

			int var_number = var_index + 1;
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
		// std::cout << "found essential row" << std::endl;
		// printSolution();
		// printMatrix();
	}
}

bool SolutionState::operator==(const SolutionState &other) const
{
	return matrix == other.matrix;
}

bool SolutionState::operator!=(const SolutionState &other) const
{
	return !(*this == other);
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

	int colwidth = 0;
	for (int i = 0; i < how_many_x_vars; i++)
	{
		int header_width = (int)colnames[current_column_to_colnames_idx[i]].size();
		if (header_width > colwidth)
		{
			colwidth = header_width;
		}
	}
	colwidth += 1;

	// --- Print header ---
	std::cout << "\n";
	for (int i = 0; i < how_many_x_vars; i++)
	{
		std::cout << std::left << std::setw(colwidth) << colnames[current_column_to_colnames_idx[i]];
	}
	std::cout << std::endl;

	// --- Print separator ---
	for (int i = 0; i < how_many_x_vars * colwidth; i++)
	{
		std::cout << "-";
	}
	std::cout << std::endl;

	// --- Print rows ---
	for (size_t i = 0; i < matrix.size(); i++)
	{

		// print row values
		for (Val v : matrix[i])
		{
			std::cout << std::left << std::setw(colwidth) << valToChar(v);
		}

		// print row name
		std::cout << "\t" << rownames[i];
		std::cout << std::endl;
	}
}

void SolutionState::printSolution()
{
	const int name_width = 16;
	const int value_width = 18;

	std::cout << "Solution cost: " << cost() << "\n"
			  << "--------------------------------------------------" << std::endl;
	std::cout << std::left
			  << std::setw(name_width) << "Variable"
			  << std::setw(value_width) << "Forced Solution"
			  << std::setw(value_width) << "Current Assignment"
			  << std::endl;

	for (uint i = 0; i < forced_solution.size(); i++)
	{
		std::cout << std::left
				  << std::setw(name_width) << colnames[i]
				  << std::setw(value_width) << valToChar(forced_solution[i])
				  << std::setw(value_width) << valToChar(current_assignment[i])
				  << std::endl;
	}
}

std::vector<std::vector<Val>> SolutionState::getMatrix()
{
	return matrix;
}

/// @brief this function mutates the state in place. it will apply these three things:
///		finding essential rows
///		delete dominating rows
///		delete dominated columns
void SolutionState::reduce()
{
	SolutionState a_prime;

	do
	{
		// std::cout << "matrix state at start of reduce() loop" << std::endl;

		// printMatrix();
		// printSolution();

		a_prime = *this;

		remove_essential_rows();
		// printMatrix();
		// printSolution();
		remove_dominating_rows();
		// printMatrix();
		// printSolution();
		remove_dominated_columns();

	} while (!matrix.empty() &&
			 *this != a_prime);
}

/// @brief this function returns the solution cost. It is the number of ones in the forced solution
/// @return
int SolutionState::cost() const
{
	int result = 0;

	for (Val v : forced_solution)
	{
		if (v == ONE)
		{
			result++;
		}
	}

	return result;
}

/// @brief getter for the is_valid boolean
/// @return
bool SolutionState::is_valid()
{
	return is_valid_solution;
}

std::vector<Val> SolutionState::getSolution()
{
	return this->current_assignment;
}

int SolutionState::getHowManyXVars() const
{
	return how_many_x_vars;
}

bool SolutionState::isEmpty()
{
	return matrix.size() == 0;
}

/// @brief this will return the best lower bound estimate.
///			const means it is not allowed to mutate the solver state,
///			but we can make a copy of the matrix and modify that instead.
///
/// @return -1 if we find an infeasible row, otherwise the lower bound cost of the current matrix (current + new lower bound)
int SolutionState::lower_bound() const
{
	// Create a mutable copy of the matrix to work with.
	std::vector<std::vector<Val>> matrix_copy = matrix;

	int matrix_cost = 0;
	while(!matrix_copy.empty())
	{
		// Calculate the cost of implementing each row as part of the solution.
		int min_row = -1;
		int min_row_cost = -1;
		for (int i = 0; i < (int)matrix_copy.size(); i++) // size is the number of rows
		{
			int current_row_cost = 0;
			for (Val column_entry : matrix_copy[i])
			{
				// Skip the row if it contains a 0 since it cannot be part of the solution.
				if (column_entry == ZERO)
				{
					current_row_cost = -1;
					break;
				}
				// Otherwise add its cost if it's implemented at the row.
				else if (column_entry == ONE)
				{
					current_row_cost++;
				}
			}
			if (min_row_cost == -1 || (current_row_cost != -1 && current_row_cost < min_row_cost))
			{
				min_row_cost = current_row_cost;
				min_row = i;
			}
		}

		// If all remaining rows have zeros we can stop here.
		if(min_row_cost == -1)
		{
			min_row_cost = 0;
			break;
		}

		// Remember the columns of this row covers before erasing it.
		std::vector<int> one_entries;
		for (int j = 0; j < (int)matrix_copy[min_row].size(); j++)
			if (matrix_copy[min_row][j] == ONE) one_entries.push_back(j);

		// Now we've identified the row with the lowest cost to implement. First verify feasibility,
		if(one_entries.empty())
		{
			return -1; // being empty means all entries were DC.
		}

		// then add its cost,
		matrix_cost += min_row_cost;

		// Remove it from the mutable copy of the original matrix,
		matrix_copy.erase(matrix_copy.begin() + min_row);

		// Remove any row that has a 1 in the same column as the implemented row (start at the end and go backwards since we are removing rows to avoid skipping any).
		for (int i = matrix_copy.size() - 1; i >= 0; i--) 
		{
			for (int entry : one_entries)
			{
				if (matrix_copy[i][entry] == ONE)
				{
					matrix_copy.erase(matrix_copy.begin() + i);
					break;
				}
			}
		}

		// and repeat until the matrix is empty or we find an infeasible row.
	}

	// The current minimal cost is the cost of the currest solution plus the cost of our lower bound.
	return this->cost() + matrix_cost;
}

/// @brief pairwise comparison of rows
/// If row i dominates row j, this function removes row i.
/// Checks every pair of rows and adds the dominating ones to a set.
/// Then removes all the dominating rows at the end.
void SolutionState::remove_dominating_rows()
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
				// we want to check if i dominates j and can be removed
				Val row_i_el = row_i[elementwise];
				Val row_j_el = row_j[elementwise];

				// Possible combinations of row_i_el and row_j_el:
				//                 i: - - - 0 0 0 1 1 1
				//                 j: - 0 1 - 0 1 - 0 1
				//     Dominates?     1 0 0 1 1 0 1 0 1 (Boolean indicating if i dominates j in the above combination)
				// i dominates j if for each column, the two are equal or j is a DC.
				if (row_i_el == row_j_el || row_j_el == DC)
				{
					continue;
				}
				else
				{
					i_dominates_j = false;
					break;
				}
			}

			// elementwise check is done.
			if (i_dominates_j)
			{
				rows_to_remove.emplace(outer);
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
	// std::set<int, std::greater<int>> cols_to_remove;

	if (matrix.empty() || matrix[0].empty())
	{
		return;
	}

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
				// cols_to_remove.emplace(colB);
				// reduce() loops over and over, so we can delete one column here and then return. reduce will run the loop again
				assign_a_variable(colB, ZERO, NOT_ESSENTIAL);

				return;
			}
		}
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
			return assign_a_variable(colNumber, assignedVal, ESSENTIAL);
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

/// @brief Description from gpt:
/// You need two different operations
/// 1. Forced assignment (essential rows)

/// update solution
/// remove rows with matching value
/// remove column

/// 2. Column dominance

/// DO NOT commit to solution yet
/// remove rows where column == 0
/// remove the column
/// @param current_column_number
/// @param val_to_assign
/// @param isForcedEssential
/// @return
bool SolutionState::assign_a_variable(int current_column_number, Val val_to_assign, bool isForcedEssential)
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
	// turns out that we need to
	if (isForcedEssential)
	{
		if (forced_solution[actual_var_column] == UNASSIGNED ||
			forced_solution[actual_var_column] == DC ||
			forced_solution[actual_var_column] == val_to_assign)
		{
			forced_solution[actual_var_column] = val_to_assign;
		}
		else
		{
			std::cout << "contradiction in solution" << std::endl;
			is_valid_solution = false;
			return false;
		}
	}

	if (current_assignment[actual_var_column] == UNASSIGNED ||
		current_assignment[actual_var_column] == DC ||
		current_assignment[actual_var_column] == val_to_assign)
	{
		current_assignment[actual_var_column] = val_to_assign;
	}
	else
	{
		std::cout << "contradiction in current assignment" << std::endl;
		is_valid_solution = false;
		return false;
	}

	remove_rows_with_same_val(current_column_number, val_to_assign);

	remove_column(current_column_number);

	return true;
}

/// @brief Chooses the branching variable by scoring columns based on short-row coverage,
/// binate occurrence, polarity balance, and total appearances in the current matrix.
/// @return column number of the variable to assign, or -1 if the matrix is empty (no variable to choose).
int SolutionState::choose_var()
{
	// If the matrix is empty, return -1 to indicate no variable to choose since 0 would be a valid column index.
	if (matrix.empty() || matrix[0].empty())
	{
		return -1;
	}

	// Scoring variables based on the selection heuristic to choose the best branching variable.
	int best_col = 0;
	int best_score = -1;
	int best_short_row_score = -1;
	int best_binate_bonus = -1;
	int best_balance_bonus = -1;
	int best_total_appearances = -1;

	// Iterate through each column to calculate its score based on the defined heuristic.
	for (int col = 0; col < (int)matrix[0].size(); col++)
	{
		int short_row_score = 0;
		int pos_count = 0;
		int neg_count = 0;

		// Evaluate the column by iterating through each row and counting the occurrences of 1s, 0s, and DCs.
		for (const auto &row : matrix)
		{
			// Skip DC entries since they do not contribute to the score and do not affect dominance or essentiality.
			if (row[col] == DC)
			{
				continue;
			}

			// Count the number of literals in the row (non-DC entries) to determine if it's a short row.
			int row_literals = 0;
			for (Val entry : row)
			{
				if (entry != DC)
				{
					row_literals++;
				}
			}

			// Short rows are more valuable to cover, so we give them a higher score. 
			// This encourages the algorithm to cover short rows early, which can lead to faster reductions in the matrix size.
			// The scoring is exponential for very short rows to prioritize them even more, while longer rows get a smaller incremental score.
			// The scoring criteria here is purely heuristic and can be adjusted based on testing and performance observations.
			if (row_literals <= 2)
			{
				short_row_score += 100;
			}
			else if (row_literals == 3)
			{
				short_row_score += 30;
			}
			else if (row_literals == 4)
			{
				short_row_score += 10;
			}
			else
			{
				short_row_score += 1;
			}

			// Count the number of positive and negative occurrences of the variable in the column.
			if (row[col] == ONE)
			{
				pos_count++;
			}
			else if (row[col] == ZERO)
			{
				neg_count++;
			}
		}

		// Calculate the total score for the column based on the short row score, binate bonus, balance bonus, and total appearances.
		int total_appearances = pos_count + neg_count;
		int binate_bonus = (pos_count > 0 && neg_count > 0) ? 50 : 0;
		int balance_bonus = std::min(pos_count, neg_count) * 5;
		int score = short_row_score + binate_bonus + balance_bonus + total_appearances;

		// Update the best column if the current column has a higher score than the best score found so far.
		// The tie-breaking criteria prioritize columns that cover more short rows, have a binate occurrence, 
		// and have a better balance of positive and negative literals in that order, followed by total appearances as a final tiebreaker.
		if (score > best_score ||
			(score == best_score && short_row_score > best_short_row_score) ||
			(score == best_score && short_row_score == best_short_row_score && binate_bonus > best_binate_bonus) ||
			(score == best_score && short_row_score == best_short_row_score && binate_bonus == best_binate_bonus && balance_bonus > best_balance_bonus) ||
			(score == best_score && short_row_score == best_short_row_score && binate_bonus == best_binate_bonus && balance_bonus == best_balance_bonus && total_appearances > best_total_appearances))
		{
			best_col = col;
			best_score = score;
			best_short_row_score = short_row_score;
			best_binate_bonus = binate_bonus;
			best_balance_bonus = balance_bonus;
			best_total_appearances = total_appearances;
		}
	}

	return best_col;
}
