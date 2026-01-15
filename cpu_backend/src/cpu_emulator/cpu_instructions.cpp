#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "instructions/instructions.h"
#include "cpu_emulator/cpu.h"
#include "cpu_emulator/cpu_instructions.h"
#include "stack/stack.h"

void cpu_deinitialize(Cpu* cpu){
    stack_free(cpu->call_stack);
    stack_free(cpu->data_stack);
}

static void read_bin_arg(Cpu* cpu, CpuInstructionArgs* cpu_instruction_args, int arg_count) {
    uint32_t arg_value = *(uint32_t*)(cpu->program_buffer + cpu->regs[RPC] + sizeof(uint32_t) * (arg_count + 1));
    
    cpu_instruction_args->cpu_imm_args[arg_count].cpu_imm_arg = arg_value;
    
    uint32_t header = *(uint32_t*)(cpu->program_buffer + cpu->regs[RPC]);
    
    int byte_position = 16 - (arg_count * 8);
    
    if (header & (1 << (byte_position + 1))) {
        if (arg_value >= NUM_OF_REGISTERS) {
            fprintf(stderr, "Cpu contain only %u general purpose registers! Given reg number is %u\n", NUM_OF_REGISTERS, arg_value);
            cpu_deinitialize(cpu);
            abort();
        }
        cpu_instruction_args->cpu_imm_args[arg_count].in_reg = true;
    }
    else {
        cpu_instruction_args->cpu_imm_args[arg_count].in_reg = false;
    }

    if (header & (1 << (byte_position + 0))) {
        cpu_instruction_args->cpu_imm_args[arg_count].abst_op = true;
    }
    else {
        cpu_instruction_args->cpu_imm_args[arg_count].abst_op = false;
    }
}

static void get_args(Cpu* cpu, CpuInstructionArgs* cpu_instruction_args, int num_of_args) {
    // Read opcode from first byte
    uint32_t instruction_header = *(uint32_t*)(cpu->program_buffer + cpu->regs[RPC]);
    cpu_instruction_args->cpu_op_code = (instruction_header >> 24) & 0xFF;
    //printf("Op code: %d\n", cpu_instruction_args->cpu_op_code);
    
    for (int arg_count = 0; arg_count < num_of_args; arg_count++) {
        read_bin_arg(cpu, cpu_instruction_args, arg_count);
    }
}

//-------------------------------------------------------------------------
uint32_t* get_uint_from_stack(Stack* stack, uint32_t position){
    //printf("DEBUG get_uint: position=%u, stack->count=%zu\n", position, stack->count);
    
    if(position > stack->count - sizeof(uint32_t)){
        fprintf(stderr, "ERROR: position %u > count %zu - %zu\n", 
               position, stack->count, sizeof(uint32_t));
        fprintf(stderr, "No read access to memory that out of the stack!");
        //cpu_deinitialize(cpu);
        abort();
    }

    uint32_t* uint_ptr = (uint32_t*)stack_get_element(stack, position);
    //printf("DEBUG: Read value: 0x%08x\n", *uint_ptr);
    
    return uint_ptr;
}

void write_uint_on_stack(Stack* stack, uint32_t position, uint32_t value){
    //printf("DEBUG write_uint: position=%u, value=0x%08x\n", position, value);
    
    for(int byte_count = 0; byte_count < sizeof(uint32_t); byte_count ++){
        stack_modify_element(stack, (uint8_t*)(&value) + byte_count, position + byte_count);
    }
}

void pop_uint_from_stack(Stack* stack){
    for(int i = 0; i < sizeof(uint32_t); i ++){
        stack_pop(stack);
    }
}


uint32_t read_from_memory(Cpu* cpu, CpuInstructionArg* cpu_instruction_arg){
    if(cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg){
        // Both flags set: indirect register addressing
        // cpu_imm_arg is a register number, whose value is an address in stack
        if(cpu_instruction_arg->cpu_imm_arg >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!", NUM_OF_REGISTERS);
            cpu_deinitialize(cpu);
            abort();
        }
        uint32_t address = cpu->regs[cpu_instruction_arg->cpu_imm_arg];
        return *get_uint_from_stack(cpu->data_stack, address);
    }
    else if(cpu_instruction_arg->in_reg){
        // Only in_reg flag: direct register access
        if(cpu_instruction_arg->cpu_imm_arg >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!", NUM_OF_REGISTERS);
            cpu_deinitialize(cpu);
            abort();
        }
        return cpu->regs[cpu_instruction_arg->cpu_imm_arg];
    }
    else if(cpu_instruction_arg->abst_op){
        // Only abst_op flag: direct memory address
        return *get_uint_from_stack(cpu->data_stack, cpu_instruction_arg->cpu_imm_arg);
    }
    else{
        // Neither flag: immediate value
        return cpu_instruction_arg->cpu_imm_arg;
    }
}

bool is_addresseble(CpuInstructionArg* cpu_instruction_arg){
    //printf("Abst op: %b\n In reg: %b\n", cpu_instruction_arg->abst_op, cpu_instruction_arg->in_reg);
    if(!cpu_instruction_arg->abst_op && !cpu_instruction_arg->in_reg)
        return false;

    return true;
}

bool is_register(CpuInstructionArg* cpu_instruction_arg){
    if(!cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg && cpu_instruction_arg->cpu_imm_arg < NUM_OF_REGISTERS)
        return true;

    return false;
}

void write_in_memory(Cpu* cpu, CpuInstructionArg* cpu_instruction_arg, uint32_t value){
    if(!is_addresseble(cpu_instruction_arg)){
        fprintf(stderr, "Arg which used as address for writing value cannot be value!\nIt must be address or stack or register!\n");
        cpu_deinitialize(cpu);
        abort();
    }

    if(cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg){
        // Indirect register addressing
        uint32_t reg_num = cpu_instruction_arg->cpu_imm_arg;
        if(reg_num >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!\n", NUM_OF_REGISTERS);
            cpu_deinitialize(cpu);
            abort();
        }
        uint32_t address = cpu->regs[reg_num];
        write_uint_on_stack(cpu->data_stack, address, value);
    }
    else if(cpu_instruction_arg->in_reg){
        // Direct register access
        uint32_t reg_num = cpu_instruction_arg->cpu_imm_arg;

        if(reg_num >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!\n", NUM_OF_REGISTERS);
            cpu_deinitialize(cpu);
            abort();
        }
        cpu->regs[reg_num] = value;
    }
    else if(cpu_instruction_arg->abst_op){
        // Direct memory address
        write_uint_on_stack(cpu->data_stack, cpu_instruction_arg->cpu_imm_arg, value);
    }
}

//--------------------------------------------------------------------------


void inp(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[0].num_of_args);

    uint32_t value;
    scanf("%x", &value);

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], value);

    cpu->regs[RPC] += 16;
}

void out(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[1].num_of_args);

    uint32_t value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    printf("%x\n", value);

    cpu->regs[RPC] += 16;
}


// --------------------------------------------------------------------
void mov(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[2].num_of_args);

    uint32_t src_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], src_value);

    cpu->regs[RPC] += 16;
}

void add(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[3].num_of_args);

    uint32_t src1_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t src2_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[2]);

    uint32_t result = src1_value + src2_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], result);

    cpu->regs[RPC] += 16;
}

void sub(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[4].num_of_args);

    uint32_t src1_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t src2_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[2]);

    uint32_t result = src1_value - src2_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], result);

    cpu->regs[RPC] += 16;
}

void mul(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[5].num_of_args);

    uint32_t src1_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t src2_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[2]);

    uint32_t result = src1_value * src2_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], result);
    
    cpu->regs[RPC] += 16;
}

void div(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[6].num_of_args);

    uint32_t src1_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t src2_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[2]);

    uint32_t result = src1_value / src2_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], result);

    cpu->regs[RPC] += 16;
}

void sqr(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[7].num_of_args);

    uint32_t src_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    
    uint32_t result = (uint32_t)sqrt((double)src_value + 0.5);

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], result);

    cpu->regs[RPC] += 16;
}

// --------------------------------------------------------------------
// First two args compaired, last used as adr
void bne(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[8].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 != value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void beq(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[9].num_of_args);
    
    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 == value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void bgt(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[10].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 > value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void blt(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[11].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 < value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void bge(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[12].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 >= value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void ble(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[13].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 <= value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void baw(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;
    get_args(cpu, &cpu_instruction_args, instruction_set[14].num_of_args);

    cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[0].cpu_imm_arg;
}

// --------------------------------------------------------------------

void str(Cpu* cpu) {
    CpuInstructionArgs cpu_instruction_args;
    get_args(cpu, &cpu_instruction_args, 1);  // Assuming index 11 for str
    
    uint32_t reg_num = cpu_instruction_args.cpu_imm_args[0].cpu_imm_arg;
    uint32_t value = cpu->regs[reg_num];
    
    //printf("DEBUG str: Writing value 0x%08x to stack\n", value);
    
    write_uint_on_stack(cpu->data_stack, cpu->data_stack->count, value);
    
    //printf("DEBUG str: New stack count = %zu\n", cpu->data_stack->count);
    
    cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void ldr(Cpu* cpu) {
    CpuInstructionArgs cpu_instruction_args;
    get_args(cpu, &cpu_instruction_args, 1);  // Assuming index 12 for ldr
    
    //printf("DEBUG ldr: Stack count before = %zu\n", cpu->data_stack->count);
    
    if (cpu->data_stack->count < 4) {  // Need at least 4 bytes
        fprintf(stderr, "Stack underflow in ldr\n");
        cpu_deinitialize(cpu);
        abort();
    }
    
    // Read from position count-4 (last 4 bytes)
    uint32_t position = cpu->data_stack->count - 4;
    uint32_t* value_ptr = get_uint_from_stack(cpu->data_stack, position);
    
    uint32_t reg_num = cpu_instruction_args.cpu_imm_args[0].cpu_imm_arg;
    cpu->regs[reg_num] = *value_ptr;
    
    //printf("DEBUG ldr: Read value 0x%08x\n", *value_ptr);
    
    pop_uint_from_stack(cpu->data_stack);
    
    //printf("DEBUG ldr: New stack count = %zu\n", cpu->data_stack->count);
    
    cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

  // --------------------------------------------------------------------

void cfn(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[17].num_of_args);

    uint32_t ret_address = cpu->regs[RPC] + BIN_INSTRUCTION_SIZE;
    stack_push(cpu->call_stack, &ret_address);
    cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[0].cpu_imm_arg;
}

void ret(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[17].num_of_args);

    cpu->regs[RPC] = *(uint32_t*)stack_get_element(cpu->call_stack, (cpu->call_stack->count - 1));
    stack_pop(cpu->call_stack);
}

// --------------------------------------------------------------------

void hlt(Cpu* cpu){
    cpu->running = false;
}
