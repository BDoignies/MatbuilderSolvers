#include "Constraints.hpp"

std::vector<int> constraintMkSubdets(
    int currentM, 
    const std::vector<GFMatrix>& matrices, 
    const std::vector<int>& k, 
    const Galois::Field& gf)
{
    const int m = currentM;

    GFMatrix matrix(m - 1);
    std::vector<int> subdets(m, 0); 
    std::fill(subdets.begin(), subdets.end(), 0);

    int indMat = 0;
    int prevlines = 0;

    for (int row = 1; row < m; row++)
    {
        while (row - prevlines >= k[indMat]) 
        {
            prevlines += k[indMat];
            indMat ++;
        }

        for (int col = 0; col < m - 1; col++)
        {
            if (row - prevlines >= m - 1)
                matrix[row - 1][col] = 0;
            else
                matrix[row - 1][col] = matrices[indMat][row - prevlines][col];
        }
    }

    long long int factor = (int) pow(-1., m + 1);

    indMat = 0;
    prevlines = 0;
    for (int row = 0; row < m; row++)
    {
        int idet = factor * matrix.determinant(gf, currentM);
        subdets[row] = idet;
        factor *= -1;

        while (row - prevlines == k[indMat])
        {
            prevlines += k[indMat];
            indMat += 1;
        }

        if (row != m-1) 
        {
            for (int col = 0; col < m - 1; col++) 
            {
                matrix[row][col] = matrices[indMat][row - prevlines][col];
            }   
        }
    }

    for (auto& w : subdets){
        w = (w + gf.q) % gf.q;
    }

    return subdets;
}

void positions2k(const std::vector<int>& positions, std::vector<int>& k, int& unblance, int max)
{
    if (positions.empty())
    {
        k[0] = max + 1;
        unblance = 0;

        return;
    }

    k[0] = positions[0];
    for (int i = 1; i < positions.size(); i++)
    {
        k[i] = positions[i] - positions[i - 1] - 1;
    }
    k[positions.size()] = max - positions.back();

    int low = max;
    int high = 0;

    for (int v : k)
    {
        if (v != 0)
        {
            if (v > high) high = v;
            if (v < low)  low = v;
        }
    }

    unblance = high - low;
}

bool advancePositions(std::vector<int>& positions, int max)
{
    int ind = 0;
    int end = int(positions.size()) - 1;
    
    // Find first position not packed to the end of range
    while(ind <= end && positions[end - ind] == max - ind) ind += 1;
    
    // If none is found return false
    if (ind > end) return false;
    
    // If found then advance it by one
    positions[end - ind] += 1;

    // And reset other positions after it
    for (int j = ind - 1; j >= 0; --j){
        positions[end - j] = positions[end - j - 1] + 1;
    }

    return true;
}

void constraintMk(
    int currentM, 
    const Galois::Field& gf, 
    const std::vector<GFMatrix>& mats, 
    const Constraint::Modifier& modifier,

    const std::vector<int>& k,
    const std::vector<int>& dims, 
    
    ILP& ilp, const Var* variables, Exp& obj)
{
    Exp det = ilp.CreateOperation();

    const std::vector<int> dets = constraintMkSubdets(currentM, mats, k, gf);
    int indMat = 0; 
    int prevlines = 0;

    for (int j = 0; j < currentM; j++)
    {
        while (j - prevlines >= k[indMat])
        {
            prevlines += k[indMat];
            indMat++;
        }

        det = det + dets[j] * variables[j - prevlines + dims[indMat] * currentM];
    }

    Var ks = ilp.CreateVariable("k");
    det = det - ks * gf.q;

    if (modifier.weak)
    {
        Var x = ilp.CreateVariable("w", 0, 1);
        obj = obj - modifier.weakWeight * x;
        
        if (modifier.weakWeight >= 0)
        {
            ilp.AddConstraint("WZN", x <= det.Clone());
            ilp.AddConstraint("WZN", det <= gf.q - 1);
        }
        else
        {
            ilp.AddConstraint("WZN", x * gf.q >= det.Clone());
            ilp.AddConstraint("WZN", det >= 0);
        }
    }
    else
    {
        if (gf.q == 2)
        {
            ilp.AddConstraint("DZN", det == 1);
        }
        else
        {
            ilp.AddConstraint("DZN", det >= 1);
            ilp.AddConstraint("DZN", det <= gf.q - 1);
        }
    }
}

void ZeroNetConstraint::Apply(
        const std::vector<GFMatrix>& matrices, 
        const Galois::Field& gf, 
        unsigned int currentM, 

        ILP& ilp, const Var* variables, Exp& obj
) const
{
    if (currentM < modifier.minM || currentM > modifier.maxM) return;

    int s = dims.size();
    std::vector<int> k(s);
    std::vector<int> positions(s - 1);

    for (int i = 0; i < s - 1; i++) positions[i] = i;

    const std::vector<GFMatrix> mats = [&](){
        std::vector<GFMatrix> mat;
        for (unsigned int i = 0; i < dims.size(); i++)
            mat.push_back(matrices[dims[i]]);
        return mat;
    }();

    do
    {
        int unblance = 0;
        positions2k(positions, k, unblance, currentM + s - 2);

        if (unblance <= max_unblance)
        {
            constraintMk(currentM, gf, mats, modifier, k, dims, ilp, variables, obj);
        }

    } while(advancePositions(positions, currentM + s - 2));
}

void StratifiedConstraint::Apply(
        const std::vector<GFMatrix>& matrices, 
        const Galois::Field& gf, 
        unsigned int currentM, 

        ILP& ilp, const Var* variables, Exp& obj
) const
{
    if (currentM < modifier.minM || currentM > modifier.maxM) return;

    const std::vector<GFMatrix> mats = [&](){
        std::vector<GFMatrix> mat;
        for (unsigned int i = 0; i < dims.size(); i++)
            mat.push_back(matrices[dims[i]]);
        return mat;
    }();

    std::vector<int> positions(currentM % dims.size(), 0);
    for (int i = 0; i < positions.size(); i++)
        positions[i] = i;

    std::vector<int> k(dims.size(), currentM / dims.size());

    do
    {
        for (int i = 0; i < positions.size(); i++)
            k[positions[i]] += 1;

        constraintMk(currentM, gf, mats, modifier, k, dims, ilp, variables, obj);

        for (int i = 0; i < positions.size(); i++)
            k[positions[i]] -= 1;

    } while(advancePositions(positions, dims.size() - 1));
}

void PropAConstraint::Apply(
        const std::vector<GFMatrix>& matrices, 
        const Galois::Field& gf, 
        unsigned int currentM, 

        ILP& ilp, const Var* variables, Exp& obj
) const
{
    if (currentM == dims.size()) 
        StratifiedConstraint::Apply(matrices, gf, currentM, ilp, variables, obj);
}


void PropAprimeConstraint::Apply(
        const std::vector<GFMatrix>& matrices, 
        const Galois::Field& gf, 
        unsigned int currentM, 

        ILP& ilp, const Var* variables, Exp& obj
) const
{
    if (currentM == 2 * dims.size()) 
        StratifiedConstraint::Apply(matrices, gf, currentM, ilp, variables, obj);
}
