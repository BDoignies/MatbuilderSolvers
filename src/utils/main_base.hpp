#pragma once

#include <iostream>

#include "ILP/backends/Backend.hpp"
#include "Matbuilder/Solver.hpp"
#include "CLI11.hpp"

template <class BackendType>
int matbuilder_solve(const char* backendName, int argc, char** argv)
{
    CLI::App app{"Matbuilder Solver (" + std::string(backendName) + ")"};
    
    std::string filename;
    app.add_option("-i", filename, "Input file name")->required();
    std::string outfile;
    app.add_option("-o", outfile, "Output file name");
    int fullSize = -1;
    app.add_option("-m", fullSize, "Override file matrix size");
    int nbTrials = 50;
    app.add_option("-n,--nbTrials", nbTrials, "Number of tentative (def:50)");
    int nbBacktrack = 15;
    app.add_option("--nbBacktrack", nbBacktrack, "Number of backtracks before starting again (def:15)");
    int s = -1;
    app.add_option("-s", s, "Override file number of dimensions");
    bool check = false;
    app.add_flag("--check", check, "Check properties (unit test)");
    int b = -1;
    app.add_option("-b", b, "Override matrices basis");
    int nbThreads = 0;
    app.add_option("--threads", nbThreads, "Number of threads to use (def: all avalaible)");
    double tolerance_ratio = 0.01;
    app.add_option("--tolerance", tolerance_ratio, "Error tolerance on objective value (def: 0.01)");
    double timeout = pow(10.,10.);
    app.add_option("-t, --timeout", timeout, "Maximum time for each cplex solve (def: 10^10 s)");
    int seed = 133742;
    app.add_option("--seed", seed, "Program seed");
    bool no_seed = false;
    app.add_flag("--no-seed", no_seed, "Disables seed objective in optimizer");
    bool header = false;
    app.add_flag("--header", header, "Writes profile as comments at the beginning of matrix file");
    
    CLI11_PARSE(app, argc, argv);
    
    Parser parser;
    parser.RegisterConstraint("net",        Constraint::Create<ZeroNetConstraint>);
    parser.RegisterConstraint("stratified", Constraint::Create<StratifiedConstraint>);
    parser.RegisterConstraint("propA",      Constraint::Create<PropAConstraint>);
    parser.RegisterConstraint("propA'",     Constraint::Create<PropAprimeConstraint>);
    
    auto program = parser.Parse(filename);
    if (s        > 0) program.s = s;
    if (b        > 0) program.p = b;
    if (fullSize > 0) program.m = fullSize;

    Backend::BackendParams bParams;
    bParams.threads = nbThreads;
    bParams.tol     = tolerance_ratio;
    bParams.to      = timeout;
    
    Solver::SolverParams sParams;
    sParams.backtrackMax  = nbBacktrack;
    sParams.greedyFailMax = nbTrials;
    sParams.rng.seed(seed);
    sParams.randomObjective = !no_seed;

    if (!program.is_valid)
    {
        std::cout << "Invalid program" << std::endl;
        return -1;
    }

    BackendType backend(bParams);
    Solver solver(sParams, &backend);
    const auto matrices = solver.solve(program);

    std::ofstream fileOut(outfile);
    std::ostream* out = &std::cout;
    if (fileOut.is_open())
        out = &fileOut;

    if (header)
    {
        std::string tmp;
        std::ifstream programTmp(filename.c_str());
        while (std::getline(programTmp, tmp)) (*out) << "# " << tmp << '\n';
        (*out) << '\n';
    }

    for (const auto& mat : matrices)
    {
        mat.To(*out);
        *out << '\n';
    }

    return 0;
}