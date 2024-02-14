#include "Solver.hpp"

Exp Solver::GetRandomObjective(ILP& ilp, const std::vector<Var>& variables, int q)
{
    Exp obj = ilp.CreateOperation();

    if (!params.randomObjective) return obj;

    std::uniform_int_distribution<int> dist(0, q - 1);
    for (unsigned int i = 0; i < variables.size(); i++)
    {
        // Replace min |A - B|
        // By:
        //      min X'
        //      st A - B <= X'
        //         B - A <= X'  

        int c = dist(params.rng);

        Var xp = ilp.CreateVariable("O");
        obj = obj + xp;

        ilp.AddConstraint("OBJL", variables[i] - c <= xp);
        ilp.AddConstraint("OBJH", c - variables[i] <= xp);
    }

    return obj;
}

ILP Solver::GetILP(const MatbuilderProgram& program, const std::vector<GFMatrix>& matrices, int m)
{
    const Galois::Field gf(program.p);

    ILP ilp;
    Exp obj = ilp.CreateOperation();

    
    const std::vector<Var> variables = ilp.CreateVariables("x", m * program.s, 0, gf.q - 1);
    for (const auto& constraint : program.constraints)
        constraint->Apply(matrices, gf, m, ilp, variables.data(), obj);

    // According to paper
    // obj = (program.s * program.m * (program.p - 1)) * obj;
    // According to code (often, 1000 >> s * m * p - 1)
    obj = 1000 * obj;

    int minWeight = 1;
    for (unsigned int i = 0; i < program.constraints.size(); i++)
    {
        if (program.constraints[i]->GetModifier().weak)
        {
            minWeight += std::min(minWeight, program.constraints[i]->GetModifier().weakWeight);
        }
    }

    ilp.SetObjective(obj + GetRandomObjective(ilp, variables, program.p));

    return ilp;
}

std::vector<GFMatrix> Solver::solve(const MatbuilderProgram& program)
{
    std::vector<GFMatrix> result;

    bool failed = true;
    int greedyFails = 0;
    
    while (failed && greedyFails < params.greedyFailMax)
    {
        result = std::vector<GFMatrix>(program.s, GFMatrix(program.m));
     
        int backtrackCounts = 0;
        for (int m = 1; m <= program.m; m++)
        {
            ILP ilp = GetILP(program, result, m);
            
            auto values = backend->SolveILP(ilp);

            // Backtracking needed, no solution found ! 
            if (values.size() == 0)
            {
                backtrackCounts ++;
                if (backtrackCounts > params.backtrackMax) 
                {
                    backtrackCounts = 0;
                    greedyFails ++;
                    break;
                }
                
                     if (m >= 3) m -= 2;
                else if (m == 2) m  = 0;
                else if (m == 1) m  = 0;
        
                continue;
            }
            
            for (unsigned int k = 0; k < result.size(); k++)
            {
                for (int i = 0; i < m; i++)
                {
                    result[k][i][m - 1] = values[i + k * m];
                }
            }
        }

        failed = false;
    }
    
    if (failed)
    {
        return result;
    }

    return result;
}