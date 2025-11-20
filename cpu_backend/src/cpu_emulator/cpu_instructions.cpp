#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "instructions/instructions.h"
#include "cpu_emulator/cpu.h"
#include "cpu_emulator/cpu_instructions.h"
#include "stack/stack.h"

static void read_bin_arg(Cpu* cpu, CpuInstructionArgs* cpu_instruction_args, int arg_count){
    cpu_instruction_args->cpu_imm_args[arg_count].cpu_imm_arg =
        *(uint32_t*)stack_get_element(cpu->stack, cpu->regs[16] + arg_count * sizeof(uint32_t));

    uint32_t op_code = *(uint32_t*)stack_get_element(cpu->stack, cpu->regs[16]);
    int byte_position = 16 - (arg_count * 8);

    if(op_code & (1 <<(byte_position + 1))){
        if(cpu_instruction_args->cpu_imm_args[arg_count].cpu_imm_arg >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!", NUM_OF_REGISTERS);
            abort();
        }

        cpu_instruction_args->cpu_imm_args[arg_count].in_reg = true;
    }
    else{
        cpu_instruction_args->cpu_imm_args[arg_count].in_reg = false;
    }


    if(op_code & (1 << byte_position)){
        cpu_instruction_args->cpu_imm_args[arg_count].abst_op = true;
    }
    else{
        cpu_instruction_args->cpu_imm_args[arg_count].abst_op = false;
    }
}

static void get_args(Cpu* cpu, CpuInstructionArgs* cpu_instruction_args, int num_of_args){
    cpu_instruction_args->cpu_op_code = (*(uint32_t*)stack_get_element(cpu->stack, cpu->regs[16]) >> 24);

    for(int arg_count = 0; arg_count < num_of_args; arg_count ++){
        read_bin_arg(cpu, cpu_instruction_args, arg_count);
    }
}

//-------------------------------------------------------------------------
uint32_t* get_uint_from_stack(Stack* stack, uint32_t position){
    uint32_t* uint_ptr = NULL;
    if(position > stack->count - 4){
        fprintf(stderr, "No read access to memory that out of the stack!");
        abort();
    }

    uint_ptr = (uint32_t*)stack_get_element(stack, position);

    return uint_ptr;
}

void write_uint_on_stack(Stack* stack, uint32_t position, uint32_t value){
    for(int byte_count = 0; byte_count < sizeof(uint32_t); byte_count ++){
        stack_modify_element(stack, (uint8_t*)(&value) + byte_count ,position);
    }
}

void pop_uint_from_stack(Stack* stack){
    for(int i = 0; i < BIN_INSTRUCTION_SIZE; i ++){
        stack_pop(stack);
    }
}


uint32_t read_from_memory(Cpu* cpu, CpuInstructionArg* cpu_instruction_arg){
    uint32_t* uint_ptr = NULL;
    if(cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg){
        if(cpu_instruction_arg->cpu_imm_arg >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!", NUM_OF_REGISTERS);
            abort();
        }

        uint_ptr = get_uint_from_stack(cpu->stack, cpu->regs[cpu_instruction_arg->cpu_imm_arg]);
    }

    if(cpu_instruction_arg->in_reg){
        if(cpu_instruction_arg->cpu_imm_arg >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!", NUM_OF_REGISTERS);
            abort();
        }

        uint_ptr = &cpu->regs[cpu_instruction_arg->cpu_imm_arg];
    }

    if(cpu_instruction_arg->abst_op){
        uint_ptr = get_uint_from_stack(cpu->stack, cpu_instruction_arg->cpu_imm_arg);
    }


    else{
        uint_ptr = &cpu_instruction_arg->cpu_imm_arg;
    }

    return *uint_ptr;
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

void write_in_memory(Cpu* cpu, CpuInstructionArg* cpu_instruction_arg, uint32_t value){
    uint32_t position = cpu_instruction_arg->cpu_imm_arg;

    if(!is_addresseble(cpu_instruction_arg)){
        fprintf(stderr, "Arg which used as address for writing value cannot be value!\nIt must be address or stack or register!\n");
        abort();
    }

    if(cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg){
        if(position >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!\n", NUM_OF_REGISTERS);
            abort();
        }

        write_uint_on_stack(cpu->stack, cpu->regs[position], value);
    }

    if(cpu_instruction_arg->in_reg){
        if(position >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!\n", NUM_OF_REGISTERS);
            abort();
        }

        cpu->regs[position] = value;
    }

    if(cpu_instruction_arg->abst_op){
        position = read_from_memory(cpu, cpu_instruction_arg);
        write_uint_on_stack(cpu->stack, position, value);
    }
}


//--------------------------------------------------------------------------


void inp(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[0].num_of_args);

    uint32_t value;
    scanf("%x", &value);

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], value);
}

void out(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[1].num_of_args);

    uint32_t value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    printf("%x", value);
}


// --------------------------------------------------------------------
void mov(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[2].num_of_args);

    uint32_t src_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], src_value);
}

void add(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[3].num_of_args);

    uint32_t src_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t dest_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);

    dest_value += src_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], dest_value);
}

void sub(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[4].num_of_args);

    uint32_t src_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t dest_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);

    dest_value -= src_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], dest_value);
}

void mul(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[5].num_of_args);

    uint32_t src_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t dest_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);

    dest_value *= src_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], dest_value);
}

void div(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[6].num_of_args);

    uint32_t src_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t dest_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);

    dest_value /= src_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], dest_value);
}

void sqr(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[7].num_of_args);

}

// --------------------------------------------------------------------
// First two args compaired, last used as adr
void bne(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[8].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 != value2){
        cpu->regs[16] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void beq(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[9].num_of_args);
    
    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 == value2){
        cpu->regs[16] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void bgt(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[10].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 > value2){
        cpu->regs[16] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void blt(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[11].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 < value2){
        cpu->regs[16] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void bge(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[12].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 >= value2){
        cpu->regs[16] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void ble(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[13].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 <= value2){
        cpu->regs[16] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void baw(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;
    get_args(cpu, &cpu_instruction_args, instruction_set[14].num_of_args);

    cpu->regs[16] = cpu_instruction_args.cpu_imm_args[0].cpu_imm_arg;
}

// --------------------------------------------------------------------

void str(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[15].num_of_args);

    if(!is_register(&cpu_instruction_args[0])){
        fprintf(stderr, "Instruction str requires register as imm!\n");
        abort();
    }

    uint32_t value = read_from_memory(cpu, &cpu_instruction_args[0]);

    write_uint_on_stack(cpu->stack, cpu->regs[16], value);
    cpu->regs[16] += BIN_INSTRUCTION_SIZE;
}

void ldr(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[16].num_of_args);

    if(!is_register(&cpu_instruction_args[0])){
        fprintf(stderr, "Instruction str requires register as imm!\n");
        abort();
    }

    uint32_t value = *get_uint_from_stack(cpu->stack, cpu->regs[16] - BIN_INSTRUCTION_SIZE);

    write_in_memory(cpu, cpu_instruction_args[0], value);
}

// --------------------------------------------------------------------

void bfn(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[17].num_of_args);

    cpu->regs[17] = cpu->regs[16];
    cpu->regs[16] = read_from_memory(cpu, &cpu_instruction_args[0]);
}


void ret(Cpu* cpu){
    cpu->regs[16] = cpu->regs[17];
}

// --------------------------------------------------------------------

void hlt(Cpu* cpu){
    cpu->running = false;
}
