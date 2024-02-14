#pragma once

#include <iostream>
#include <string>

#include <algorithm>
#include <vector>
#include <memory>
#include <map>

namespace ilp
{    
    struct DenseStorage
    {
        uint32_t size() const { return values.size(); }

        void add(uint32_t loc, int32_t value)
        {
            if (values.size() <= loc) values.resize(loc + 1, 0);
            values[loc] += value;
        }

        void add(const DenseStorage& other)
        {
            for (uint32_t i = 0; i < other.values.size(); i++)
            {
                add(i, other.values[i]);
            }
        }

        void times(int32_t value)
        {
            for (uint32_t i = 0; i < values.size(); i++)
            {
                values[i] *= value;
            }
        }

        int32_t coeff(uint32_t id) const
        {
            if (values.size() < id + 1) return 0;
            return values[id];
        }

        std::vector<std::pair<uint32_t, int32_t>> Get() const
        {
            std::vector<std::pair<uint32_t, int32_t>> rslt;
            rslt.reserve(values.size());

            for (uint32_t i = 0; i < values.size(); i++)
            {
                if (values.at(i) != 0)
                {
                    rslt.push_back(std::make_pair(i, values.at(i)));
                }
            }
            return rslt;
        }

        std::vector<int32_t> values;
    };

    struct SparseStorage
    {
        uint32_t size() const { return values.size(); }

        void add(uint32_t loc, int32_t value)
        {
            auto lb = values.lower_bound(loc);
            if (lb != values.end() && loc == lb->first)
                lb->second += value; 
            else
                values.insert(lb, std::make_pair(loc, value));
        }

        void add(const SparseStorage& storage)
        {
            for (const auto& it : storage.values)
            {
                add(it.first, it.second);
            }
        }

        void times(int32_t value)
        {
            for (auto& it : values)
                it.second *= value;
        }

        int32_t coeff(uint32_t id) const
        {
            auto it = values.find(id);
            if (it == values.end()) return 0;
            return it->second;
        }

        std::vector<std::pair<uint32_t, int32_t>> Get() const
        {
            std::vector<std::pair<uint32_t, int32_t>> rslt;
            rslt.reserve(values.size());

            for (const auto& var : values)
            {
                if (var.second != 0)
                {
                    rslt.push_back(std::make_pair(var.first, var.second));
                }
            }
            return rslt;
        }

        std::map<uint32_t, int32_t> values;
    };

    // Templated so operations now what type of Storage to inherit from
    template<typename Storage>
    struct Variable
    {
        const uint32_t   id = 0;
        const int32_t coeff = 1;
    };

    template<typename Storage>
    struct LinearOperation
    {
        LinearOperation() :
            coefficients(std::make_shared<Storage>())
        { }

        LinearOperation Clone() const
        {
            LinearOperation<Storage> rslt;
            *rslt.coefficients = *coefficients;
            rslt.shift = shift;
            return rslt;
        }

        void add(const Variable<Storage>& var)
        {
            coefficients->add(var.id, var.coeff);
        }    

        void add(int32_t value)
        {
            shift += value;
        }

        void add(const LinearOperation<Storage>& other)
        {
            coefficients->add(*other.coefficients);
        }

        void times(int32_t value)
        {
            coefficients->times(value);
            shift *= value;
        }

        std::vector<std::pair<uint32_t, int32_t>> get() const
        {
            return coefficients->Get();
        }

        int32_t shift = 0;
        std::shared_ptr<Storage> coefficients = nullptr;
    };

    enum struct ComparisonType
    {
        EQUAL,
        NOT_EQUAL,

        GREATER,
        GREATER_EQUAL, 

        LOWER,
        LOWER_EQUAL
    };

    inline std::string to_string(ComparisonType tp)
    {
        switch (tp)
        {
        case ComparisonType::EQUAL: return "==";
        case ComparisonType::NOT_EQUAL: return "!=";
        case ComparisonType::GREATER: return ">";
        case ComparisonType::LOWER: return "<";
        case ComparisonType::GREATER_EQUAL: return ">=";
        case ComparisonType::LOWER_EQUAL: return "<=";
        default:
            return "";
        }
    }

    template<typename Storage>
    struct Constraint
    {
        const ComparisonType type;
        const Storage coefs;
        const int32_t rhs;
    };

    template<typename Storage>
    std::string MPS(
        const std::string& name, 
        const Storage& objective, 
        const std::vector<std::string>& vName, 
        const std::vector<std::string>& cName, 
        const std::vector<Constraint<Storage>>& constraints
    )
    {
        const std::string OBJECTIVE_NAME = "COST";
        std::string MPS = "NAME          " + name + "\n";
        MPS += "ROWS\n";
        
        if (objective.size() > 0)
            MPS += " N   " + OBJECTIVE_NAME + "\n";

        // Write rows
        for (unsigned int i = 0; i < constraints.size(); i++) 
        {
            const Constraint<Storage>& c = constraints[i];      
            if (c.coefs.size() >= 1) 
            {
                switch (c.type)
                {
                case ComparisonType::EQUAL:         MPS += " E   "; break;
                case ComparisonType::GREATER: 
                case ComparisonType::GREATER_EQUAL: MPS += " G   "; break;
                case ComparisonType::LOWER:
                case ComparisonType::LOWER_EQUAL:   MPS += " L   "; break;
                default:
                    std::cout << "[MPS] Comparison type not supported by MPS format !" << std::endl;
                    return "";
                }
                std::string capitalized = cName[i];
                // std::transform(capitalized.begin(), capitalized.end(), ::toupper);
                MPS += capitalized + "\n";
            }
        }
        
        MPS += "COLUMNS\n";
        MPS += "    MARK0000  'MARKER'  'INTORG'\n";

        for (unsigned int i = 0; i < vName.size(); i++)
        {
            std::string MPS_VAR;
            if (vName[i].size() > 9) {
                std::cout << "[MPS] Variable name is too long: " << vName[i] << std::endl;
                return "";
            }

            const std::string begin = "    " + vName[i] + std::string(10 - vName[i].size(), ' ');

            int32_t obj_coeff = objective.coeff(i);
            if (obj_coeff != 0)
            {
                MPS_VAR += begin + OBJECTIVE_NAME + std::string(10 - OBJECTIVE_NAME.size(), ' ');
                MPS_VAR += std::to_string(obj_coeff);
                MPS_VAR += "\n";
            }

            for (unsigned int j = 0; j < constraints.size(); j++)
            {
                const int32_t coeff = constraints[j].coefs.coeff(i); 
                if (coeff != 0)
                {
                    if (cName[j].size() > 9) 
                    {
                        std::cout << "[MPS] Constaint name too long: " << cName[j] << std::endl;
                        return "";
                        }   

                    MPS_VAR += begin + cName[j] + std::string(10 - cName[j].size(), ' ');
                    MPS_VAR += std::to_string(coeff);
                    MPS_VAR += '\n';
                }
            }



            MPS += MPS_VAR;
        }
        
        MPS += "    MARK0000  'MARKER'  'INTEND'\n";

        MPS += "RHS\n";
        for (unsigned int i = 0; i < constraints.size(); i++)
        {
            if (constraints[i].coefs.size() == 0) continue;

            std::string MPS_RHS;
            MPS_RHS += "    RHS1      " + cName[i] + std::string(10 - cName[i].size(), ' ');

            switch (constraints[i].type)
            {
            case ComparisonType::EQUAL:         MPS_RHS += std::to_string(constraints[i].rhs + 0); break;
            case ComparisonType::GREATER:       MPS_RHS += std::to_string(constraints[i].rhs + 1); break;
            case ComparisonType::GREATER_EQUAL: MPS_RHS += std::to_string(constraints[i].rhs + 0); break;
            case ComparisonType::LOWER:         MPS_RHS += std::to_string(constraints[i].rhs - 1); break;
            case ComparisonType::LOWER_EQUAL:   MPS_RHS += std::to_string(constraints[i].rhs - 0); break;
            default:
                std::cout << "[MPS] Comparison type not supported by MPS format !";
                return "";
            }
            MPS += MPS_RHS + '\n';
        }

        MPS += "ENDATA";
        return MPS;
    }

    template<typename Storage>
    std::string StrEquation(
        const std::vector<std::string>& vName, 
        const Storage& equation
    )
    {
        std::string EQ;

        std::vector<std::pair<uint32_t, std::string>> symbols;
        for (unsigned int i = 0; i < vName.size(); i++)
        {
            int32_t coeff = equation.coeff(i);
            if (coeff != 0)
            {
                     if (coeff > 0 && EQ.size() != 0) EQ += " + ";
                else if (coeff < 0) EQ += " - ";

                EQ += std::to_string(std::abs(coeff)) + ' ' + vName[i];
            }
        }
        return EQ;
    }

    template<typename Storage>
    std::string LP(
        const std::string& name, 
        const Storage& objective, 
        const std::vector<std::string>& vName, 
        const std::vector<std::string>& cName, 
        const std::vector<Constraint<Storage>>& constraints
    )
    {
        std::string LP;
        LP += "Minimize\n";
        
        LP += " " + StrEquation<Storage>(vName, objective);
        LP += "\nSubject To\n";
        
        for (unsigned int i = 0; i < constraints.size(); i++)
        {
            if (constraints[i].coefs.size() > 0)
            {
                LP += " " + cName[i] + ": " + StrEquation(vName, constraints[i].coefs);
                LP += " " +to_string(constraints[i].type) + " " + std::to_string(constraints[i].rhs) + "\n"; 
            }
        }

        LP += "Generals\n ";
        for (unsigned int i = 0; i < vName.size(); i++)
            LP += vName[i] + " ";
        LP += "\nEnd";
        return LP;
    }

    template<typename Storage = SparseStorage>
    class IntegerLinearProgramBuilder
    {
    public:
        Variable<Storage> CreateVariable(const std::string& prefix, int low = 0, int high = INT32_MAX)
        {
            auto it = categoryCount.find(prefix);
            if (it == categoryCount.end())
            {
                categoryCount[prefix] = 0;
                it = categoryCount.find(prefix);
            }    

            variableName.push_back(prefix + std::to_string(it->second++));
            variableBounds.push_back(std::make_pair(low, high));
            return Variable<Storage>{static_cast<uint32_t>(variableName.size() - 1), 1};
        }

        std::vector<Variable<Storage>> CreateVariables(const std::string& prefix, uint32_t count, int low = 0, int high = INT32_MAX)
        { 
            std::vector<Variable<Storage>> rslt;
            rslt.reserve(count);

            for (uint32_t i = 0; i < count; i++)
                rslt.emplace_back(CreateVariable(prefix, low, high));
            return rslt;
        }

        LinearOperation<Storage> CreateOperation()
        {
            return LinearOperation<Storage>();
        }

        void SetObjective(const LinearOperation<Storage>& op, bool maximize = false)
        {
            objective = *op.coefficients;
            
            if (maximize) objective.times(-1);
        }

        void AddConstraint(const std::string prefix, Constraint<Storage>&& constraint)
        {
            auto it = categoryCount.find(prefix);
            if (it == categoryCount.end())
            {
                categoryCount[prefix] = 0;
                it = categoryCount.find(prefix);
            }    

            constraintsNames.push_back(prefix + std::to_string(it->second++));
            constraints.push_back(std::move(constraint));
        }

        std::string ToMPS(const std::string& name) const 
        {
            return MPS(
                name, 
                objective, 
                variableName, 
                constraintsNames, 
                constraints
            );
        }

        std::string ToLP(const std::string& name) const
        {
            return LP(
                name, 
                objective, 
                variableName, 
                constraintsNames, 
                constraints
            );
        }

        void DebugPrint() const
        {
            std::cout << ToLP("") << std::endl;
        }

        const Storage& GetObjective() const
        {
            return objective;
        } 

        const std::vector<Constraint<Storage>>& GetConstraints() const
        {
            return constraints;
        }

        const std::vector<std::string>& GetConstraintNames() const
        {
            return constraintsNames;
        }

        const std::vector<std::string>& GetVariableNames() const
        {
            return variableName;
        }

        const std::vector<std::pair<int, int>>& GetVariablesBounds() const
        {
            return variableBounds;
        }

    private:
        std::map<std::string, uint32_t> categoryCount;
        std::vector<std::string> variableName;
        std::vector<std::pair<int, int>> variableBounds;

        Storage objective;
        std::vector<std::string> constraintsNames;
        std::vector<Constraint<Storage>> constraints;
    };

    template<typename Storage>
    inline LinearOperation<Storage> asLinearOperation(const Variable<Storage>& var)
    {
        LinearOperation<Storage> op; op.add(var);
        return op;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator+(const LinearOperation<Storage>& op, const Variable<Storage>& variable)
    {
        LinearOperation<Storage> result = op; result.add(variable);
        return result;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator+(const Variable<Storage>& variable, const LinearOperation<Storage>& op)
    {
        LinearOperation<Storage> result = op; result.add(variable);
        return result;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator-(const LinearOperation<Storage>& op, const Variable<Storage>& variable)
    {
        LinearOperation<Storage> result = op; result.add({variable.id, -variable.coeff});
        return result;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator-(const Variable<Storage>& variable, const LinearOperation<Storage>& op)
    {
        LinearOperation<Storage> result = op; 
        result.times(-1);
        result.add(variable);
        return result;
    }

    template<typename Storage>
    inline Variable<Storage> operator*(const Variable<Storage>& variable, int32_t rhs)
    {
        Variable<Storage> newVar{variable.id, rhs * variable.coeff};
        return newVar;
    }

    template<typename Storage>
    inline Variable<Storage> operator*(int32_t lhs, const Variable<Storage>& variable)
    {
        Variable<Storage> newVar{variable.id, lhs * variable.coeff};
        return newVar;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator+(const Variable<Storage>& lhs, const Variable<Storage>& rhs)
    {
        LinearOperation<Storage> newOperation;
        newOperation.add(lhs);
        newOperation.add(rhs);
        return newOperation;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator-(const Variable<Storage>& lhs, const Variable<Storage>& rhs)
    {
        LinearOperation<Storage> newOperation;
        newOperation.add(lhs);
        newOperation.add({rhs.id, -rhs.coeff});
        return newOperation;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator+(int32_t val, const Variable<Storage>& lhs)
    {
        LinearOperation<Storage> newOperation;
        newOperation.add(lhs);
        newOperation.shift += val;
        return newOperation;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator+(const Variable<Storage>& lhs, int32_t val)
    {
        LinearOperation<Storage> newOperation;
        newOperation.add(lhs);
        newOperation.add(val);
        return newOperation;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator-(int32_t val, const Variable<Storage>& lhs)
    {
        LinearOperation<Storage> newOperation;
        newOperation.add({lhs.id, -lhs.coeff});
        newOperation.add(val);
        return newOperation;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator-(const Variable<Storage>& lhs, int32_t val)
    {
        LinearOperation<Storage> newOperation;
        newOperation.add(lhs);
        newOperation.add(-val);
        return newOperation;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator-(const LinearOperation<Storage>& op, int32_t val)
    {
        LinearOperation<Storage> rslt = op;
        rslt.add(-val);
        return rslt;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator+(const LinearOperation<Storage>& op, int32_t val)
    {
        LinearOperation<Storage> rslt = op;
        rslt.add(val);
        return rslt;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator*(const LinearOperation<Storage>& op, int32_t val)
    {
        LinearOperation<Storage> newOp = op;
        newOp.times(val);
        return newOp;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator*(int32_t val, const LinearOperation<Storage>& op)
    {
        LinearOperation<Storage> newOp = op;
        newOp.times(val);
        return newOp;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator+(const LinearOperation<Storage>& op1, const LinearOperation<Storage>& op2)
    {
        if (op1.coefficients.get() == op2.coefficients.get()) return op1 * 2;

        LinearOperation<Storage> newOp;
        newOp.add(op1);
        newOp.add(op2);
        return newOp;
    }

    template<typename Storage>
    inline LinearOperation<Storage> operator-(const LinearOperation<Storage>& op1, const LinearOperation<Storage>& op2)
    {
        if (op1.coefficients.get() == op2.coefficients.get())
        {
            return op1 * 0;
        }

        LinearOperation<Storage> newOp;
        newOp.add(op2);
        newOp.times(-1);
        newOp.add(op1);
        return newOp;
    }

    // Comparison between linearop and integers

    template<typename Storage>
    inline Constraint<Storage> operator==(const LinearOperation<Storage>& op, int32_t value)
    {
        return Constraint<Storage>{
            ComparisonType::EQUAL, 
            *op.coefficients, 
            value - op.shift
        };
    }

    template<typename Storage>
    inline Constraint<Storage> operator==(int32_t value, const LinearOperation<Storage>& op)
    {
        return Constraint<Storage>{
            ComparisonType::EQUAL, 
            *op.coefficients, 
            value - op.shift
        };
    }

    template<typename Storage>
    inline Constraint<Storage> operator>=(int32_t value, const LinearOperation<Storage>& op)
    {
        return Constraint<Storage>{
            ComparisonType::LOWER_EQUAL, 
            *op.coefficients, 
            value - op.shift
        };
    }

    template<typename Storage>
    inline Constraint<Storage> operator>=(const LinearOperation<Storage>& op, int32_t value)
    {
        return Constraint<Storage>{
            ComparisonType::GREATER_EQUAL, 
            *op.coefficients, 
            value - op.shift
        };
    }

    template<typename Storage>
    inline Constraint<Storage> operator>(int32_t value, const LinearOperation<Storage>& op)
    {
        return Constraint<Storage>{
            ComparisonType::LOWER, 
            *op.coefficients, 
            value - op.shift
        };
    }

    template<typename Storage>
    inline Constraint<Storage> operator>(const LinearOperation<Storage>& op, int32_t value)
    {
        return Constraint<Storage>{
            ComparisonType::GREATER, 
            *op.coefficients, 
            value - op.shift
        };
    }

    template<typename Storage>
    inline Constraint<Storage> operator<=(const LinearOperation<Storage>& op, int32_t value)
    {
        return Constraint<Storage>{
            ComparisonType::LOWER_EQUAL, 
            *op.coefficients, 
            value - op.shift
        };
    }

    template<typename Storage>
    inline Constraint<Storage> operator<=(int32_t value, const LinearOperation<Storage>& op)
    {
        return Constraint<Storage>{
            ComparisonType::GREATER_EQUAL, 
            *op.coefficients, 
            value - op.shift
        };
    }

    template<typename Storage>
    inline Constraint<Storage> operator<(const LinearOperation<Storage>& op, int32_t value)
    {
        return Constraint<Storage>{
            ComparisonType::LOWER, 
            *op.coefficients, 
            value - op.shift
        };
    }

    template<typename Storage>
    inline Constraint<Storage> operator<(int32_t value, const LinearOperation<Storage>& op)
    {
        return Constraint<Storage>{
            ComparisonType::GREATER, 
            *op.coefficients, 
            value - op.shift
        };
    }

    // Comparison between variables and integers, everything is defined by 
    // comparison between lineaop and integers

    template<typename Storage>
    inline Constraint<Storage> operator==(const Variable<Storage>& var, int32_t value)
    {
        return asLinearOperation(var) == value;
    }

    template<typename Storage>
    inline Constraint<Storage> operator==(int32_t value, const Variable<Storage>& var)
    {
        return value == asLinearOperation(var);
    }

    template<typename Storage>
    inline Constraint<Storage> operator>=(int32_t value, const Variable<Storage>& var)
    {
        return value >= asLinearOperation(var);
    }

    template<typename Storage>
    inline Constraint<Storage> operator>=(const Variable<Storage>& var, int32_t value)
    {
        return asLinearOperation(var) >= value;
    }

    template<typename Storage>
    inline Constraint<Storage> operator>(int32_t value, const Variable<Storage>& var)
    {
        return value > asLinearOperation(var);
    }

    template<typename Storage>
    inline Constraint<Storage> operator>(const Variable<Storage>& var, int32_t value)
    {
        return asLinearOperation(var) > value;
    }

    template<typename Storage>
    inline Constraint<Storage> operator<=(const Variable<Storage>& var, int32_t value)
    {
        return asLinearOperation(var) <= value;
    }

    template<typename Storage>
    inline Constraint<Storage> operator<=(int32_t value, const Variable<Storage>& var)
    {
        return value <= asLinearOperation(var);
    }

    template<typename Storage>
    inline Constraint<Storage> operator<(const Variable<Storage>& var, int32_t value)
    {
        return asLinearOperation(var) < value;
    }

    template<typename Storage>
    inline Constraint<Storage> operator<(int32_t value, const Variable<Storage>& var)
    {
        return value < asLinearOperation(var);
    }


    // Comparison between variables and linearops, everything is defined by 
    // comparison between linearops and integers

    template<typename Storage>
    inline Constraint<Storage> operator==(const Variable<Storage>& var, const LinearOperation<Storage>& op)
    {
        // op - var less costly than var - op
        return op - var == 0;
    }

    template<typename Storage>
    inline Constraint<Storage> operator==(const LinearOperation<Storage>& op, const Variable<Storage>& var)
    {
        return op - var == 0;
    }

    template<typename Storage>
    inline Constraint<Storage> operator>=(const LinearOperation<Storage>& op, const Variable<Storage>& var)
    {
        return op - var >= 0;
    }

    template<typename Storage>
    inline Constraint<Storage> operator>=(const Variable<Storage>& var, const LinearOperation<Storage>& op)
    {
        return 0 >= op - var;
    }

    template<typename Storage>
    inline Constraint<Storage> operator>(const LinearOperation<Storage>& op, const Variable<Storage>& var)
    {
        return op - var > 0;
    }

    template<typename Storage>
    inline Constraint<Storage> operator>(const Variable<Storage>& var, const LinearOperation<Storage>& op)
    {
        return 0 > var - op;
    }

    template<typename Storage>
    inline Constraint<Storage> operator<=(const Variable<Storage>& var, const LinearOperation<Storage>& op)
    {
        return 0 <= (op - var);
    }

    template<typename Storage>
    inline Constraint<Storage> operator<=(const LinearOperation<Storage>& op, const Variable<Storage>& var)
    {
        return (op - var) <= 0;
    }

    template<typename Storage>
    inline Constraint<Storage> operator<(const Variable<Storage>& var, const LinearOperation<Storage>& op)
    {
        return 0 < op - var;
    }

    template<typename Storage>
    inline Constraint<Storage> operator<(const LinearOperation<Storage>& op, const Variable<Storage>& var)
    {
        return op - var < 0;
    }

    // Comparison between linearops and linearops, everything is defined by 
    // comparison between linearops and integers

    template<typename Storage>
    inline Constraint<Storage> operator==(const LinearOperation<Storage>& var, const LinearOperation<Storage>& op)
    {
        return op - var == 0;
    }

    template<typename Storage>
    inline Constraint<Storage> operator>=(const LinearOperation<Storage>& var, const LinearOperation<Storage>& op)
    {
        return 0 >= op - var;
    }

    template<typename Storage>
    inline Constraint<Storage> operator>(const LinearOperation<Storage>& var, const LinearOperation<Storage>& op)
    {
        return 0 > var - op;
    }

    template<typename Storage>
    inline Constraint<Storage> operator<=(const LinearOperation<Storage>& op, const LinearOperation<Storage>& var)
    {
        return op - var <= 0;
    }

    template<typename Storage>
    inline Constraint<Storage> operator<(const LinearOperation<Storage>& op, const LinearOperation<Storage>& var)
    {
        return op - var < 0;
    }
};