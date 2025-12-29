#include <stdint.h>
#include "stack/stack.h"

#ifndef CPU_H
#define CPU_H

#define NUM_OF_REGISTERS 19
#define RBP NUM_OF_REGISTERS - 3
#define RPC NUM_OF_REGISTERS - 2
#define RCC NUM_OF_REGISTERS - 1

#define BIN_INSTRUCTION_SIZE 16

#define DEBUG 1

typedef struct Cpu{
    // reg0, ... reg15 - general purpose registers
    // reg16           - used as rsp
    // reg17           - used as rbp
    // reg18           - used as rpc
    // reg19           - used as rcc(register cpu cycles)

    // Special purpose registers implementation
    bool running;
    // General purpose registers implementations
    uint32_t regs[NUM_OF_REGISTERS];

    // Stack implementation
    Stack* stack;
}Cpu;


#endif 
