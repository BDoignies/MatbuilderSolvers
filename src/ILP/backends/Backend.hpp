#pragma once

#include <ILP/ILP_def.hpp>

class Backend
{
public:
    Backend() { }

    virtual std::vector<int> SolveILP(const ILP& ilp) const = 0;

    virtual ~Backend() {}
protected:
};