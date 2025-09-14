#include<stdint.h>

#define INSTRUCTION_SIZE 3
#define ARG_SIZE 2
#define INSTRUCTIONS_SET_NUMBER 19
#define INSTRUCTIONS_FLAGS_NUMBER 4

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
    char imm_flag [2];
    int32_t imm;
}InstructionArg;

typedef struct TextInstructionOp{
    char operation_name [4];  // Change to char array, not pointer array
    uint32_t num_of_args;
}TextInstructionOp;

typedef struct TextInstruction{
    int32_t address;
    TextInstructionOp operation;
    TextInstructionArg imm [3];
}TextInstruction;

typedef struct TextInstructionArray{
    uint32_t count;
    TextInstruction* text_instruction_list;
}TextInstructionArray;
//-------------------------------------------------------

typedef struct InstructionSet {
    const char* op_name;  
    int8_t num_of_args;
} InstructionSet; 

extern const char* instruction_flag[4];
extern const InstructionSet instruction_set[INSTRUCTIONS_SET_NUMBER];

#endif // INSTRUCTIONS_H
