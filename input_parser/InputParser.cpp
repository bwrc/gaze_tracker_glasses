#include <string>
#include <stdio.h>
#include  "InputParser.h"
#include <string.h>


static void getParamValue(const char *str, std::string &paramValue);
    static bool getParamName(const char *str, std::string &paramName);


/*
 * looks for -param_name value
 */
bool parseInput(int argc, const char **args, std::vector<ParamAndValue> &res) {

    for(int i = 1; i < argc; ++i) {

        const char *str = args[i];

        // parameter found
        if(str[0] == '-') {

            std::string paramName;
            if(!getParamName(str, paramName)) {

                printf("Bad parameter name %s\n", str);

                return false;

            }

            std::string paramValue;
            if(i < argc - 1) {
                getParamValue(args[i + 1], paramValue);
            }

            res.push_back(ParamAndValue(paramName, paramValue));

        }

    }

    return true;

}


bool getParamName(const char *str, std::string &paramName) {

    paramName.clear();

    int len = strlen(str);

    if(len < 2) {
        return false;
    }

    paramName = std::string(str + 1);

    return true;

}


void getParamValue(const char *str, std::string &paramValue) {

    paramValue.clear();

    if(str[0] != '-') {

        paramValue = std::string(str);

    }

}

