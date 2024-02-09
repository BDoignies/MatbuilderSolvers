#pragma once

#include "ILP/backends/Backend.hpp"

class GLPKBackend : public Backend
{
public: 
    std::vector<int> SolveILP(const ILP& ilp) const;
};