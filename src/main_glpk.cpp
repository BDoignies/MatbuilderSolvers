#include "utils/main_base.hpp"
#include "ILP/backends/GLPK.hpp"

int main(int argc, char** argv)
{
    matbuilder_solve<GLPKBackend>(argc, argv);    
}