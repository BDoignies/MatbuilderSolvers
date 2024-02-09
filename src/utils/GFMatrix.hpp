#pragma once

#include <galois++/field.h>
#include <fstream>
#include <vector>
#include <cmath>

class GFMatrix
{
public:
    explicit GFMatrix(int m): m(m), data(m * m, 0) {}
    GFMatrix(const std::vector<int>& data): m(int(round(sqrt(data.size())))), data(data) {} 

    static GFMatrix From(std::istream& iss);

    void To(std::ostream& iss) const;

    int size() const { return m; }

    int& at(int i, int j)       { return *(data.data() + j + i * m); }
    int  at(int i, int j) const { return *(data.data() + j + i * m); }
    
          int* operator[](int idx)       { return data.data() + idx * m; }
    const int* operator[](int idx) const { return data.data() + idx * m; }

    void Print() const;

    // int multiply(int other, const Galois::Field& gf) const;
    int determinant(const Galois::Field& gf, int override_m = -1) const;
private:
    int m;
    std::vector<int> data;
};

GFMatrix TakeRowsFrom(const GFMatrix& first, const GFMatrix& second, int m);