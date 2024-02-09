#pragma once

#include <random>

#include "utils/GFMatrix.hpp"
#include "ILP/ILP_def.hpp"

#include "Parser.hpp"

#include "ILP/backends/Backend.hpp"

class Solver
{
public:
    Solver(Backend* backend) : backend(backend) { setSeed(std::random_device{}(), 0); }

    void setRandomObjective(bool rand) { randomObjective = rand; }
    void setSeed(uint64_t seed, uint32_t burnIn) { rng.seed(seed); rng.discard(burnIn); }

    ILP GetILP(const MatbuilderProgram& program, const std::vector<GFMatrix>& matrices, int m);

    std::vector<GFMatrix> solve(const MatbuilderProgram& program);
protected:
    int  backtrackMax = 100;
    int greedyFailMax = 100;
    
    std::mt19937 rng;
    bool randomObjective = true;

    Exp GetRandomObjective(ILP& ilp, const std::vector<Var>& variables, int q);

    
    Backend* backend;
};