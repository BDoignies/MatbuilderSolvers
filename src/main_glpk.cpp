#include <iostream>

#include "utils/CLI11.hpp"

#include "ILP/backends/GLPK.hpp"

#include "Matbuilder/Parser.hpp"
#include "Matbuilder/Solver.hpp"


int main(int argc, char** argv)
{
    CLI::App app{"Matbuilder Solver (GLPK)"};
    
    std::string filename;
    app.add_option("-i", filename, "Input filename")->required();
    
    CLI11_PARSE(app, argc, argv);

    Parser parser;
    parser.RegisterConstraint("net",        Constraint::Create<ZeroNetConstraint>);
    parser.RegisterConstraint("stratified", Constraint::Create<StratifiedConstraint>);
    parser.RegisterConstraint("propA",      Constraint::Create<PropAConstraint>);
    parser.RegisterConstraint("propA'",     Constraint::Create<PropAprimeConstraint>);
    
    auto program = parser.Parse(filename);

    if (!program.is_valid)
    {
        std::cout << "Invalid program" << std::endl;
        return -1;
    }

    GLPKBackend backend;
    Solver solver(&backend);
    const auto matrices = solver.solve(program);

    for (const auto& mat : matrices)
    {
        mat.Print();
        std::cout << std::endl;
    }

    return 0;
}