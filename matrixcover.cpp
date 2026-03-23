#include "matrixcover.h"
#include <iostream>

/// @brief default constructor
MatrixCover::MatrixCover()
{
	std::cout << "MatrixCover: default constructor" << std::endl;
}

MatrixCover::~MatrixCover()
{
	std::cout << "MatrixCover: destructor" << std::endl;
}

MatrixCover::MatrixCover(const MatrixCover &other)
	: data(other.data)
{
	std::cout << "MatrixCover: copy constructor" << std::endl;
}

MatrixCover::MatrixCover(MatrixCover &&other) noexcept
	: data(std::move(other.data))
{
	std::cout << "MatrixCover: move constructor" << std::endl;
}

MatrixCover &MatrixCover::operator=(const MatrixCover &other)
{
	if (this != &other)
		data = other.data;
	std::cout << "MatrixCover: copy assignment" << std::endl;
	return *this;
}

MatrixCover &MatrixCover::operator=(MatrixCover &&other) noexcept
{
	if (this != &other)
		data = std::move(other.data);
	std::cout << "MatrixCover: move assignment" << std::endl;
	return *this;
}

/// @brief print the matrix cover
void MatrixCover::printMatrix()
{
	std::cout << "MatrixCover: rows=" << data.size() << std::endl;
	for (const auto &row : data) {
		for (int v : row)
			std::cout << v << ' ';
		std::cout << std::endl;
	}
}
