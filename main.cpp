#include <iostream>
#include <vector>

#include "matrixcover.h"

int main(int argc, char **argv)
{
    std::cout << "main()" << std::endl;

    
    // using new puts it on the heap, but we probably dont need to do that
    //MatrixCover test = new MatrixCover();
    
    // we can put it on the stack by just declaring it, this runs the constructor i think
    MatrixCover test;

    test.printMatrix();

    // when the variable test goes out of scope, the destructor is run
}
