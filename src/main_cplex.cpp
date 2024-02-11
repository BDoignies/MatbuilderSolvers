#include <iostream>

#include "ILP/backends/CPLEX.hpp"
#include "utils/main_base.hpp"

int main(int argc, char** argv)
{
    matbuilder_solve<CPLEXBackend>(argc, argv);    
}