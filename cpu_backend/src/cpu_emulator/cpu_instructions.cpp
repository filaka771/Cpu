#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "instructions/instructions.h"
#include "cpu_emulator/cpu.h"
#include "cpu_emulator/cpu_instructions.h"
#include "stack/stack.h"

static void read_bin_arg(Cpu* cpu, CpuInstructionArgs* cpu_instruction_args, int arg_count) {
    uint32_t arg_value = *(uint32_t*)stack_get_element(cpu->stack, 
                                                       cpu->regs[16] + 4 + (arg_count * sizeof(uint32_t)));
    
    cpu_instruction_args->cpu_imm_args[arg_count].cpu_imm_arg = arg_value;
    
    uint32_t header = *(uint32_t*)stack_get_element(cpu->stack, cpu->regs[16]);
    
    int byte_position = 16 - (arg_count * 8);
    
    if (header & (1 << (byte_position + 1))) {
        if (arg_value >= NUM_OF_REGISTERS) {
            fprintf(stderr, "Cpu contain only %u general purpose registers!", NUM_OF_REGISTERS);
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
    uint32_t instruction_header = *(uint32_t*)stack_get_element(cpu->stack, cpu->regs[RPC]);
    cpu_instruction_args->cpu_op_code = (instruction_header >> 24) & 0xFF;
    
    for (int arg_count = 0; arg_count < num_of_args; arg_count++) {
        read_bin_arg(cpu, cpu_instruction_args, arg_count);
    }

    if(DEBUG){
        printf("RPC %u\n", cpu->regs[RPC]);
        printf("%s: ", instruction_set[cpu_instruction_args->cpu_op_code].op_name);
        for (int i = 0; i < 3; i++){
            printf("arg%d: in_reg %d abst_op: %d arg_value: %u\n", i,
                   cpu_instruction_args->cpu_imm_args[i].in_reg,
                   cpu_instruction_args->cpu_imm_args[i].abst_op,
                   cpu_instruction_args->cpu_imm_args[i].cpu_imm_arg);
        }
        printf("\n");
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


/*
  uint32_t read_from_memory(Cpu* cpu, CpuInstructionArg* cpu_instruction_arg){
  uint32_t* uint_ptr = NULL;
  if(cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg){
  if(cpu_instruction_arg->cpu_imm_arg >= NUM_OF_REGISTERS){
  fprintf(stderr, "2Cpu contain only %u general purpose registers!", NUM_OF_REGISTERS);
  abort();
  }

        uint_ptr = get_uint_from_stack(cpu->stack, cpu->regs[cpu_instruction_arg->cpu_imm_arg]);
        }

    if(cpu_instruction_arg->in_reg){
    if(cpu_instruction_arg->cpu_imm_arg >= NUM_OF_REGISTERS){
    fprintf(stderr, "3Cpu contain only %u general purpose registers!", NUM_OF_REGISTERS);
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
 */
uint32_t read_from_memory(Cpu* cpu, CpuInstructionArg* cpu_instruction_arg){
    if(cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg){
        // Both flags set: indirect register addressing
        // cpu_imm_arg is a register number, whose value is an address in stack
        if(cpu_instruction_arg->cpu_imm_arg >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!", NUM_OF_REGISTERS);
            abort();
        }
        uint32_t address = cpu->regs[cpu_instruction_arg->cpu_imm_arg];
        return *get_uint_from_stack(cpu->stack, address);
    }
    else if(cpu_instruction_arg->in_reg){
        // Only in_reg flag: direct register access
        if(cpu_instruction_arg->cpu_imm_arg >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!", NUM_OF_REGISTERS);
            abort();
        }
        return cpu->regs[cpu_instruction_arg->cpu_imm_arg];
    }
    else if(cpu_instruction_arg->abst_op){
        // Only abst_op flag: direct memory address
        return *get_uint_from_stack(cpu->stack, cpu_instruction_arg->cpu_imm_arg);
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

/*
  void write_in_memory(Cpu* cpu, CpuInstructionArg* cpu_instruction_arg, uint32_t value){
  uint32_t position = cpu_instruction_arg->cpu_imm_arg;

    if(!is_addresseble(cpu_instruction_arg)){
    fprintf(stderr, "Arg which used as address for writing value cannot be value!\nIt must be address or stack or register!\n");
    abort();
    }

    if(cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg){
    if(position >= NUM_OF_REGISTERS){
    fprintf(stderr, "4Cpu contain only %u general purpose registers!\n", NUM_OF_REGISTERS);
    abort();
    }

        write_uint_on_stack(cpu->stack, cpu->regs[position], value);
        }

    if(cpu_instruction_arg->in_reg){
    if(position >= NUM_OF_REGISTERS){
    fprintf(stderr, "5Cpu contain only %u general purpose registers!\n", NUM_OF_REGISTERS);
    abort();
    }

        cpu->regs[position] = value;
        }

    if(cpu_instruction_arg->abst_op){
    position = read_from_memory(cpu, cpu_instruction_arg);
    write_uint_on_stack(cpu->stack, position, value);
    }
    }
 */
void write_in_memory(Cpu* cpu, CpuInstructionArg* cpu_instruction_arg, uint32_t value){
    if(!is_addresseble(cpu_instruction_arg)){
        fprintf(stderr, "Arg which used as address for writing value cannot be value!\nIt must be address or stack or register!\n");
        abort();
    }

    if(cpu_instruction_arg->abst_op && cpu_instruction_arg->in_reg){
        // Indirect register addressing
        uint32_t reg_num = cpu_instruction_arg->cpu_imm_arg;
        if(reg_num >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!\n", NUM_OF_REGISTERS);
            abort();
        }
        uint32_t address = cpu->regs[reg_num];
        write_uint_on_stack(cpu->stack, address, value);
    }
    else if(cpu_instruction_arg->in_reg){
        printf("\n\nWRITE DEBUG\n\n");
        // Direct register access
        uint32_t reg_num = cpu_instruction_arg->cpu_imm_arg;
        printf("Reg num: %u\n", reg_num);

        if(reg_num >= NUM_OF_REGISTERS){
            fprintf(stderr, "Cpu contain only %u general purpose registers!\n", NUM_OF_REGISTERS);
            abort();
        }
        cpu->regs[reg_num] = value;
        printf("Reg value: %u\n", cpu->regs[reg_num]);
    }
    else if(cpu_instruction_arg->abst_op){
        // Direct memory address
        write_uint_on_stack(cpu->stack, cpu_instruction_arg->cpu_imm_arg, value);
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
    printf("%x", value);

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

    uint32_t src_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t dest_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);

    dest_value += src_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], dest_value);

    cpu->regs[RPC] += 16;
}

void sub(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[4].num_of_args);

    uint32_t src_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t dest_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);

    dest_value -= src_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], dest_value);

    cpu->regs[RPC] += 16;
}

void mul(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[5].num_of_args);

    uint32_t src_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t dest_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);

    dest_value *= src_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], dest_value);
    
    cpu->regs[RPC] += 16;
}

void div(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[6].num_of_args);

    uint32_t src_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);
    uint32_t dest_value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);

    dest_value /= src_value;

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], dest_value);

    cpu->regs[RPC] += 16;
}

void sqr(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[7].num_of_args);

    cpu->regs[RPC] += 16;

}

// --------------------------------------------------------------------
// First two args compaired, last used as adr
void bne(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[8].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 != value2){
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void beq(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[9].num_of_args);
    
    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 == value2){
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void bgt(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[10].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 > value2){
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void blt(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[11].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 < value2){
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void bge(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[12].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 >= value2){
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void ble(Cpu* cpu){

    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[13].num_of_args);

    uint32_t value1 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
    uint32_t value2 = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[1]);

    if(value1 <= value2){
        cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[2].cpu_imm_arg;
    }
}

void baw(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;
    get_args(cpu, &cpu_instruction_args, instruction_set[14].num_of_args);

    cpu->regs[RPC] = cpu_instruction_args.cpu_imm_args[0].cpu_imm_arg;
}

// --------------------------------------------------------------------

void str(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[15].num_of_args);

    if(!is_register(&cpu_instruction_args.cpu_imm_args[0])){
        fprintf(stderr, "Instruction str requires register as imm!\n");
        abort();
    }

    uint32_t value = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);

    write_uint_on_stack(cpu->stack, cpu->regs[RPC], value);
    cpu->regs[RPC] += BIN_INSTRUCTION_SIZE;
}

/*
  void ldr(Cpu* cpu){
  CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[16].num_of_args);

    if(!is_register(&cpu_instruction_args.cpu_imm_args[0])){
    fprintf(stderr, "Instruction str requires register as imm!\n");
    abort();
    }

    uint32_t value = *get_uint_from_stack(cpu->stack, cpu->regs[16] - BIN_INSTRUCTION_SIZE);

    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], value);
    }
 */
void ldr(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;
    get_args(cpu, &cpu_instruction_args, instruction_set[16].num_of_args);

    if(!is_register(&cpu_instruction_args.cpu_imm_args[0])){
        fprintf(stderr, "Instruction ldr requires register as destination!\n");
        abort();
    }

    // Check bounds before accessing
    uint32_t read_pos = cpu->regs[RPC] - BIN_INSTRUCTION_SIZE;
    if(read_pos > cpu->stack->count - sizeof(uint32_t)){
        fprintf(stderr, "Stack underflow in ldr instruction!\n");
        abort();
    }

    uint32_t value = *get_uint_from_stack(cpu->stack, read_pos);
    write_in_memory(cpu, &cpu_instruction_args.cpu_imm_args[0], value);
}

// --------------------------------------------------------------------

void cfn(Cpu* cpu){
    CpuInstructionArgs cpu_instruction_args;

    get_args(cpu, &cpu_instruction_args, instruction_set[RBP].num_of_args);

    cpu->regs[RBP] = cpu->regs[RPC];
    cpu->regs[RPC] = read_from_memory(cpu, &cpu_instruction_args.cpu_imm_args[0]);
}

void ret(Cpu* cpu){
    cpu->regs[RPC] = cpu->regs[RBP];
}

// --------------------------------------------------------------------

void hlt(Cpu* cpu){
    cpu->running = false;
}
