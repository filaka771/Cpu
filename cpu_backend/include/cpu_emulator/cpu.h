#include <stdint.h>
#include "stack/stack.h"

#ifndef CPU_H
#define CPU_H

#define NUM_OF_REGISTERS 20 
#define BIN_INSTRUCTION_SIZE 16

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
