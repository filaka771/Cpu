#include<stdint.h>

#define INSTRUCTION_SIZE 3
#define ARG_SIZE 2
#define INSTRUCTIONS_SET_NUMBER 20
#define INSTRUCTIONS_FLAGS_NUMBER 2
#define IGNORABLE_SYMBOLS_NUMBER 2
#define PARSE_STOPING_SYMB_NUMBER 2

#define INIT_TEXT_INSTR_ARR_SIZE 1000

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

typedef struct Instruction{
    uint32_t op_name;
    uint32_t imm [3];
}Instruction;

typedef struct InstructionArray{
    uint32_t count;
    Instruction* instruction_list;
}InstructionList;

//-------------------------------------------------------


typedef struct TextInstructionArg{
    char imm_flag [3];
    int32_t imm;
}TextInstructionArg;

typedef struct TextInstructionOp{
    char operation_name [4];
    uint32_t num_of_args;
}TextInstructionOp;

typedef struct TextInstruction{
    int32_t address;
    TextInstructionOp operation;
    TextInstructionArg imm [3];
}TextInstruction;

typedef struct TextInstructionArray{
    uint32_t count;
    uint32_t capacity;
    TextInstruction* text_instruction_list;
}TextInstructionArray;
//-------------------------------------------------------

typedef struct InstructionSet {
    const char* op_name;  
    int8_t num_of_args;
} InstructionSet; 

extern const char parse_stoping_symbols[PARSE_STOPING_SYMB_NUMBER];
extern const char ignorable_symbols[IGNORABLE_SYMBOLS_NUMBER];
extern const char instruction_flag[INSTRUCTIONS_FLAGS_NUMBER];
extern const InstructionSet instruction_set[INSTRUCTIONS_SET_NUMBER];

#endif // INSTRUCTIONS_H
