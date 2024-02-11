#include "Parser.hpp"

#include <fstream>

inline void trim(std::string& tmp)
{
    tmp.erase(tmp.find_last_not_of(' ') + 1);
    tmp.erase(0, tmp.find_first_not_of(' '));

    if (!tmp.empty() && tmp.back() == '\n')
        tmp.erase(tmp.size() - 1);
}

int try_convert(const std::string& value)
{
    bool isNumber = !value.empty() && std::all_of(value.begin(), value.end(), ::isdigit);

    if (!isNumber) return -1;
    return std::atoi(value.c_str());
}

std::string get_next_token(std::string& tmp)
{
    int start = 0;
    int end = 0;
    while (start < tmp.size() && ::isspace(tmp[start])) start++;

    end = start;
    while (end < tmp.size() && !::isspace(tmp[end])) end ++;
    
    std::string rslt = tmp.substr(start, end);
    if (!rslt.empty() && ::isspace(rslt.back())) rslt.pop_back();

    tmp = tmp.substr(end);
    return rslt;
}

MatbuilderProgram Parser::Parse(const std::string& filename) const
{
    MatbuilderProgram program;
    std::ifstream file (filename);
    
    if (!file.is_open()) return program;

    std::string tmp;
    while (std::getline(file, tmp))
    {
        trim(tmp);
        if (tmp.empty()) continue;

        if (program.s == 0 && tmp[0] == 's')      // Dimensions. Check for program.s to avoid conflicts with stratified
        {
            program.s = try_convert(tmp.substr(tmp.find_first_of('=') + 1));
            if (program.s < 1) 
            {
                std::cerr << "Error: can not parse s value (" << tmp << ")" << std::endl;
                return program;
            }
        }
        else if (program.p == 0 && tmp[0] == 'p') // Base
        {
            program.p = try_convert(tmp.substr(tmp.find_first_of('=') + 1));
            if (program.p < 2) 
            {
                std::cerr << "Error: can not parse p value or p < 2 (" << tmp << ")" << std::endl;
                return program;
            }
        }
        else if (program.m == 0 && tmp[0] == 'm') // Numbers to generate
        {
            program.m = try_convert(tmp.substr(tmp.find_first_of('=') + 1));
            if (program.m < 1) 
            {
                std::cerr << "Error: can not parse m value or m == 0 (" << tmp << ")" << std::endl;
                return program;
            }
        }
        else
        {
            Constraint::Modifier modifier;
            std::string token = get_next_token(tmp);
            
            if (token.rfind("weak") == 0)
            {
                modifier.weak = true;
                modifier.weakWeight = std::atoi(get_next_token(tmp).c_str());

                token = get_next_token(tmp);
            }
            
            if (token.rfind("from", 0) == 0)
            {
                modifier.minM = std::atoi(get_next_token(tmp).c_str());
                
                token = get_next_token(tmp);
                if (token.rfind("to", 0) == 0)
                {
                    modifier.maxM = std::atoi(get_next_token(tmp).c_str());
                    token = get_next_token(tmp);
                }
            }
            // no constraint in the end...
            // if (token.empty()) continue;

            std::string name = token;

            auto it = constraints.find(name);

            if (it == constraints.end()) 
            {
                std::cerr << name << " is not a known constraint" << std::endl;
                return program;
            }
            
            std::vector<int> dims;
            std::vector<std::string> arguments;

            token = get_next_token(tmp);
            while (try_convert(token) < 0)
            {
                arguments.push_back(token);
                token = get_next_token(tmp);
            }

            while (try_convert(token) >= 0)
            {
                dims.push_back(try_convert(token));
                token = get_next_token(tmp);
            }    

            if (dims.size() == 0)
            {
                std::cerr << "No dimension for the constraint: " << name << std::endl;
                return program;
            }
            
            program.constraints.push_back(
                it->second(modifier, dims, arguments)
            );
        }
    }
    program.is_valid = (program.s >= 1) && (program.m > 0) && (program.p > 1);
    return program;
}