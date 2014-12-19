#include "InputParser.h"
#include <stdio.h>


int main(int argc, const char **args) {

    std::vector<ParamAndValue> argVec;
    if(!parseInput(argc, args, argVec)) {
        printf("main(): parse error\n");
        return -1;
    }

    for(int i = 0; i < (int)argVec.size(); ++i) {

        const ParamAndValue &curPair = argVec[i];
        printf("%s  : %s\n", curPair.name.c_str(), curPair.value.c_str());

    }


    return 0;

}

