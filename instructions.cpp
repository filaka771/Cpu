#include "instructions.h"


const char* instruction_flag[] = {
    "*x",
    " *",
    " x",
    "  " 
};

const InstructionSet instruction_set[] = {  
    {"inp", 1},
    {"out", 1},
    {"mov", 2},
    {"add", 3},
    {"sub", 3},
    {"mul", 3},
    {"div", 3},
    {"sqr", 3},
    {"bne", 3},
    {"beq", 3},
    {"bgt", 3},
    {"blt", 3},
    {"bge", 3},
    {"ble", 3},
    {"str", 1},
    {"ldr", 1},
    {"cfn", 1},
    {"ret", 0},
    {"hlt", 0}
};
