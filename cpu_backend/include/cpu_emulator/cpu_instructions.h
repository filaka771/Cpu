#include <stdint.h>

#include "stack/stack.h"
#include "./cpu.h"

#ifndef CPU_INSTRUCTIONS
#define CPU_INSTRUCTIONS
typedef struct CpuInstructionArg{
    bool abst_op;
    bool in_reg;
    uint32_t cpu_imm_arg;
} CpuInstructionArg;

typedef struct CpuInstructionArgs{
    uint32_t cpu_op_code;
    CpuInstructionArg cpu_imm_args[3];
} CpuInstructionArgs;


void cpu_deinitialize(Cpu* cpu);

void inp(Cpu* cpu);

void out(Cpu* cpu);

// --------------------------------------------------------------------
void mov(Cpu* cpu);

void add(Cpu* cpu);

void sub(Cpu* cpu);

void mul(Cpu* cpu);

void div(Cpu* cpu);

void sqr(Cpu* cpu);

// --------------------------------------------------------------------

void bne(Cpu* cpu);

void beq(Cpu* cpu);

void bgt(Cpu* cpu);

void blt(Cpu* cpu);

void bge(Cpu* cpu);

void ble(Cpu* cpu);

void baw(Cpu* cpu);

// --------------------------------------------------------------------

void str(Cpu* cpu);

void ldr(Cpu* cpu);

// --------------------------------------------------------------------
void cfn(Cpu* cpu);

void ret(Cpu* cpu);

// --------------------------------------------------------------------

void hlt(Cpu* cpu);

#endif
