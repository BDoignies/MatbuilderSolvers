#pragma once

#include <functional>
#include "Constraints.hpp"

using CreateConstraint = 
    std::function<Constraint*(const Constraint::Modifier&, std::vector<int>, const std::vector<std::string>&)>;

struct MatbuilderProgram
{
    bool is_valid = false;
    int m = 0, p = 0, s = 0;
    std::vector<Constraint*> constraints;

    ~MatbuilderProgram()
    {
        for (auto c : constraints) delete c;
    }
};

class Parser
{
public:
    void RegisterConstraint(const std::string& name, CreateConstraint create)
    {
        constraints[name] = create;
    }

    MatbuilderProgram Parse(const std::string& filename) const;
private:
    std::map<std::string, CreateConstraint> constraints;
};