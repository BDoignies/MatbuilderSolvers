#include "CPLEX.hpp"

#include <ilcplex/ilocplex.h>

std::vector<int> CPLEXBackend::SolveILP(const ILP& ilp) const
{
    const auto& constraints = ilp.GetConstraints();
    
    const auto& cNames  = ilp.GetConstraintNames();
    const auto& vNames  = ilp.GetVariableNames();
    const auto& vBounds = ilp.GetVariablesBounds();

    IloEnv env;
    IloModel model(env);
    IloNumVarArray vars(env);
    IloNumVarArray ks(env);
    IloConstraintArray c(env);
    IloNumExpr weakObj(env);
    IloNumVarArray weakVars(env);

    for (unsigned int i = 0; i < vNames.size(); i++)
    {
        const auto& name = vNames[i];
        const auto& bounds = vBounds[i];

        IloInt low  = bounds.first;
        IloInt high = bounds.second;
        if (bounds.second == INT32_MAX) high = IloIntMax;
         
        IloIntVar x(env, bounds.first, bounds.second);
        x.setName(name.c_str());
        vars.add(x);
    }

    for (unsigned int i = 0; i < constraints.size(); i++)
    {
        IloNumExpr exp(env);
        exp.setName(cNames[i].c_str());
        std::vector<std::pair<uint32_t, int32_t>> matrix = constraints[i].coefs.Get();
            
        for (unsigned int i = 0; i < matrix.size(); i++)
            exp += matrix[i].second * vars[matrix[i].first];
        
        switch (constraints[i].type)
        {
        case ilp::ComparisonType::EQUAL:         c.add(exp == constraints[i].rhs); break;
        
        case ilp::ComparisonType::GREATER:       c.add(exp >  constraints[i].rhs); break;
        case ilp::ComparisonType::GREATER_EQUAL: c.add(exp >= constraints[i].rhs); break;
        
        case ilp::ComparisonType::LOWER:         c.add(exp <  constraints[i].rhs); break;
        case ilp::ComparisonType::LOWER_EQUAL:   c.add(exp <= constraints[i].rhs); break;

        case ilp::ComparisonType::NOT_EQUAL: 
        default:
            break;
        }
    }   

    IloNumExpr obj(env);
    for (const auto& coeffs : ilp.GetObjective().Get())
        obj += coeffs.second * vars[coeffs.first];
    
    model.add(IloMinimize(env, obj));
    model.add(c);

    IloCplex cplex(model);
    cplex.setParam(IloCplex::Param::ParamDisplay, 0);
    cplex.setParam(IloCplex::Param::Threads, params.threads);
    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, params.tol);
    cplex.setParam(IloCplex::TiLim, params.to);

    bool success = cplex.solve();

    if (!success) return {};

    IloNumArray vals(env);
    cplex.getValues(vals, vars);

    std::vector<int> values(vNames.size());
    for (unsigned int i = 0; i < vNames.size(); i++)
    {
        values[i] = IloRound(vals[i]);
    }
    return values;  
}