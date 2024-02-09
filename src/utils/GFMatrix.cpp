#include "GFMatrix.hpp"

#include <iostream>
#include <sstream>

int GFMatrix::determinant(const Galois::Field& gf, int override_m) const
{
    const int M = (override_m < 0) ?  m : std::min(m, override_m);

    GFMatrix memory = *this;

    int factorPermut = 1;
    for (int i = 0; i < M; i++)
    {
        int swapi = i;
        while (swapi < M && memory[swapi][i] == 0) swapi ++;

        if (swapi >= M) return 0;

        if (swapi != i)
        {
            factorPermut *= -1;
            for (int k = i; k < M; k++)
            {
                std::swap(memory[swapi][k], memory[i][k]);
            }
        }


        for (int j = i + 1; j < M; j++)
        {
            if (memory[i][j] != 0)
            {
                
                int factor = gf.neg[
                    gf.times(
                        gf.inv[memory[i][i]], 
                               memory[j][i]
                    )
                ];

                for (int k = i; k < m; k++)
                {
                    memory[j][k] = gf.plus(
                        memory[j][k], 
                        gf.times(
                            factor, 
                            memory[i][k]
                        )
                    );
                }
            }
        }
    }

    int res = 1;
    for (int i = 0; i < M; i++)
    {
        res = gf.times(res, memory[i][i]);
    }

    if (factorPermut == 1) return res;
    return gf.neg[res];
}

void GFMatrix::Print() const
{
    for (unsigned int i = 0; i < m; i++)
    {
        for (unsigned int j = 0; j < m; j++)
        {
            std::cout << at(i, j) << " ";
        }
        std::cout << std::endl;
    }
}

std::vector<int> getInts(const std::string& str, int hint=0)
{
    std::vector<int> ints; ints.reserve(hint);
    
    std::stringstream sstr(str);
    int tmp;
    while (sstr >> tmp) { ints.push_back(tmp); } ;

    return ints;
    
}

#include <iostream>
GFMatrix GFMatrix::From(std::istream& iss)
{
    std::string tmpS = "";
    int tmpI;

    while (tmpS.empty() && std::getline(iss, tmpS));
    if    (tmpS.empty()) { return GFMatrix(0); }

    const std::vector<int> firstLine = getInts(tmpS, 0);

    int m = firstLine.size();
    
    GFMatrix rslt(m);
    for (int i = 0; i < m; i++) rslt[0][i] = firstLine[i];
    for (int i = 1; i < m; i++)
    {
        if (!iss.good()) { return GFMatrix(0); }
     
        std::getline(iss, tmpS);
        const std::vector<int> lineInts = getInts(tmpS, m);

        if (lineInts.size() != firstLine.size()) { return GFMatrix(0); }

        for (int j = 0; j < m; j++) rslt[i][j] = lineInts[j];
    }

    return rslt;
}

void GFMatrix::To(std::ostream& oss) const
{
    for (int i = 0; i < m; i++)
    {
        oss << at(i, 0);
        for (int j = 1; j < m; j++) oss << " " << at(i, j);
        oss << '\n';
    }
}