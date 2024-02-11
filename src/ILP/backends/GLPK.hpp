#pragma once

#include "ILP/backends/Backend.hpp"

class GLPKBackend : public Backend
{
public: 
    GLPKBackend(Backend::BackendParams& params): Backend(params) {}

    std::vector<int> SolveILP(const ILP& ilp) const;
};