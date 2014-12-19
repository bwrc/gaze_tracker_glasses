#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H


#include <string>
#include <vector>


class ParamAndValue {
public:
    ParamAndValue(const std::string _name, const std::string _val) {
        name  = _name;
        value = _val;
    }

    std::string name;
    std::string value;
};


/*
 * looks for -param_name value
 */
bool parseInput(int argc, const char **args, std::vector<ParamAndValue> &res);



#endif

