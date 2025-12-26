#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cpu_emulator/cpu.h"
#include "cpu_emulator/cpu_instructions.h"
#include "instructions/instructions.h"
#include "exceptions/exceptions.h"
#include "stack/stack.h"
#include "errors/errors.h"



//--------------------------------------------------------------------------------------------------------
void instruction_execute(Cpu* cpu){
    uint8_t op_code = *(uint8_t*)stack_get_element(cpu->stack, cpu->regs[16]);

    instruction_set[op_code].cpu_instruction_pointer(cpu);
}

void cpu_execute(Cpu* cpu){
    while(cpu->running){
        instruction_execute(cpu);
    }
}

int main(int argc,char* argv[]){
    // Check number of args and set bin_file_name
    if(argc != 2){
        fprintf(stderr, "Wrong number of args!\n");
        abort();
    }

    const char* bin_file_name = argv[1];

    // Initialize buffer for registers
    Cpu cpu_struct;
    Cpu* cpu = &cpu_struct;


    // Initialize stack
    Stack stk;
    Stack* stack = &stk;
    stack_init_from_file(stack, sizeof(uint8_t), bin_file_name);
    cpu->stack = stack;

    // Program loading
    stack_dump(cpu->stack);
    stack_free(cpu->stack);
    // Program execution

    return 0;
}



    
