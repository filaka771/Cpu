#include "../../include/instructions.h"
#include "./text_asm_parser/text_asm_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
void assemble_flag(uint32_t* bin_operation, const char* text_flag, int arg_count) {
    int byte_position = 16 - (arg_count * 8);

    for (int flag_idx = 0; flag_idx < INSTRUCTIONS_FLAGS_NUMBER && text_flag[flag_idx] != '\0'; flag_idx++) {
        if (text_flag[flag_idx] == ' ') {
            continue;  
        }
        
        for (int i = 0; i < INSTRUCTIONS_FLAGS_NUMBER; i++) {
            if (text_flag[flag_idx] == instruction_flag[i]) {
                *bin_operation |= (1 << (byte_position + i));
                break;
            }
        }
    }
}


void assemble_text_instruction(BinInstruction* bin_instruction, TextInstruction* text_instruction){
    bin_instruction->operation = 0;
    
    bin_instruction->operation |= (text_instruction->operation.op_code << 24);
    
    printf("op code: %d  arg list: %s  bin rep: %b\n", text_instruction->operation.op_code,
           instruction_set[text_instruction->operation.op_code].op_name,
           (bin_instruction->operation >> 24) & 0xFF);
    for(uint32_t arg_count = 0; arg_count < text_instruction->operation.num_of_args; arg_count++){
        bin_instruction->arg_list[arg_count] = text_instruction->imm[arg_count].imm;
        
        assemble_flag(&bin_instruction->operation,
                      text_instruction->imm[arg_count].imm_flag,
                      arg_count);
    }
}

//--------------------DEBUG----------------------
void print_bin_instruction(BinInstruction* bin_instruction){
    printf("%b %b %b %b\n", bin_instruction->operation, bin_instruction->arg_list[0], bin_instruction->arg_list[1], bin_instruction->arg_list[2]);
}
//--------------------DEBUG----------------------
void assemble_text_instructions_array(BinInstructionArray* bin_instructions_array, TextInstructionArray* text_instructions_array){
    for(uint32_t instructions_count = 0; instructions_count < text_instructions_array->count; instructions_count ++){
        assemble_text_instruction(&bin_instructions_array->bin_instruction_list[instructions_count], &text_instructions_array->text_instruction_list[instructions_count]);
        print_bin_instruction(&bin_instructions_array->bin_instruction_list[instructions_count]);
    }
}


void write_bin_file(BinInstructionArray* bin_instructions_array, const char* output_bin_file){
    int fd = open(output_bin_file, O_RDWR | O_CREAT | O_TRUNC, 0644);  
    if(fd == -1){
        fprintf(stderr, "Error opening output file");
        abort();
    }

    size_t bin_file_size = bin_instructions_array->count * sizeof(BinInstruction);

    if(ftruncate(fd, bin_file_size) == -1){
        close(fd);
        fprintf(stderr, "Error truncating file");
        abort();
    }

    BinInstruction* mapped_mem = (BinInstruction*)mmap(NULL, bin_file_size, PROT_WRITE, MAP_SHARED, fd, 0);
    if(mapped_mem == MAP_FAILED){
        close(fd);
        fprintf(stderr, "Cannot map memory for bin file");
        abort();
    }

    memcpy(mapped_mem, bin_instructions_array->bin_instruction_list, bin_file_size);
    
    // Ensure data is written to disk
    msync(mapped_mem, bin_file_size, MS_SYNC);
    
    munmap(mapped_mem, bin_file_size);
    close(fd);
}


int main(int argc, char* argv[]){
    if(argc == 1){
        fprintf(stderr, "Man!!!!!!!!!!!!!!");
        abort();
    }

    const char* text_asm_file = NULL;
    const char* output_bin_file = NULL;
    

    if(argc == 3){
        text_asm_file = argv[1];
        output_bin_file = argv[2];
    }

    else{

        fprintf(stderr,"To many args");
        abort();
    }

    TextInstructionArray text_instr_arr;
    TextInstructionArray* text_instructions_array = & text_instr_arr;


    LabelTable lab_tab;
    LabelTable* label_table = &lab_tab;


    // TODO: In text_instructions_array reallocation handle case, when capacity * 1.5 > UINT_MAX

    // Parse assembler file into text instruction representation
    parse_text_asm_file(text_asm_file, text_instructions_array, label_table);

    BinInstructionArray bin_inst_arr;
    BinInstructionArray* bin_instructions_array = & bin_inst_arr;

    bin_instructions_array->count = text_instructions_array->count;
    bin_instructions_array->bin_instruction_list = (BinInstruction*)calloc( text_instructions_array->count, sizeof(BinInstruction));

    assemble_text_instructions_array(bin_instructions_array, text_instructions_array);


    write_bin_file(bin_instructions_array, output_bin_file);

    free_text_instruction_list(text_instructions_array);
    free_label_table(label_table);

    free(bin_instructions_array->bin_instruction_list);
    return 0;
}
