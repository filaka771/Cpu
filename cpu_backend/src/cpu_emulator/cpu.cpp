#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cpu_emulator/cpu.h"
#include "cpu_emulator/cpu_instructions.h"
#include "instructions/instructions.h"
#include "exceptions/exceptions.h"
#include "stack/stack.h"
#include "errors/errors.h"


//-------------------------DEBUG-------------------------
static void cpu_state(Cpu* cpu){
    printf("Registers values:\n");
    for(int i = 0; i < NUM_OF_REGISTERS - 3; i ++){
        printf("reg %d: %x\n", i, cpu->regs[i]);
    }

    printf("Rbp: %x\n", cpu->regs[RBP]);
    printf("Rpc: %x\n", cpu->regs[RPC]);
    printf("Rcc: %x\n", cpu->regs[RCC]);

    printf("\n Cpu state: %b\n", cpu->running);
    stack_dump(cpu->data_stack);
}

//-------------------------CPU-------------------------
static void load_program_data(Cpu* cpu, const char* file_name) {
    FILE* file = fopen(file_name, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file!");
        cpu_deinitialize(cpu);
        abort();
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    const size_t max_program_size = 16 * 65536;
    
    if (file_size > max_program_size) {
        fprintf(stderr, "Error: File '%s' is too large (%ld bytes > %zu bytes max)\n",
                file_name, file_size, max_program_size);
        fprintf(stderr, "Execution aborted: program exceeds maximum size!\n");
        fclose(file);
        cpu_deinitialize(cpu);
        abort();
    }
    
    if (file_size == 0) {
        fprintf(stderr, "Error: File '%s' is empty!\n", file_name);
        fclose(file);
        cpu_deinitialize(cpu);
        abort();
    }
    
    // Read directly into CPU buffer
    size_t bytes_read = fread(cpu->program_buffer, 1, file_size, file);
    
    if (bytes_read != file_size) {
        if (feof(file)) {
            fprintf(stderr, "Error: Unexpected end of file!\n");
        } else if (ferror(file)) {
            perror("Error reading file!");
        }
        fclose(file);
        cpu_deinitialize(cpu);
        abort();
    }
    
    fclose(file);
    
}

static void cpu_init(Cpu* cpu, Stack* data_stack, Stack* call_stack, const char* bin_file_name){
    // Initialize data buffer
    cpu->program_buffer = (uint8_t*)calloc(CODE_MEM_SIZE, sizeof(uint8_t));
    load_program_data(cpu, bin_file_name);

    // Initialize stacks
    stack_init(data_stack, DATA_STACK_INIT_SIZE, sizeof(uint8_t));
    cpu->data_stack = data_stack;

    stack_init(call_stack, CALL_STACK_INIT_SIZE, sizeof(uint32_t));
    cpu->call_stack = call_stack;
    // Cpu state
    cpu->running = true;

    // Initialize all regs with zero values
    for (int i = 0; i < NUM_OF_REGISTERS; i++){
        cpu->regs[i] = 0;
    }
}


void instruction_execute(Cpu* cpu){
    uint32_t op_code = *(uint32_t*)(cpu->program_buffer + cpu->regs[RPC]);
    op_code = (op_code >> 24) & 0xFF;
    //printf("Instruction: %s\n\n", instruction_set[op_code].op_name);

    instruction_set[op_code].cpu_instruction_pointer(cpu);
}


void cpu_execute(Cpu* cpu) {
    while(cpu->running) {
        instruction_execute(cpu);
        cpu_state(cpu);
        cpu->regs[RCC] ++;
    }
}

int main(int argc,char* argv[]){
    // Check number of args and set bin_file_name
    if(argc != 2){
        fprintf(stderr, "Wrong number of args!\n");
        //cpu_deinitialize(cpu);
        abort();
    }

    const char* bin_file_name = argv[1];

    // Cpu initialization
    Cpu cpu_struct;
    Cpu* cpu = &cpu_struct;

    Stack d_stk;
    Stack* data_stack = &d_stk;

    Stack c_stk;
    Stack* call_stack = &c_stk;

    cpu_init(cpu, data_stack, call_stack, bin_file_name);


    cpu_execute(cpu);

    //stack_dump(cpu->stack);
    stack_free(cpu->data_stack);
    stack_free(cpu->call_stack);
    return 0;
}



    
