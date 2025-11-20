#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cpu_emulator/cpu.h"
#include "cpu_emulator/cpu_instructions.h"
#include "instructions/instructions.h"
#include "stack/stack.h"
#include "errors/errors.h"

uint32_t* get_uint_from_stack(Stack* stack, uint32_t address){
    uint32_t* uint_pointer;

    TRY{
        uint_pointer = (uint32_t*)stack_get_element(stack, address + 4) - 4;
    }

    CATCH(ERR_ELEM_INDEX_OUT_OF_RANGE){
        fprintf(stderr, "Stack overflow!\n");
        abort();
    }

    END_TRY;

    return uint_pointer;
}

void loader(const char* bin_file_name, Stack* stack){
    FILE* bin_file = fopen(bin_file_name, "r");

    if(!bin_file){
        fprintf(stderr, "Error while bin file opening!");
        abort();
    }

    char* readed_bin_asm = NULL;
    size_t bin_asm_len = 0;
    ssize_t readed;

    readed = getline(&readed_bin_asm, &bin_asm_len, bin_file);

    if(readed == -1){
        fprintf(stderr, "Error while reading bin file!");
        abort();
    }

    // Verify, that readed file conatin less then 2^32 instructions
    if(bin_asm_len / 4 > UINT32_MAX){
        fprintf(stderr, "Bin file is to large and cannot be addressed due to cpu size restrictions!");
        abort();
    }

    // Load bin file on stack
    for(size_t instr_count = 0; instr_count < (bin_asm_len / 4); instr_count ++){
        stack_push(stack, (void*)(readed_bin_asm + instr_count * BIN_INSTRUCTION_SIZE));
    }


    // Deallocate readed_bin_asm buffer
    free(readed_bin_asm);
}

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
    cpu->stack = stack_init(1000 * BIN_INSTRUCTION_SIZE, sizeof(uint8_t));

    // Program loading
    loader(bin_file_name, cpu->stack);

    // Program execution

    return 0;
}



