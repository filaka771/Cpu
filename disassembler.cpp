#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "instructions.h"

#define INIT_SIZE 1000

void instruction_addres_disassemble(TextInstruction* text_instruction){
    text_instruction->address = 0;
}

// Instruction name disassemble
void instruction_name_disassemble(TextInstruction* text_instruction, Instruction* instruction, const InstructionSet* instruction_set){
    uint32_t op_code = (uint32_t)(instruction->op_name & 0x1F);
    if (op_code < INSTRUCTIONS_SET_NUMBER){
        text_instruction->operation.num_of_args = instruction_set[op_code].num_of_args;
        text_instruction->operation.operation_name = instruction_set[op_code].op_name;
        return;
    }
        //TODO: THROW ERROR WITH INSTRUCTION NAME
    else{
        printf("Error");
        return;
    }
}

// Instruction arg disassemble (arg_position 0, 1 or 2)
void instruction_argument_disassemble(TextInstruction* text_instruction, Instruction* instruction, uint8_t arg_position){
    bool op_mem = instruction->op_name & (1 << (5 + arg_position * 2));
    bool op_reg = instruction->op_name & (1 << (5 + arg_position * 2 + 1));

    // Arg value set
    text_instruction->imm[arg_position].imm = *((int32_t*)instruction + arg_position + 1);

    // Arg value set
    if(op_mem && op_reg){
        text_instruction->imm[arg_position].imm_flag = instruction_flag[0];
        return;
    }

    if(op_mem && !op_reg){
        text_instruction->imm[arg_position].imm_flag = instruction_flag[1];
        return;
    }

    if(!op_mem && op_reg){
        text_instruction->imm[arg_position].imm_flag = instruction_flag[2];
        return;
    }

    if(!op_mem && !op_reg){
        text_instruction->imm[arg_position].imm_flag = instruction_flag[3];
        return;
    }

}

//-----------------------------Read_bin_file-------------------------------

InstructionArray* read_bin_file(char* bin_file_name){
    InstructionArray instr_arr;
    InstructionArray* instruction_array = &instr_arr;

    FILE *file = fopen(bin_file_name, "rb");
    if (!file){
        perror("Failed to open file");
        return NULL;
    }

    // Determine file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Calculate number of instructions
    instruction_array->count = file_size / sizeof(Instruction);
    instruction_array->instruction_list = (Instruction*)calloc(file_size, 1);
    if (!instruction_array->instruction_list){
        perror("malloc failed");
        fclose(file);
        return NULL;
    }

    // Read all instructions at once
    size_t read = fread(instruction_array->instruction_list, sizeof(Instruction), instruction_array->count, file);
    if (read != instruction_array->count){
        perror("fread failed");
        free(instruction_array);
        fclose(file);
        return NULL;
    }

    return instruction_array;
}
/*
  void instructions_array_dump(Instruction* instructions, size_t num_instructions){
  for (size_t i = 0; i < num_instructions; i++){
  printf(
  "Instruction %zu: op_name=%u, imm_0=%u, imm_1=%u, imm_2=%u\n",
  i,
  instructions[i]->op_name,
  instructions[i]->imm_0,
  instructions[i]->imm_1,
  instructions[i]->imm_2
  );
  }
 */

//-------------------------------------Disassembler------------------------
void instruction_disassemble(TextInstruction* text_instruction, Instruction* instruction, const InstructionSet* instruction_set){
    instruction_addres_disassemble(text_instruction);
    instruction_name_disassemble(text_instruction, instruction, instruction_set);

    for(int arg_position = 0; arg_position < text_instruction->operation.num_of_args; arg_position++ ){
        instruction_argument_disassemble(text_instruction, instruction, arg_position);
    }
}

void print_instruction(TextInstruction* text_instruction){
    printf("\n%x    %s", text_instruction->address, text_instruction->operation.operation_name);
    for (int i = 0; i < text_instruction->operation.num_of_args; i++){
        if(text_instruction->imm[i].imm_flag == " x" || text_instruction->imm[i].imm_flag == "*x")
            printf(" %s%x ", text_instruction->imm[i].imm_flag, text_instruction->imm[i].imm);
        else

            printf(" %s%d ", text_instruction->imm[i].imm_flag, text_instruction->imm[i].imm);
    }
    printf("\n");

}


//-----------------------------------Test------------------------
/*
  int main(){
  Instruction instr;
  Instruction* instruction = &instr;
  TextInstruction text_instr;
  TextInstruction* text_instruction = &text_instr;

    instruction->op_name = 167772160;
    instruction->imm[0] = 0;
    instruction->imm[1] = 0;
    instruction->imm[2] = 0;

    instruction_disassemble(text_instruction,instruction,instruction_set);
    print_instruction(text_instruction);

    return 0;
    }
 */

int main(int argc, char** argv){
    char* file_name = argv[2];
    InstructionArray* instruction_array = read_bin_file(file_name);
    TextInstruction* text_instruction_array = (TextInstruction*)calloc(instruction_array->count, sizeof(TextInstruction));

    for(uint32_t i = 0; i < instruction_array->count; i ++){
        instruction_disassemble(text_instruction_array + i, instruction_array + i, instruction_set);
        print_instruction(text_instruction_array + i);
    }

    free(instruction_array->instruction_list);
    free(text_instruction_array)




    return 0;
}









