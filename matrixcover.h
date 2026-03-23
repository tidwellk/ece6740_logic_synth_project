#ifndef MATRIXCOVER_H
#define MATRIXCOVER_H

#include <vector>

class MatrixCover
{

private:
    std::vector<std::vector<int>> data;


public:
    /// @brief default constructor
    MatrixCover();

    /// @brief prints the current matrix cover
    void printMatrix();
    
    /// @brief destructor
    ~MatrixCover();

    /// Rule of five: copy / move
    MatrixCover(const MatrixCover &other);
    MatrixCover(MatrixCover &&other) noexcept;
    MatrixCover &operator=(const MatrixCover &other);
    MatrixCover &operator=(MatrixCover &&other) noexcept;
};

#endif