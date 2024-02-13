#include "utils/GFMatrix.hpp"
#include "utils/CLI11.hpp"

#include "Matbuilder/Solver.hpp"
#include "Matbuilder/Parser.hpp"
#include "Matbuilder/Constraints.hpp"

int main(int argc, char** argv)
{
    CLI::App app{"Matbuilder ILP export"};
    
    std::string filename;
    app.add_option("-i", filename, "Input file name")->required();
    std::string matFile;
    app.add_option("-m", matFile, "Matrices file name")->required();
    std::string outfile;
    app.add_option("-o", outfile, "Output file name");
    int seed = 133742;
    app.add_option("--seed", seed, "Program seed");
    bool no_seed = false;
    app.add_flag("--no-seed", no_seed, "Disables seed objective in optimizer");
    std::string format;
    app.add_option("--format", format, "Output format (LP/MPS)")->default_val("LP");

    CLI11_PARSE(app, argc, argv);

    Solver::SolverParams sParams;
    sParams.backtrackMax  = 0;
    sParams.greedyFailMax = 0;
    sParams.rng.seed(seed);
    sParams.randomObjective = !no_seed;

    Parser parser;
    parser.RegisterConstraint("net",        Constraint::Create<ZeroNetConstraint>);
    parser.RegisterConstraint("stratified", Constraint::Create<StratifiedConstraint>);
    parser.RegisterConstraint("propA",      Constraint::Create<PropAConstraint>);
    parser.RegisterConstraint("propA'",     Constraint::Create<PropAprimeConstraint>);
    
    auto program = parser.Parse(filename);
    
    if (program.is_valid)
    {
        std::vector<GFMatrix> matrices;
        {
            std::ifstream matF(matFile);

            if (!matF.is_open())
            {
                std::cerr << "No such file or directory : " << matFile << std::endl;
                return 1;
            }

            while (matF.good())
                matrices.push_back(GFMatrix::From(matF));
            
            if (matrices.size() == 0) // No matrix read, assume to generate m=0
            {
                matrices = std::vector<GFMatrix>(program.s, GFMatrix(0));
            }
            else
            {
                if (matrices.size() != program.s)
                {
                    std::cerr << "Matrix file does not have enough dimensions" << std::endl;
                    return 0;
                }
            
                int maxMat = -32765;
                int minMat =  32765;
                bool sameSize = true;
                for (unsigned int i = 0; i < matrices.size(); i++)
                {
                    sameSize = sameSize && (matrices[i].size() == matrices[0].size());
                    for (unsigned int j = 0; j < matrices[i].size(); i++)
                    {
                        for (unsigned int k = 0; k < matrices[i].size(); k++)
                        {
                            maxMat = std::max(maxMat, matrices[i][j][k]);
                            minMat = std::min(minMat, matrices[i][j][k]);
                        }
                    }
                }

                if (minMat < 0)
                {
                    std::cerr << "Error: generated matrix has negative numbers" << std::endl;
                    return 1;
                }

                if (maxMat >= program.p) 
                {
                    std::cerr << "Error: generated matrix has numbers greater than base" << std::endl;
                    return 1;
                }

                if (!sameSize)
                {
                    std::cerr << "Error: all matrices must have the same size !" << std::endl;
                    return 1;
                }
            }
        }

        sParams.rng.discard(matrices[0].size() * matrices.size());


        Solver solver(sParams, nullptr);
        auto ilp = solver.GetILP(program, matrices, matrices[0].size() + 1);

        std::string ilpstr;
        if (format == "MPS")
            ilpstr = ilp.ToMPS(outfile);
        else
            ilpstr = ilp.ToLP(outfile);
    
        std::ofstream out(outfile);
        out << ilpstr;
    }
}