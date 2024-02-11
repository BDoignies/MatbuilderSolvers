#pragma once

#include "Backend.hpp"

class CPLEXBackend : public Backend
{
public: 
    CPLEXBackend(Backend::BackendParams& params): Backend(params) {}   

    std::vector<int> SolveILP(const ILP& ilp) const;
};