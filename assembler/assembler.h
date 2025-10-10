#include <stdint.h>

typedef struct BinInstruction{
    uint32_t operation;
    uint32_t arg_list[3];
} BinInstruction;

typedef struct BinInstructionArray{
    BinInstruction* bin_instruction_list;
    uint32_t count;
} BinInstructionArray;
