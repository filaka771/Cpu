#include <stdint.h>
#include "stack/stack.h"

#ifndef CPU_H
#define CPU_H

#define NUM_OF_REGISTERS 19
#define RPC NUM_OF_REGISTERS - 3
#define RBP NUM_OF_REGISTERS - 2
#define RCC NUM_OF_REGISTERS - 1

#define BIN_INSTRUCTION_SIZE 16
#define MAX_INSTRUCTIONS 65536   // 64K instructions max
#define CODE_MEM_SIZE (MAX_INSTRUCTIONS * INSTRUCTION_SIZE) 

#define CALL_STACK_INIT_SIZE 8192
//#define DATA_STACK_INIT_SIZE 65536
#define DATA_STACK_INIT_SIZE 499

#define DEBUG 0

typedef struct Cpu{
    // reg0, ... reg15 - general purpose registers
    // reg16           - used as rpc
    // reg19           - used as rbp
    // reg20           - used as rcc(register cpu cycles)

    // Cpu state
    bool running;
    uint32_t error_code;

    // General purpose registers implementations
    uint32_t regs[NUM_OF_REGISTERS];

    // Program buffer
    uint8_t* program_buffer;

    // Stack implementation
    Stack* data_stack;
    Stack* call_stack;
}Cpu;


#endif 
