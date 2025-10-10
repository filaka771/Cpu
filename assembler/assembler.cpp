#include "./assembler.h"
#include "../instructions.h"
#include "../text_asm_parser/text_asm_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
void assemble_flag(uint32_t* bin_flag, char* text_flag, int* arg_count){
    int flag_count = 0;

    while(text_flag[flag_count] != '\0'){
        //Find flag in instruction flag array    
        for(int i = 0; i < INSTRUCTIONS_FLAGS_NUMBER; i ++){
            if(text_flag[flag_count] == instruction_flag[i]){
                *bin_flag |= 1 << (i + flag_count * 8);
            }
            if(text_flag[flag_count] == ' '){
                continue;
            }
        }
        flag_count ++;
    }
}


void assemble_text_instruction(BinInstruction* bin_instruction, TextInstruction* text_instruction){
    // Verify that layout of bin instructions support given instructions and flags sets.
    if(INSTRUCTIONS_FLAGS_NUMBER > MAX_FLAG_NUMBER){
        fprintf(stderr, "Instructions layout doesn't support %d different flags. Maximum number of flags is %d", INSTRUCTIONS_FLAGS_NUMBER, MAX_FLAG_NUMBER);
        abort();
    }

    if(INSTRUCTIONS_SET_NUMBER > MAX_INSTRUCTIONS_NUMBER){
        fprintf(stderr, "Instructions layout doesn't support %d different instructions. Maximum number of instructions is %d", INSTRUCTIONS_SET_NUMBER, MAX_INSTRUCTIONS_NUMBER);
        abort();
    }

    int arg_count = 0;

    // Set all bits in operation buffer to 0
    bin_instruction->operation &= 1 << 31;
    bin_instruction->operation &= 1;

    // Assemble operation code
    bin_instruction->operation = bin_instruction->operation && text_instruction->operation.op_code << 24;

    // Asseble args and flags
    while(arg_count < text_instruction->operation.num_of_args){
        bin_instruction->arg_list[arg_count] = text_instruction->imm[arg_count].imm;
        assemble_flag(&bin_instruction->operation, text_instruction->imm[arg_count].imm_flag, &arg_count);

        arg_count ++;
    }
}

void assemble_text_instructions_array(BinInstructionArray* bin_instructions_array, TextInstructionArray* text_instructions_array){
    for(uint32_t instructions_count = 0; instructions_count < text_instructions_array->count; instructions_count ++){
        assemble_text_instruction(&bin_instructions_array->bin_instruction_list[instructions_count], &text_instructions_array->text_instruction_list[instructions_count]);
    }
}

void write_bin_file(BinInstructionArray* bin_instructions_array, const char* output_bin_file){
    int fd = open(output_bin_file, O_RDWR | O_CREAT | O_TRUNC,  0666);

    if(fd == -1){
        fprintf(stderr, "Error while opening output file!");
        abort();
    }

    size_t bin_file_size = bin_instructions_array->count * sizeof(BinInstruction);

    ftruncate(fd, bin_file_size);

    BinInstruction* mapped_mem = (BinInstruction*)mmap(NULL, bin_file_size, PROT_WRITE, MAP_SHARED, fd, 0);

    if(mapped_mem == MAP_FAILED){
        close(fd);
        fprintf(stderr, "ERROR: Cannot map memory for bin file!");
        abort();
    }

    memcpy(mapped_mem, bin_instructions_array->bin_instruction_list, bin_file_size);

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

    free(bin_instructions_array);
    return 0;
}
