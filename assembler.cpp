#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "assembler.h"
#include "instructions.h"

// LABEL_SIZE must be bigger then 8, to be possible save ptr on allocated buffer
#define LINE_SIZE 300
#define LABEL_SIZE 100

typedef struct Label{
    uint label_size;
    char label[LABEL_SIZE];
    uint32_t address;
}Label;

typedef struct LabelTable{
    uint32_t count;
    uint32_t capacity;
    Label* label_list;
}LabelTable;



//----------------------------------------------------------
void parse_label(char* line_buf, LabelTable* label_table, uint32_t address, int symb_count){
    // Reallocate LabelTable if needed
    if(label_table->count == label_table->capacity - 1){
        label_table = (LabelTable*)realloc(label_table, (label_table->capacity * 1.5) * sizeof(Label));
    }


    Label* current_label = &label_table->label_list[label_table->count];
    current_label->label_size = 0;

    // Save label address
    current_label->address = address;

    // Save label name
    while(isalpha(line_buf[symb_count])){
        symb_count ++;
        current_label->label_size ++;
    }

    if(current_label->label_size <= LABEL_SIZE){
        memcpy(current_label->label, &line_buf[symb_count - current_label->label_size], current_label->label_size);
    }
    else{
        void* label_buf = calloc(current_label->label_size, sizeof(char));
        memcpy(label_buf, &line_buf[symb_count - current_label->label_size], current_label->label_size);
        memcpy(current_label->label, &label_buf, sizeof(void*));
    }

    // Check
    while(line_buf[symb_count] != '\n' && line_buf[symb_count] != '\0'){
        if(line_buf[symb_count] == ' ')
            continue;

        if(isalpha(line_buf[symb_count])){
            perror("Label name must contain only letters, spaces not allowed!");
            abort();
        }

        if(isdigit(line_buf[symb_count])){
            perror("Label name must contain only letters, digits not allowed!");
            abort();
        }

        else{
            perror("Wrong label name!");
            abort();
        }
    }
}

Label* find_label(LabelTable* label_table, char* label, int label_length){
    for(int i = 0; i < label_table->count; i ++){
        if(label_table->label_list[i].label_size == label_length && label_length <= LABEL_SIZE){
            if(memcmp(label, label_table->label_list[i].label, label_length)){
                return &label_table->label_list[i];
            }
        }
        if(label_table->label_list[i].label_size == label_length){
            if(memcmp(label, *label_table->label_list[i].label, label_length)){
                return &label_table->label_list[i];
            }
        }
    }
            return NULL;
}

//----------------------------------------------------------

void set_op_name(TextInstruction* text_instruction, char* op_name, int arg_length){
    for(int i = 0; i < INSTRUCTIONS_SET_NUMBER; i++){
        if(memcmp(op_name,(const void*)instruction_set[i].op_name,arg_length) == 0){
            strcpy(text_instruction->operation.operation_name, instruction_set[i].op_name);
            text_instruction->operation.num_of_args = instruction_set[i].num_of_args;
            return;
        }
    }
    perror("Wrong instruction name! No instruction with this name!" );
    abort();
}

void set_arg_flag(TextInstruction* text_instruction, char* arg_flag, int arg_count, int arg_length){
    for(int i = 0; i < INSTRUCTIONS_FLAGS_NUMBER; i ++){
        if(memcmp(arg_flag, instruction_flag[i], arg_length) == 0){
            memcpy(text_instruction->imm[arg_count].imm_flag, instruction_flag[i], 2);
        }
    }
    perror("Wrong arg flag!");
    abort();
}

void set_num_arg(TextInstruction* text_instruction, char* num_arg, int arg_count, int arg_length){
    bool flag_set = false;
    // Parse case without flag
    for(int i = 0; i < INSTRUCTIONS_FLAGS_NUMBER; i++){
        if(memcmp(text_instruction->imm[arg_count].imm_flag, instruction_flag[i], 3)){
            flag_set = true;
            break;
        }
    }

    if(!flag_set){
        memcpy(text_instruction->imm[arg_count].imm_flag, instruction_flag[3], 3);
    }

    // Parse num arg
    long long arg = strtoll(num_arg, &(num_arg + arg_length), 10);
    if(arg > UINT_MAX){
        perror("To large arg. Arg must me less then UINT_MAX!");
        abort();
    }

    text_instruction->imm[arg_count].imm = arg;
}

void set_label_arg(TextInstruction* text_instruction,LabelTable* label_table, char* label_arg, int arg_length, int arg_count){
    Label* label = find_label(label_table, label_arg, arg_length);

    // Case when label in label table
    if(label != NULL){
        if(label->address != UINT_MAX){
            text_instruction->imm[arg_count].imm = label->address;
        }
    }

        // Case when label not in label table
    else{
        // Realloc label list if needed
        if(label_table->count == label_table->capacity){
            label_table->label_list = realloc(label_table->label_list, label_table->capacity * 1.5);
            label_table->capacity *= 1.5;
        }

        label_table->count ++;
        label_table->label_list[label_table->count].label_size = arg_length;

        if(label_table->label_list[label_table->count].label_size <= LABEL_SIZE){
            memcpy(label_table->label_list[label_table->count].label, label_arg, label_table->label_list[label_table->count].label_size);
        }

        else{
            void* label_buf = calloc(label_table->label_list[label_table->count].label_size, sizeof(char));
            memcpy(label_table->label_list[label_table->count].label, &label_buf, sizeof(void*));
        }
    }

}

void parse_instruction(char* line_buf, LabelTable* label_table, TextInstruction* text_instruction, int symb_count){
    int arg_length = 0;
    int arg_count = 0;
    // Read op_name
    while(isalpha(line_buf[symb_count])){
        arg_length ++;
        symb_count ++;
    }

    if(arg_length == INSTRUCTION_SIZE){
        set_op_name(text_instruction, &line_buf[symb_count], arg_length);
    }
    else{
        perror("Wrong operation name. Operation name must contain only 3 symbols!");
        abort();
    }

    // Read args
    while(symb_count <= strlen(line_buf) && arg_count < text_instruction->operation.num_of_args){
        // Skip spaces
        if(line_buf[symb_count] == ' '){
            symb_count ++;
            continue;
        }

        // Parse flag
        if(isalpha(line_buf[symb_count])){
            arg_length = 0;
            while(isalpha(line_buf[symb_count])){
                symb_count ++;
                arg_length ++;
            }

            if(arg_length == ARG_SIZE ){
                set_arg_flag(text_instruction, &line_buf[symb_count - arg_length], arg_count, arg_length);
                arg_count ++;
            }

            else{
                perror("Wrong arg flag!");
                abort();
            }
        }

        // Parse num args
        if(isdigit(line_buf[symb_count])){
            arg_length = 0;
            while(isdigit(line_buf[symb_count])){
                symb_count ++;
                arg_length ++;
            }
            set_num_arg();
        }

        // Parse lable 
        if(line_buf[symb_count] == ':'){
            arg_length = 0;

        }
    }

}

// ---------------------------------------------------------

void parse_line(char* line_buf, LabelTable* label_table, TextInstruction* text_instruction, int pass_count){
    uint32_t address = 0;
    for(int symb_count = 0; symb_count < strlen(line_buf); symb_count ++){
        if(line_buf[symb_count] == ':'){
            parse_label(line_buf, label_table, address, symb_count);
        }

        if(isalpha(line_buf[symb_count])){
            parse_instruction(line_buf, label_table, text_instruction, symb_count);
        }
    }
}

//--------------------------------------------------
void read_asm_file(const char* file_name, TextInstructionArray* text_instruction_array){
    FILE* asm_file = fopen(file_name, "r");

    //TODO: Add exception mechanism.
    if(asm_file == NULL){
        perror("Error opening file!");
        return;
    }
    // ---------------------------------------------
    size_t count = 0;
    char* line_buff = NULL;

    while(getline(&line_buff, &count, asm_file) != -1){
        parse_line(line_buff, &text_instruction_array->text_instruction_list[text_instruction_array->count]);
        text_instruction_array->count ++;
        continue;
    }

    free(line_buff);
    fclose(asm_file);

}

// --------------------------Assembler--------------------------
void instruction_name_assemble(TextInstruction* text_instruction, Instruction* instruction){
    // Op_code finding
    for(int i = 0; i < INSTRUCTIONS_SET_NUMBER; i++)
        if(strcmp(text_instruction->operation.operation_name, instruction_set[i].op_name)){
            instruction->op_name = i;
            return;
        }

    //TODO: Add exception mechanism
    perror("Wrong op_name");
}

void instruction_arg_assemble(TextInstruction* text_instruction, Instruction* instruction, uint8_t arg_position){
    
    if(strcmp(text_instruction->imm[arg_position].imm_flag, instruction_flag[0])){
        instruction->op_name |= (1 << (31 - 4 + arg_position ));
        instruction->op_name |= (1 << (31 - 5 + arg_position ));
        instruction->imm[arg_position] = text_instruction->imm[arg_position].imm;
        return;
    }

    if(strcmp(text_instruction->imm[arg_position].imm_flag, instruction_flag[1])){
        instruction->op_name |= (1 << (31 - 4 + arg_position ));
        instruction->imm[arg_position] = text_instruction->imm[arg_position].imm;
        return;
    }

    if(strcmp(text_instruction->imm[arg_position].imm_flag, instruction_flag[1])){
        instruction->op_name |= (1 << (31 - 5 + arg_position ));
        instruction->imm[arg_position] = text_instruction->imm[arg_position].imm;
        return;
    }
    instruction->imm[arg_position] = text_instruction->imm[arg_position].imm;
    return;
}

void write_asm_file(){

}



