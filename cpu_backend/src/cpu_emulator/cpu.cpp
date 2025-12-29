#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cpu_emulator/cpu.h"
#include "cpu_emulator/cpu_instructions.h"
#include "instructions/instructions.h"
#include "exceptions/exceptions.h"
#include "stack/stack.h"
#include "errors/errors.h"


static void cpu_state(Cpu* cpu){
    printf("Registers values:\n");
    for(int i = 0; i < NUM_OF_REGISTERS - 3; i ++){
        printf("reg %d: %u\n", i, cpu->regs[i]);
    }

    printf("Rbp: %u\n", cpu->regs[RBP]);
    printf("Rpc: %u\n", cpu->regs[RPC]);
    printf("Rcc: %u\n", cpu->regs[RCC]);

    printf("\n Cpu state: %b\n", cpu->running);
    //stack_dump(cpu->stack);
}

//--------------------------------------------------------------------------------------------------------
static void cpu_init(Cpu* cpu, Stack* stack, const char* bin_file_name){
    // Initialize stack
    stack_init_from_file(stack, sizeof(uint8_t), bin_file_name);
    cpu->stack = stack;

    // Cpu state
    cpu->running = true;

    // Counters conditions
    cpu->regs[RBP] = 0;
    cpu->regs[RPC] = 0;
    cpu->regs[RCC] = 0;
}



void instruction_execute(Cpu* cpu){
    uint32_t op_code = *(uint32_t*)stack_get_element(cpu->stack, cpu->regs[RPC]);
    op_code = (op_code >> 24) & 0xFF;

    printf("\nOp code: %u\n", op_code);

    instruction_set[op_code].cpu_instruction_pointer(cpu);
    cpu->regs[RCC] ++;
}

static int debug_step = 0; 

void cpu_execute(Cpu* cpu) {
    bool debug_paused = true;  // Control whether to pause at each instruction
    
    while(cpu->running) {
        instruction_execute(cpu);

        if (DEBUG && debug_paused) {
            printf("\n=== Step %d ===\n", debug_step++);
            cpu_state(cpu);
        
            char response[10];
            printf("\nCommands: [Enter]=next, s=skip (run with pauses), r=run to end, q=quit: ");
        
            if (fgets(response, sizeof(response), stdin)) {
                if (response[0] == 's' || response[0] == 'S') {
                    // Skip this pause but continue pausing at next instructions
                    continue;
                }
                else if (response[0] == 'r' || response[0] == 'R') {
                    // Run to completion without any more pauses
                    debug_paused = false;
                    printf("Running to completion...\n");
                }
                else if (response[0] == 'q' || response[0] == 'Q') {
                    exit(0);
                }
                // Enter key just continues (stays in debug mode)
            }
        }
    }
}


int main(int argc,char* argv[]){
    // Check number of args and set bin_file_name
    if(argc != 2){
        fprintf(stderr, "Wrong number of args!\n");
        abort();
    }

    const char* bin_file_name = argv[1];

    // Cpu initialization
    Cpu cpu_struct;
    Cpu* cpu = &cpu_struct;

    Stack stk;
    Stack* stack = &stk;

    cpu_init(cpu, stack, bin_file_name);


    cpu_execute(cpu);

    //stack_dump(cpu->stack);
    stack_free(cpu->stack);
    return 0;
}



    
