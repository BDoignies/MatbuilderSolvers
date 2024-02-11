#pragma once

#include <random>

#include "utils/GFMatrix.hpp"
#include "ILP/ILP_def.hpp"

#include "Parser.hpp"

#include "ILP/backends/Backend.hpp"

class Solver
{
public:
    struct SolverParams
    {
        int backtrackMax;
        int greedyFailMax;

        std::mt19937 rng;
        bool randomObjective;
    };

    Solver(const SolverParams& params, Backend* backend) :
        params(params), backend(backend) 
    { }

    ILP GetILP(const MatbuilderProgram& program, const std::vector<GFMatrix>& matrices, int m);

    std::vector<GFMatrix> solve(const MatbuilderProgram& program);

protected:
    SolverParams params;
    Backend* backend;


    Exp GetRandomObjective(ILP& ilp, const std::vector<Var>& variables, int q);

    
};