#include "GLPK.hpp"

#include <glpk.h>
#include <cmath>

// http://most.ccib.rutgers.edu/glpk.pdf
std::vector<int> GLPKBackend::SolveILP(const ILP& ilp) const
{
    const auto& constraints = ilp.GetConstraints();
    
    const auto& cNames  = ilp.GetConstraintNames();
    const auto& vNames  = ilp.GetVariableNames();
    const auto& vBounds = ilp.GetVariablesBounds();

    glp_prob* lp = glp_create_prob();
    glp_set_obj_dir(lp, GLP_MIN);

    // Declare variables
    glp_add_cols(lp, vNames.size());
    for (unsigned int i = 0; i < vNames.size(); i++)
    {
        glp_set_col_name(lp, i + 1, vNames[i].c_str());
        glp_set_col_bnds(lp, i + 1, GLP_DB, vBounds[i].first, vBounds[i].second);
        glp_set_col_kind(lp, i + 1, GLP_IV);
    }

    // Declares constraints

    // First, count number of non-null values in constraint array
    unsigned int sum = 0;
    for (const auto& c : constraints)
        sum += c.coefs.Get().size();

    std::vector<int>    ia(1 + sum, 0);
    std::vector<int>    ja(1 + sum, 0);
    std::vector<double> ra(1 + sum, 0);
    
    unsigned int valueCount = 1;
    glp_add_rows(lp, constraints.size());
    for (unsigned int i = 0; i < constraints.size(); i++)
    {
        glp_set_row_name(lp, i + 1, cNames[i].c_str());
        switch (constraints[i].type)
        {
        case ilp::ComparisonType::EQUAL:         glp_set_row_bnds(lp, i + 1, GLP_FX, constraints[i].rhs, constraints[i].rhs); break;
        
        case ilp::ComparisonType::GREATER:       glp_set_row_bnds(lp, i + 1, GLP_UP, constraints[i].rhs + 1, constraints[i].rhs + 1); break;
        case ilp::ComparisonType::GREATER_EQUAL: glp_set_row_bnds(lp, i + 1, GLP_UP, constraints[i].rhs + 0, constraints[i].rhs + 0); break;
        
        case ilp::ComparisonType::LOWER:         glp_set_row_bnds(lp, i + 1, GLP_LO, constraints[i].rhs - 1, constraints[i].rhs - 1); break;
        case ilp::ComparisonType::LOWER_EQUAL:   glp_set_row_bnds(lp, i + 1, GLP_LO, constraints[i].rhs - 0, constraints[i].rhs - 0); break;

        case ilp::ComparisonType::NOT_EQUAL: 
        default:
            break;
        }

        const auto& coeffs = constraints[i].coefs.Get();
        for (const auto& coeff : coeffs)
        {
            ia[valueCount] = i + 1;
            ja[valueCount] = coeff.first + 1;
            ra[valueCount] = coeff.second;

            valueCount ++;
        }
        
    }
    glp_load_matrix(lp, sum, ia.data(), ja.data(), ra.data());

    // Declares objective
    for (const auto& coeffs : ilp.GetObjective().Get())
        glp_set_obj_coef(lp, coeffs.first + 1, coeffs.second);

    glp_iocp parm;
    glp_init_iocp(&parm); 
    parm.presolve = GLP_ON;
    parm.msg_lev = GLP_MSG_ERR;
    parm.tol_obj = params.tol;
    parm.tm_lim = static_cast<int>(to * 1000);

    int err = glp_intopt(lp, &parm);
    
    if (err != 0) return {};

    std::vector<int> values(vNames.size());

    for (unsigned int i = 0; i < vNames.size(); i++)
    {
        values[i] = round(glp_mip_col_val(lp, i + 1));
    }
        
    glp_delete_prob(lp);
    return values;
}