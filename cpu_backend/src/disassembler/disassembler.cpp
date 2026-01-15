#include "instructions/instructions.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//--------------------------Debug--------------------------
void print_bin_instruction(BinInstructionArray* bin_instructions_array){
    printf("Bin representation:\n");
    for(int count = 0; count < bin_instructions_array->count; count ++){
        BinInstruction* bin_instruction = &bin_instructions_array->bin_instruction_list[count];
        printf("%b %b %b %b\n", bin_instruction->operation, bin_instruction->arg_list[0], bin_instruction->arg_list[1], bin_instruction->arg_list[2]);
    }
}

//--------------------------Debug--------------------------

// TODO: Make aliпed print
void print_text_instruction(TextInstruction* text_instruction){
    printf("%x %s    ", text_instruction->address, text_instruction->operation.operation_name);
    for(uint32_t j = 0; j < text_instruction->operation.num_of_args; j ++){
        printf("%s%x    ", text_instruction->imm[j].imm_flag, text_instruction->imm[j].imm);
    }
    printf("\n");
}

void print_parsed_asm(TextInstructionArray* text_instruction_array){
    for(uint32_t i = 0; i < text_instruction_array->count; i++){
        print_text_instruction(&text_instruction_array->text_instruction_list[i]);
    }
}


void read_bin_file(BinInstructionArray* bin_instructions_array, const char* bin_file_name){
    FILE* bin_file = fopen(bin_file_name, "rb");

    if(!bin_file){
        perror("Failed to open");
        abort();
    }

    // Measure file size
    fseek(bin_file, 0, SEEK_END);
    size_t bin_file_size = ftell(bin_file);
    fseek(bin_file, 0, SEEK_SET);

    // Memory allocation
    bin_instructions_array->count = bin_file_size / sizeof(BinInstruction);
    bin_instructions_array->bin_instruction_list = (BinInstruction*)calloc(bin_file_size, 1);

    if(!bin_instructions_array->bin_instruction_list){
        fprintf(stderr, "Error: Cannot allocat memory!");
        abort();
    }

    // Read bin file
    size_t readed = fread(bin_instructions_array->bin_instruction_list, sizeof(BinInstruction), bin_instructions_array->count, bin_file);

    print_bin_instruction(bin_instructions_array);

    if(readed != bin_instructions_array->count){
        fprintf(stderr, "Error: Something went wrong while reading bin file");
        fclose(bin_file);
        abort();
    }
}

void flag_disassemble(BinInstruction* bin_instruction, TextInstruction* text_instruction, int arg_count){
    uint32_t current_operation = bin_instruction->operation;

    // Read all setted flags
    int flag_count = 0;
    int text_flag_count = 0;
    while(flag_count < INSTRUCTIONS_FLAGS_NUMBER){
        uint32_t flag_mask = 1 << (16 - arg_count * 8 + flag_count);

        if((current_operation & flag_mask) == flag_mask){
            text_instruction->imm[arg_count].imm_flag[text_flag_count] = instruction_flag[flag_count];
            text_flag_count ++;
        }

        flag_count ++;
    }

    // Fill rest of flags buffer by spaces
    if(flag_count != text_flag_count){
        for(int i = text_flag_count; i < flag_count; i ++){
            text_instruction->imm[arg_count].imm_flag[i] = ' ';
        }
    }
}

void instruction_disassemble(BinInstruction* bin_instruction, TextInstruction* text_instruction){
    // Instruction name disassemble
    uint32_t instruction_code = (bin_instruction->operation >> 24) & 0xFF;

    text_instruction->operation.op_code     = instruction_code;
    text_instruction->operation.num_of_args = instruction_set[instruction_code].num_of_args;
    if(instruction_code < 20){
        strcpy(text_instruction->operation.operation_name, instruction_set[instruction_code].op_name);
    }
    else{
        printf("\nStrange code: %d\n", instruction_code);
    }

    // Instruction imms disassemble
    for(int imm_count = 0; imm_count < text_instruction->operation.num_of_args; imm_count ++){
        flag_disassemble(bin_instruction, text_instruction, imm_count);
        
        // Imm value disassembling
        text_instruction->imm[imm_count].imm = bin_instruction->arg_list[imm_count];
    }
}
void bin_file_disassemble(BinInstructionArray* bin_instruction_array, TextInstructionArray* text_instruction_array){
    text_instruction_array->count = 0;
    text_instruction_array->capacity = bin_instruction_array->count;

    text_instruction_array->text_instruction_list = (TextInstruction*)calloc(text_instruction_array->capacity, sizeof(TextInstruction));

    if(!text_instruction_array->text_instruction_list){
        fprintf(stderr, "Error while text instruction list buffer allocating!");
        abort();
    }

    // Use the actual number of bin instructions as the limit
    while(text_instruction_array->count < bin_instruction_array->count){
        text_instruction_array->text_instruction_list[text_instruction_array->count].address = text_instruction_array->count * sizeof(BinInstruction);
        instruction_disassemble(&bin_instruction_array->bin_instruction_list[text_instruction_array->count], 
                                &text_instruction_array->text_instruction_list[text_instruction_array->count]);

        text_instruction_array->count++;
    }
}

//-----------------------------------------------------------

int main(int argc, char* argv[]){
    const char* bin_asm_file_name;
    const char* text_asm_file_name;

    if(argc == 2){
        bin_asm_file_name = argv[1];
        text_asm_file_name = NULL;
    }

    else if(argc == 3){
        bin_asm_file_name = argv[1];
        text_asm_file_name = argv[2];
    }

    else{
        fprintf(stderr, "Error: Wrong arg number");
        abort();
    }

    BinInstructionArray bin_instr_arr;
    BinInstructionArray* bin_instructions_array = & bin_instr_arr;

    TextInstructionArray text_instr_arr;
    TextInstructionArray* text_instructions_array = & text_instr_arr;

    read_bin_file(bin_instructions_array, bin_asm_file_name);

    bin_file_disassemble(bin_instructions_array, text_instructions_array);

    if(!text_asm_file_name){
        print_parsed_asm(text_instructions_array);
    }

    free(bin_instructions_array->bin_instruction_list);
    free(text_instructions_array->text_instruction_list);

    return 0;
}
