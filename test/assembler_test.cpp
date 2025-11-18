#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../assembler.h"

#define TEXT_INSTR_ARR_SIZE 1000

void print_label_table(LabelTable* label_table){
    printf("\nLabel table count: %u", label_table->count);
    for(uint32_t i = 0; i < label_table->count; i++){
        printf("\n%u %s %u\n", i, label_table->label_list[i].label, label_table->label_list[i].address);
    }
}

void print_text_instruction(TextInstruction* text_instruction){
    printf("\n%u %s ", text_instruction->address, text_instruction->operation.operation_name);
    for(uint32_t i = 0; i < text_instruction->operation.num_of_args; i++){
        printf("%s %u ", text_instruction->imm[i].imm_flag, text_instruction->imm[i].imm);
    }
    printf("\n");
}

int main(){
    
    //char* line_buf1 = (char*)"      :label";
    //char* line_buf2 = (char*)"               :labell           ";

    TextInstruction text_instr;
    TextInstruction* text_instruction = &text_instr;

    LabelTable lab_tab;
    LabelTable* label_table = &lab_tab;

    int symb_count = 0;

    label_table->capacity = 10;
    label_table->count = 0;
    label_table->label_list = (Label*)calloc(label_table->capacity, sizeof(Label));
    //-----------------------------
    // Emulate label table
    const char* label = "start";
    strcpy(label_table->label_list[0].label, label);
    label_table->label_list[0].label_size = strlen(label);
    label_table->label_list[0].address = 12012;
    label_table->count ++;


    char* instruction_buf = (char*)"mov *x10  :startstartstartstartstartstartstartstartstartstartstartstartstartstartstartstartstartstartstartstartstart\n";
    parse_instruction(instruction_buf, label_table, text_instruction, symb_count);

    print_text_instruction(text_instruction);
    print_label_table(label_table);

    return 0;
}
