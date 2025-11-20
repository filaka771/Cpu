#ifndef TEXT_ASM_PARSER_H
#define TEXT_ASM_PARSER_H

#include <stdlib.h>
#include <stdint.h>
#include "instructions/instructions.h"

#define LINE_SIZE 300
#define LABEL_SIZE 100
#define LABEL_TABLE_INIT_SIZE 1000

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

typedef struct ParserState{
    bool flag_seted;
    bool  arg_seted;
}ParserState;


void free_label_table(LabelTable* label_table);

void free_text_instruction_list(TextInstructionArray* text_instruction_array);

void parse_text_asm_file(const char* file_name, TextInstructionArray* text_instruction_array, LabelTable* label_table);
#endif // TEXT_ASM_PARSER_H
