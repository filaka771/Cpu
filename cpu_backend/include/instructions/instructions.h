#include<stdint.h>
#include "cpu_emulator/cpu.h"

#define INSTRUCTION_SIZE 3
#define ARG_SIZE 2
#define INSTRUCTIONS_SET_NUMBER 20
#define MAX_INSTRUCTIONS_NUMBER 256
#define INSTRUCTIONS_FLAGS_NUMBER 2
#define MAX_FLAG_NUMBER 8
#define IGNORABLE_SYMBOLS_NUMBER 2
#define PARSE_STOPING_SYMB_NUMBER 4

#define INIT_TEXT_INSTR_ARR_SIZE 1000

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

// Bin instruction representation

typedef struct BinInstruction{
    uint32_t operation;
    uint32_t arg_list[3];
} BinInstruction;

typedef struct BinInstructionArray{
    BinInstruction* bin_instruction_list;
    uint32_t count;
} BinInstructionArray;

// Text instruction representation


typedef struct TextInstructionArg{
    char imm_flag [3];
    int32_t imm;
}TextInstructionArg;

typedef struct TextInstructionOp{
    char operation_name [4];
    short op_code;
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
    const int8_t num_of_args;
    void (*cpu_instruction_pointer)(Cpu* cpu);
} InstructionSet; 

extern const char parse_stoping_symbols[PARSE_STOPING_SYMB_NUMBER];
extern const char ignorable_symbols[IGNORABLE_SYMBOLS_NUMBER];
extern const char instruction_flag[INSTRUCTIONS_FLAGS_NUMBER];
extern const InstructionSet instruction_set[INSTRUCTIONS_SET_NUMBER];

#endif // INSTRUCTIONS_H
