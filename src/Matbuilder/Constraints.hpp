#pragma once

#include <random>
#include "ILP/ILP_def.hpp"
#include "utils/GFMatrix.hpp"
#include <vector>

class Constraint
{
public:
    struct Modifier
    {
        bool weak = false;
        int weakWeight = 0;

        int minM = 0;
        int maxM = INT32_MAX;
    };

    Constraint(const Modifier& modifier, const std::vector<int>& dimList) : 
        modifier(modifier), dims(dimList)
    { }

    template<typename T>
    static T* Create(const Modifier& modifier, const std::vector<int>& dimList, const std::vector<std::string>& opts)
    {
        return new T(modifier, dimList, opts);
    }

    virtual void Apply(
        const std::vector<GFMatrix>& matrices,
        const Galois::Field& gf,  
        unsigned int currentM, 

        ILP& ilp, const Var* variables, Exp& obj
    ) const = 0;

    const Modifier& GetModifier() const
    { return modifier; }

    // virtual void Check() const = 0;

    virtual ~Constraint() {}
protected:
    Modifier modifier;
    std::vector<int> dims;
};

class ZeroNetConstraint : public Constraint
{
public:
    ZeroNetConstraint(const Modifier& modifier, const std::vector<int>& dimList, const std::vector<std::string>& opts) : 
        Constraint(modifier, dimList), max_unblance(32768)
    { 
        if (opts.size() == 1 && opts[0].size() > 1)
        {
            std::string unba(opts[0].data() + 1, opts[0].size() - 1);
            
            // Check if number first
            if (!unba.empty() && std::all_of(unba.begin(), unba.end(), ::isdigit))
            {
                max_unblance = std::stoi(unba);
            }
        }
    }

    virtual void Apply(
        const std::vector<GFMatrix>& matrices, 
        const Galois::Field& gf, 
        unsigned int currentM, 

        ILP& ilp, const Var* variables, Exp& obj
    ) const;
public:
    int max_unblance;
};

class StratifiedConstraint : public Constraint
{
public:
    StratifiedConstraint(const Modifier& modifier, const std::vector<int>& dimList, const std::vector<std::string>& opts) : 
        Constraint(modifier, dimList)
    {  }

    virtual void Apply(
        const std::vector<GFMatrix>& matrices, 
        const Galois::Field& gf, 
        unsigned int currentM, 

        ILP& ilp, const Var* variables, Exp& obj
    ) const;
private:
};

class PropAConstraint : public StratifiedConstraint
{
public:
    PropAConstraint(const Modifier& modifier, const std::vector<int>& dimList, const std::vector<std::string>& opts) : 
        StratifiedConstraint(modifier, dimList, opts)
    {  }

    virtual void Apply(
        const std::vector<GFMatrix>& matrices, 
        const Galois::Field& gf, 
        unsigned int currentM, 

        ILP& ilp, const Var* variables, Exp& obj
    ) const;
private:
};

class PropAprimeConstraint : public StratifiedConstraint
{
public:
    PropAprimeConstraint(const Modifier& modifier, const std::vector<int>& dimList, const std::vector<std::string>& opts) : 
        StratifiedConstraint(modifier, dimList, opts)
    {  }

    virtual void Apply(
        const std::vector<GFMatrix>& matrices, 
        const Galois::Field& gf, 
        unsigned int currentM, 

        ILP& ilp, const Var* variables, Exp& obj
    ) const;
private:
};
