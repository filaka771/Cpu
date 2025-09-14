#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdlib.h>
#include <stdint.h>
#include "instructions.h"

// Parse given line of asm code into TextInstruction representation
void parse_line(char* line_buf, TextInstruction* text_instruction);

// Read asm file and returns list of instruction in TextInstruction
// representation
void read_asm_file(const char* file_name, TextInstructionArray* text_instruction_array);


void instruction_name_assemble(TextInstruction* text_instruction, Instruction* instruction);

void instruction_arg_assemble(TextInstruction* text_instruction, Instruction* instruction, uint8_t arg_position);

void write_asm_file(char* file_name, InstructionArray* instruction_array);

#endif // ASSEMBLER_H
