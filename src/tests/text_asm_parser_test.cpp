#include <stdlib.h>

#include "../instructions.h"
#include "../text_asm_parser/text_asm_parser.h"

int main(){
    const char* file_name = "../examples/fibonacci.myasm";
    
    TextInstructionArray text_instr_arr;
    TextInstructionArray* text_instructions_array = & text_instr_arr;


    LabelTable lab_tab;
    LabelTable* label_table = &lab_tab;

    parse_text_asm_file(file_name, text_instructions_array, label_table);

    free(text_instructions_array->text_instruction_list);
    free(label_table->label_list);
    return 0;
}
