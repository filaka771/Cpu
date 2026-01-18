#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "instructions/instructions.h"
#include "cpu_emulator/cpu.h"
#include "cpu_emulator/cpu_instructions.h"
#include "stack/stack.h"
#include "exceptions/exceptions.h"

//---------------------ERROR_HANDLING---------------------

static void cpu_deinitialize(Cpu* cpu){
    free(cpu->program_buffer);
    stack_free(cpu->call_stack);
    stack_free(cpu->data_stack);
}

void cpu_critical_error(Cpu* cpu, const char* error_message){
    fprintf(stderr, "%s", error_message);
    cpu_deinitialize(cpu);
    abort();
}

//---------------------READ_BIN_ASM---------------------

static void parse_binary_operand(Cpu* cpu, CpuInstructionArgs* cpu_instruction_args, int arg_count) {
    uint32_t arg_value = *(uint32_t*)(cpu->program_buffer + cpu->regs[RPC] + sizeof(uint32_t) * (arg_count + 1));
    
    cpu_instruction_args->cpu_imm_args[arg_count].cpu_imm_arg = arg_value;
    
    uint32_t header = *(uint32_t*)(cpu->program_buffer + cpu->regs[RPC]);
    
    int byte_position = 16 - (arg_count * 8);
    
    if (header & (1 << (byte_position + 1))) {
        if (arg_value >= NUM_OF_REGISTERS) {
            cpu_critical_error(cpu, "Cpu contain only %u general purpose registers! Given reg number is %u\n");
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

static void parse_instruction_operands(Cpu* cpu, CpuInstructionArgs* cpu_instruction_args, int num_of_args) {
    // Read opcode from first byte
    uint32_t instruction_header = *(uint32_t*)(cpu->program_buffer + cpu->regs[RPC]);
    cpu_instruction_args->cpu_op_code = (instruction_header >> 24) & 0xFF;
    
    for (int arg_count = 0; arg_count < num_of_args; arg_count++) {
        parse_binary_operand(cpu, cpu_instruction_args, arg_count);
    }
}

//---------------------STACK_OPERATIONS---------------------

uint32_t* get_uint_from_stack(Cpu* cpu, uint32_t position){
    Stack* stack = cpu->data_stack;
    
    if(position > stack->count - sizeof(uint32_t)){
        fprintf(stderr, "ERROR: position %u > count %zu - %zu\n", 
                position, stack->count, sizeof(uint32_t));
        cpu_critical_error(cpu, "No read access to memory that out of the stack!");
    }

    uint32_t* uint_ptr = NULL;
    TRY{
        uint32_t* uint_ptr = (uint32_t*)stack_get_element(stack, position);
        return uint_ptr;
    }
    CATCH_ALL{
        cpu_critical_error(cpu, "Error while getting uint from stack!");
        return NULL;
    }
    END_TRY;
    
    return uint_ptr;
}

void write_uint_on_stack(Cpu* cpu, uint32_t position, uint32_t value){
    Stack* stack = cpu->data_stack;
    
    for(size_t byte_count = 0; byte_count < sizeof(uint32_t); byte_count ++){
        TRY{
            stack_modify_element(stack, (uint8_t*)(&value) + byte_count, position + byte_count);
        }
        CATCH_ALL{
            cpu_critical_error(cpu, "Error while writing uint on stack!");
        }
        END_TRY;
    }
}

void pop_uint_from_stack(Cpu* cpu){
    Stack* stack = cpu->data_stack;

    for(size_t i = 0; i < sizeof(uint32_t); i ++){
        TRY{
            stack_pop(stack);
        }
        CATCH_ALL{
            cpu_critical_error(cpu, "Error while popping uint from stack!");
        }
        END_TRY;
    }
}


//---------------------RUNTIME_OPERANDS_PROCESSING---------------------

uint32_t get_runtime_operand_value(Cpu* cpu, CpuInstructionArg* cpu_instruction_arg){
    if(cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg){
        // Both flags set: indirect register addressing
        // cpu_imm_arg is a register number, whose value is an address in stack
        if(cpu_instruction_arg->cpu_imm_arg >= NUM_OF_REGISTERS){
            cpu_critical_error(cpu, "Cpu contain only %u general purpose registers!");
        }
        uint32_t address = cpu->regs[cpu_instruction_arg->cpu_imm_arg];
        return *get_uint_from_stack(cpu, address);
    }
    else if(cpu_instruction_arg->in_reg){
        // Only in_reg flag: direct register access
        if(cpu_instruction_arg->cpu_imm_arg >= NUM_OF_REGISTERS){
            cpu_critical_error(cpu, "Cpu contain only %u general purpose registers!");
        }
        return cpu->regs[cpu_instruction_arg->cpu_imm_arg];
    }
    else if(cpu_instruction_arg->abst_op){
        // Only abst_op flag: direct memory address
        return *get_uint_from_stack(cpu, cpu_instruction_arg->cpu_imm_arg);
    }
    else{
        // Neither flag: immediate value
        return cpu_instruction_arg->cpu_imm_arg;
    }
}

bool is_addresseble(CpuInstructionArg* cpu_instruction_arg){
    if(!cpu_instruction_arg->abst_op && !cpu_instruction_arg->in_reg)
        return false;

    return true;
}

bool is_register(CpuInstructionArg* cpu_instruction_arg){
    if(!cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg && cpu_instruction_arg->cpu_imm_arg < NUM_OF_REGISTERS)
        return true;

    return false;
}

void set_runtime_operand_value(Cpu* cpu, CpuInstructionArg* cpu_instruction_arg, uint32_t value){
    if(!is_addresseble(cpu_instruction_arg)){
        cpu_critical_error(cpu, "Arg which used as address for writing value cannot be value!\nIt must be address or stack or register!\n");
    }

    if(cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg){
        // Indirect register addressing
        uint32_t reg_num = cpu_instruction_arg->cpu_imm_arg;
        if(reg_num >= NUM_OF_REGISTERS){
            cpu_critical_error(cpu, "Cpu contain only %u general purpose registers!");
        }
        uint32_t address = cpu->regs[reg_num];
        write_uint_on_stack(cpu, address, value);
    }
    else if(cpu_instruction_arg->in_reg){
        // Direct register access
        uint32_t reg_num = cpu_instruction_arg->cpu_imm_arg;

        if(reg_num >= NUM_OF_REGISTERS){
            cpu_critical_error(cpu, "Cpu contain only %u general purpose registers!");
        }
        cpu->regs[reg_num] = value;
    }
    else if(cpu_instruction_arg->abst_op){
        // Direct memory address
        write_uint_on_stack(cpu, cpu_instruction_arg->cpu_imm_arg, value);
    }
}

//---------------------CPU_INSTRUCTIONS_IMPLEMENTATION---------------------

void inp(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[0].num_of_args);

    uint32_t value;
    scanf("%x", &value);

    set_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0], value);

    cpu->regs[RPC] += 16;
}

void out(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[1].num_of_args);

    uint32_t value = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    printf("%x\n", value);

    cpu->regs[RPC] += 16;
}

void mov(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[2].num_of_args);

    uint32_t src_value = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    set_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0], src_value);

    cpu->regs[RPC] += 16;
}

void add(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[3].num_of_args);

    uint32_t src1_value = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t src2_value = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[2]);

    uint32_t result = src1_value + src2_value;

    set_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0], result);

    cpu->regs[RPC] += 16;
}

void sub(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[4].num_of_args);

    uint32_t src1_value = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t src2_value = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[2]);

    uint32_t result = src1_value - src2_value;

    set_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0], result);

    cpu->regs[RPC] += 16;
}

void mul(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[5].num_of_args);

    uint32_t src1_value = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t src2_value = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[2]);

    uint32_t result = src1_value * src2_value;

    set_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0], result);
    
    cpu->regs[RPC] += 16;
}

void div(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[6].num_of_args);

    uint32_t src1_value = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t src2_value = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[2]);

    if(src2_value == 0)
        cpu_critical_error(cpu, "Division by zero!\n")

    uint32_t result = src1_value / src2_value;

    set_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0], result);

    cpu->regs[RPC] += 16;
}

void sqr(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[7].num_of_args);

    uint32_t src_value = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    
    uint32_t result = (uint32_t)sqrt((double)src_value + 0.5);

    set_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0], result);

    cpu->regs[RPC] += 16;
}

// First two args compaired, last used as adr
void bne(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[8].num_of_args);

    uint32_t value1 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 != value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void beq(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[9].num_of_args);
    
    uint32_t value1 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 == value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void bgt(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[10].num_of_args);

    uint32_t value1 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 > value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void blt(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[11].num_of_args);

    uint32_t value1 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 < value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void bge(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[12].num_of_args);

    uint32_t value1 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 >= value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void ble(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[13].num_of_args);

    uint32_t value1 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = get_runtime_operand_value(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 <= value2)
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;

    else
        cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void baw(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;
    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[14].num_of_args);

    cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[0].cpu_imm_arg;
}

void str(Cpu* cpu) {
    CpuInstructionArgs cpu_instruction_args;
    parse_instruction_operands(cpu, &cpu_instruction_args, 1);  // Assuming index 11 for str
    
    uint32_t reg_num = cpu_instruction_args.cpu_imm_args[0].cpu_imm_arg;
    uint32_t value = cpu->regs[reg_num];
    
    write_uint_on_stack(cpu, cpu->data_stack->count, value);
    
    cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void ldr(Cpu* cpu) {
    CpuInstructionArgs cpu_instruction_args;
    parse_instruction_operands(cpu, &cpu_instruction_args, 1);
    
    if (cpu->data_stack->count < 4) 
        cpu_critical_error(cpu, "Stack underflow in ldr\n");
    
    // Read from position count-4 (last 4 bytes)
    uint32_t position = cpu->data_stack->count - 4;
    uint32_t* value_ptr = get_uint_from_stack(cpu, position);
    
    uint32_t reg_num = cpu_instruction_args.cpu_imm_args[0].cpu_imm_arg;
    cpu->regs[reg_num] = *value_ptr;
    
    pop_uint_from_stack(cpu);
    
    cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

void cfn(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[17].num_of_args);

    uint32_t ret_address = cpu->regs[RPC] + BIN_INSTRUCTION_SIZE;
    stack_push(cpu->call_stack, &ret_address);
    cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[0].cpu_imm_arg;
}

void ret(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    parse_instruction_operands(cpu, &cpu_instruction_args, instruction_set[17].num_of_args);

    cpu->regs[RPC] = *(uint32_t*)stack_get_element(cpu->call_stack, (cpu->call_stack->count - 1));
    stack_pop(cpu->call_stack);
}

void hlt(Cpu* cpu){
    cpu->running = false;
}
