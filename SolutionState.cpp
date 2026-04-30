#include "SolutionState.h"

/// @brief Constructs an empty solution state.
SolutionState::SolutionState()
	: is_valid_solution(true)
{
	// std::cout << "SolutionState: default constructor" << std::endl;
}

/// @brief Constructs a solution state by reading a covering problem from a file.
/// @param filename Input covering problem file.
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

/// @brief Destroys the solution state.
SolutionState::~SolutionState()
{ 
	// std::cout << "SolutionState: destructor" << std::endl;
}

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

/// @brief Populates column names and the current-to-original column index map.
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

/// @brief Prints the current matrix cover in tabular form.
void SolutionState::printMatrix()
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

/// @brief Returns the current matrix representation.
std::vector<std::vector<Val>> SolutionState::getMatrix()
{
	return matrix;
}

/// @brief Applies all matrix reductions until the state no longer changes.
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

/// @brief Returns the solution cost as the number of forced ONE assignments.
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

/// @brief Returns whether the state is still logically consistent.
bool SolutionState::is_valid()
{
	return is_valid_solution;
}

/// @brief Returns the current assignment vector.
std::vector<Val> SolutionState::getSolution()
{
	return this->current_assignment;
}

/// @brief Returns the total number of variables in the original problem.
int SolutionState::getHowManyXVars() const
{
	return how_many_x_vars;
}

/// @brief Returns true when no uncovered rows remain in the matrix.
bool SolutionState::isEmpty()
{
	return matrix.size() == 0;
}

/// @brief Computes a greedy lower bound on the remaining solution cost.
/// @return The current forced cost plus the greedy lower bound, or -1 if the
///         remaining matrix contains an infeasible all-DC row.
int SolutionState::lower_bound() const
{
	// Create a mutable copy of the matrix to work with.
	std::vector<std::vector<Val>> matrix_copy = matrix;

	int matrix_cost = 0;
	while(!matrix_copy.empty())
	{
		int min_row = -1;
		int min_row_ones = -1;
		for (int i = 0; i < (int)matrix_copy.size(); i++)
		{
			int current_row_ones = 0;
			for (Val column_entry : matrix_copy[i])
			{
				if (column_entry == ZERO)
				{
					current_row_ones = -1;
					break;
				}
				else if (column_entry == ONE)
				{
					current_row_ones++;
				}
			}
			if (min_row_ones == -1 || (current_row_ones != -1 && current_row_ones < min_row_ones))
			{
				min_row_ones = current_row_ones;
				min_row = i;
			}
		}

		if(min_row_ones == -1)
		{
			min_row_ones = 0;
			break;
		}

		std::vector<int> one_entries;
		for (int j = 0; j < (int)matrix_copy[min_row].size(); j++)
			if (matrix_copy[min_row][j] == ONE) one_entries.push_back(j);

		if(one_entries.empty())
		{
			return -1;
		}

		matrix_cost += (bool) min_row_ones;

		matrix_copy.erase(matrix_copy.begin() + min_row);

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
	}

	return this->cost() + matrix_cost;
}

/// @brief Removes rows dominated by other rows in the current matrix.
void SolutionState::remove_dominating_rows()
{
	// this makes a set which iterates in descending order
	std::set<int, std::greater<int>> rows_to_remove;

	// need to compare row by row
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

			for (uint elementwise = 0; elementwise < row_i.size(); elementwise++)
			{
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

/// @brief Removes columns dominated by other columns in the current matrix.
void SolutionState::remove_dominated_columns()
{
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
				assign_a_variable(colB, ZERO, NOT_ESSENTIAL);

				return;
			}
		}
	}
}

/// @brief Finds and processes one essential row, if one exists.
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

/// @brief Removes all rows covered by an assignment to a specific column value.
/// @param column_to_check Column index in the current reduced matrix.
/// @param value Assigned value that covers the row.
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

/// @brief Removes a row and its name entry from the current matrix state.
/// @param rownum Row index to erase.
void SolutionState::remove_row_number(int rownum)
{
	rownames.erase(rownames.begin() + rownum);
	matrix.erase(matrix.begin() + rownum);
}

/// @brief Removes a column from the matrix after that variable has been assigned.
/// @param column_number Column index in the current reduced matrix.
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

/// @brief Assigns a value to a column, removes covered rows, and removes the column.
/// @param current_column_number Column index in the current reduced matrix.
/// @param val_to_assign Value to assign (ONE or ZERO).
/// @param isForcedEssential True when the assignment comes from an essential row.
/// @return False if the assignment contradicts an existing assignment; true otherwise.
bool SolutionState::assign_a_variable(int current_column_number, Val val_to_assign, bool isForcedEssential)
{
	int actual_var_column = current_column_to_colnames_idx[current_column_number];
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

/// @brief Chooses the branching variable using the repository's heuristic score.
/// @return Reduced-matrix column index of the chosen variable, or -1 if the matrix is empty.
int SolutionState::choose_var()
{
	if (matrix.empty() || matrix[0].empty())
	{
		return -1;
	}

	int best_col = 0;
	int best_score = -1;
	int best_short_row_score = -1;
	int best_binate_bonus = -1;
	int best_balance_bonus = -1;
	int best_total_appearances = -1;

	for (int col = 0; col < (int)matrix[0].size(); col++)
	{
		int short_row_score = 0;
		int pos_count = 0;
		int neg_count = 0;

		for (const auto &row : matrix)
		{
			if (row[col] == DC)
			{
				continue;
			}

			int row_literals = 0;
			for (Val entry : row)
			{
				if (entry != DC)
				{
					row_literals++;
				}
			}

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

			if (row[col] == ONE)
			{
				pos_count++;
			}
			else if (row[col] == ZERO)
			{
				neg_count++;
			}
		}

		int total_appearances = pos_count + neg_count;
		int binate_bonus = (pos_count > 0 && neg_count > 0) ? 50 : 0;
		int balance_bonus = std::min(pos_count, neg_count) * 5;
		int score = short_row_score + binate_bonus + balance_bonus + total_appearances;

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
