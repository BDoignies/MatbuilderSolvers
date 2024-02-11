#pragma once

#include <ILP/ILP_def.hpp>

class Backend
{
public:
    struct BackendParams
    {
    public:
        double tol = 1e-7;       // Tolerance for integers

        uint32_t to = (1 << 31); // Timeout in seconds
        int32_t threads = 0;
    private:
    };

    Backend(BackendParams params) : params(params) { }

    virtual std::vector<int> SolveILP(const ILP& ilp) const = 0;

    virtual ~Backend() {}
protected:
    BackendParams params;
};