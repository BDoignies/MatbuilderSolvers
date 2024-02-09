#pragma once

#include "Backend.hpp"

class CPLEXBackend : public Backend
{
public: 
    std::vector<int> SolveILP(const ILP& ilp) const;
};