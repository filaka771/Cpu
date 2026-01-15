#include "instructions/instructions.h"
#include "cpu_emulator/cpu_instructions.h"


const char instruction_flag [INSTRUCTIONS_FLAGS_NUMBER] = {'*', 'x'};
const char ignorable_symbols[IGNORABLE_SYMBOLS_NUMBER] = {' ',  '\t'};
const char parse_stoping_symbols[PARSE_STOPING_SYMB_NUMBER] = {'\n', '\0', '\r', ';'};

const InstructionSet instruction_set[] = {  
    {"inp", 1, &inp},
    {"out", 1, &out},
    {"mov", 2, &mov},
    {"add", 3, &add},
    {"sub", 3, &sub},
    {"mul", 3, &mul},
    {"div", 3, &div},
    {"sqr", 2, &sqr},
    {"bne", 3, &bne},
    {"beq", 3, &beq},
    {"bgt", 3, &bgt},
    {"blt", 3, &blt},
    {"bge", 3, &bge},
    {"ble", 3, &ble},
    {"baw", 1, &baw},
    {"str", 1, &str},
    {"ldr", 1, &ldr},
    {"cfn", 1, &cfn},
    {"ret", 0, &ret},
    {"hlt", 0, &hlt}
};
