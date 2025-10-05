#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "text_asm_parser.h"
#include "../instructions.h"

// LABEL_SIZE must be bigger then 8, to be possible save ptr on allocated buffer

//----------------------------------------------------------
static void critical_label_error(LabelTable* label_table, const char* msg){
    if(label_table && label_table->label_list){
        free(label_table->label_list);
    }
    fprintf(stderr,"\n%s" ,msg);
    abort();
}

static Label* find_label(LabelTable* label_table, char* label, uint32_t label_length){
    for(uint32_t i = 0; i < label_table->count; i ++){
        
        // If label less then LABEL_LENGTH
        if(label_table->label_list[i].label_size == label_length && label_length <= LABEL_SIZE){
            if(memcmp(label, label_table->label_list[i].label, label_length) == 0){
                return &label_table->label_list[i];
            }
        }

        // If label name have large length
        if(label_table->label_list[i].label_size == label_length){
            if(memcmp(label, label_table->label_list[i].label, label_length)){
                return &label_table->label_list[i];
            }
        }
    }
    return NULL;
}

static void parse_label(char* line_buf, LabelTable* label_table, uint32_t address, int symb_count){
    // Move to first leter in label name
    symb_count ++;
    Label* current_label;
    int current_label_size = 0;
    // Measure label length
    while(isalpha(line_buf[symb_count + current_label_size]) || line_buf[symb_count + current_label_size] == '_'){
        current_label_size ++;
    }

    // Zero length label name case
    if(current_label_size == 0){
        critical_label_error(label_table, "Error: Label name must contain one or more symbol! Also shouldn't be spaces after :!\n");
    }

    // Reallocate LabelTable if needed
    if(label_table->count == label_table->capacity){
        label_table = (LabelTable*)realloc(label_table, (label_table->capacity * 1.5) * sizeof(Label));
    }


    current_label = find_label(label_table, &line_buf[symb_count], current_label_size);

    // Verify if label is already in label table
    if(current_label != NULL ){
        if(current_label->address == UINT32_MAX)
            current_label->address = address;
        else{
            if(current_label->address == address)
                return;
            else{
                critical_label_error(label_table, "Error: label redefinition!\n");
            }
        }
    }

    // Case, when there now label with given name
    else{
        label_table->count ++;
        current_label = &label_table->label_list[label_table->count - 1];
        current_label->label_size = current_label_size;
        current_label->address = address;


        // If label size is smaller then allocated buffer
        if(current_label->label_size <= LABEL_SIZE){
            memcpy(current_label->label, &line_buf[symb_count], current_label->label_size);
            printf("Saved label name: %s address: %u\n", current_label->label, current_label->address);
        }

            // If label size is bigger then allocated buffer
        else{
            void* label_buf = calloc(current_label->label_size, sizeof(char));
            memcpy(label_buf, &line_buf[symb_count], current_label->label_size);
            memcpy(current_label->label, &label_buf, sizeof(void*));
        }
    }
    
    symb_count += current_label_size;

    // Check rest of line
    while (line_buf[symb_count] != '\n' && line_buf[symb_count] != '\0') {
        if (line_buf[symb_count] == ' ') {
            symb_count++;
            continue;
        }
    
        // Non-space symbol after label name
        critical_label_error(label_table, "Error: non-space symbol after label name!\nLabel name can contain only latters and '_' symbol!\n");
    }
}

static void free_label_table(LabelTable* label_table){
    // Deallocation of long strings
    for(uint32_t i = 0; i < label_table->count; i ++){
        if(label_table->label_list[i].label_size > LABEL_SIZE){
            free(label_table->label_list[i].label);
        }
    }

    // Labels buffer deallocation
    free(label_table->label_list);
}


//----------------------------------------------------------
static void set_op_name(TextInstruction* text_instruction, char* op_name, int* symb_count){
    int op_name_length = 0;
    while(isalpha(*(op_name + op_name_length))){
        op_name_length ++;
    }
    if(op_name_length == INSTRUCTION_SIZE){
        memcpy(text_instruction->operation.operation_name, op_name, INSTRUCTION_SIZE);
        text_instruction->operation.operation_name[INSTRUCTION_SIZE] = '\0';

        for(int i = 0; i < INSTRUCTIONS_SET_NUMBER; i++){
            if(memcmp(text_instruction->operation.operation_name, instruction_set[i].op_name, INSTRUCTION_SIZE) == 0){
                text_instruction->operation.num_of_args = instruction_set[i].num_of_args;
                *symb_count += op_name_length;
                return;
            }
        }
    }

    fprintf(stderr, "Wrong instruction name!\n");
    abort();
}

//--------------------------------FOR_DEBUG--------------------
void print_flags(uint32_t arg_count, TextInstruction* text_instruction){
    printf("Parsed flags: %u\n", arg_count);
    for(uint32_t i = 0; i <= arg_count; i ++){
        printf("flag %u: %s ", i, text_instruction->imm[i].imm_flag);
    }
    printf("\n");
}

void print_args(uint32_t arg_count, TextInstruction* text_instruction){
    printf("Parsed args: %u\n", arg_count);
    for(uint32_t i = 0; i <= arg_count; i ++){
        printf("arg %u: %u ", i, text_instruction->imm[i].imm);
    }
    printf("\n");
}

//--------------------------------FOR_DEBUG--------------------
static bool check_flag(char flag){
    for(int j = 0; j < INSTRUCTIONS_FLAGS_NUMBER; j ++){
        if (flag == instruction_flag[j]) {
            return true;
        }
    }
    return false;
}

static bool is_ignorable_symbol(char symbol){
    for(int i = 0; i < IGNORABLE_SYMBOLS_NUMBER; i ++){
        if(symbol == ignorable_symbols[i])
            return true;
    }
    return false;
}

static bool is_parse_stoping_symbol(char symbol){
    for(int i = 0; i < PARSE_STOPING_SYMB_NUMBER; i ++){
        if(symbol == parse_stoping_symbols[i])
            return true;
    }
    return false;
}

static ParserState set_arg_flag (TextInstruction* text_instruction, char* arg_flag, uint32_t* arg_count, int* symb_count, ParserState parser_state){

    // Check that there no two flags in a row
    if(parser_state.flag_seted){
        fprintf(stderr, "Error: Flag cannot contain spaces!");
        abort();
    }

    int arg_length = 0;
    while(!isdigit(*(arg_flag + arg_length)) && *(arg_flag + arg_length) != ':' && *(arg_flag + arg_length) != ' '){
        bool flag_found = check_flag(*(arg_flag + arg_length));
        if(flag_found){
            arg_length ++;
            continue;
        }
        else{
            fprintf(stderr, "Unknown flag %c!\n", *(arg_flag + arg_length));
            abort();
        }
    }

    // Flags more with length more then 2 is unsupported
    assert(arg_length <= 2);
    memcpy(text_instruction->imm[*arg_count].imm_flag, arg_flag, arg_length);
    text_instruction->imm[*arg_count].imm_flag[arg_length] = '\0';


    *symb_count += arg_length;

    parser_state.flag_seted = true;
    parser_state.arg_seted = false;
    return parser_state;
}


static ParserState set_num_arg(TextInstruction* text_instruction, char* num_arg, uint32_t arg_count, int* symb_count, ParserState parser_state){
    // Parse case without flag
    if(!parser_state.flag_seted){
        strcpy(text_instruction->imm[arg_count].imm_flag,"  ");
    }

    // Parse num arg
    char* end_of_arg = NULL;
    long long arg = strtoll(num_arg, &end_of_arg, 10);
    if(arg > UINT_MAX){
        perror("To large arg. Arg must me less then UINT_MAX!");
        abort();
    }

    text_instruction->imm[arg_count].imm = (uint32_t)arg;
    *symb_count += end_of_arg - num_arg;

    parser_state.flag_seted = false;
    parser_state.arg_seted = true;
    return parser_state;
}

static ParserState set_label_arg(TextInstruction* text_instruction,LabelTable* label_table, char* line_buf, int* symb_count, uint32_t arg_count, ParserState parser_state){
    int arg_length = 0;

    //Measure label length 
    while(isalpha(line_buf[*symb_count + arg_length]) || line_buf[*symb_count + arg_length] == '_'){
        arg_length ++;
    }

    Label* label = find_label(label_table, &line_buf[*symb_count], arg_length);

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
            label_table->capacity *= 1.5;
            label_table->label_list = (Label*)realloc((void*)label_table->label_list, label_table->capacity * sizeof(Label));

            if(label_table->label_list == NULL){
                fprintf(stderr, "Error while reallocating text label buffer");
                abort();
            }

            // Store label in label table with short strings mechanizm
            label_table->count ++;
            label_table->label_list[label_table->count].label_size = arg_length;

            if(label_table->label_list[label_table->count].label_size <= LABEL_SIZE){
                memcpy(label_table->label_list[label_table->count].label, &line_buf[*symb_count], label_table->label_list[label_table->count].label_size);
            }

            else{
                void* label_buf = calloc(label_table->label_list[label_table->count].label_size, sizeof(char));
                memcpy(label_table->label_list[label_table->count].label, &label_buf, sizeof(void*));
            }

            label_table->label_list[label_table->count].address = UINT32_MAX;
        }
    }

    // Update parser state variables
    *symb_count += arg_length;


    parser_state.arg_seted = true;
    parser_state.flag_seted = false;

    return parser_state;
}

static bool finish_parsing(char* line_buf, TextInstruction* text_instruction, int* symb_count ,uint32_t arg_count){
    if(is_parse_stoping_symbol(line_buf[*symb_count])){
        // Verify that required num of args readed 
        if(arg_count == text_instruction->operation.num_of_args){
            return false;
        }

        else{
            fprintf(stderr, "Error: Instruction %s requires %u but %u was(were) given!", text_instruction->operation.operation_name, text_instruction->operation.num_of_args, arg_count);
            abort();
        }
            
    }
    return true;
}

static void parse_instruction(char* line_buf, LabelTable* label_table, TextInstruction* text_instruction, int symb_count){
    uint32_t arg_count = 0;
    // Read op_name
    set_op_name(text_instruction, line_buf, &symb_count);

    ParserState parser_state;
    parser_state.flag_seted = false;
    parser_state.arg_seted = true;
    // Read args
    while(symb_count <= (int)strlen(line_buf) && finish_parsing(line_buf, text_instruction, &symb_count, arg_count)){
        // Skip spaces
        if(is_ignorable_symbol(line_buf[symb_count])){
            symb_count ++;
            continue;
        }

        // Parse flag
        // TODO: Make func, which verify that symb is in list of flags
        if((isalpha(line_buf[symb_count]) || line_buf[symb_count] == '*') && arg_count < text_instruction->operation.num_of_args){
            parser_state = set_arg_flag(text_instruction, &line_buf[symb_count], &arg_count,  &symb_count, parser_state);
            continue;
        }

        // Parse num args
        if(isdigit(line_buf[symb_count]) && arg_count < text_instruction->operation.num_of_args){
            parser_state = set_num_arg(text_instruction, &line_buf[symb_count], arg_count, &symb_count, parser_state);
            arg_count ++;
            continue;
        }

        // Parse lable
        if(line_buf[symb_count] == ':'){
            // Moove to firs latter symb, which must follow after. 
            symb_count ++;
            parser_state = set_label_arg(text_instruction, label_table, line_buf, &symb_count, arg_count, parser_state);
            arg_count ++;
            continue;
        }


        // Fall on to many args
        if(arg_count >= text_instruction->operation.num_of_args && !is_ignorable_symbol(line_buf[symb_count]) && !is_parse_stoping_symbol(line_buf[symb_count])){
            printf("%s\n",&line_buf[symb_count]);
            fprintf(stderr, "To many args. For operation %s\n", text_instruction->operation.operation_name);
            abort();
        }
    }
}
// ---------------------------------------------------------

static void parse_line(char* line_buf, LabelTable* label_table, TextInstructionArray* text_instruction_array, uint32_t* address){
    TextInstruction* text_instruction = &text_instruction_array->text_instruction_list[text_instruction_array->count];

    for(uint32_t symb_count = 0; symb_count < (uint32_t)strlen(line_buf); symb_count ++){
        if(line_buf[symb_count] == ':'){
            parse_label(line_buf, label_table, *address, symb_count);
            break;
        }
        if(isalpha(line_buf[symb_count])){
            text_instruction->address = *address;
            *address += 4;

            text_instruction_array->count ++;

            parse_instruction(line_buf, label_table, text_instruction, symb_count);
            break;
        }
        if(is_parse_stoping_symbol(line_buf[symb_count])){
            break;
        }
    }
}

void free_text_instruction_list(TextInstructionArray* text_instruction_array){
    if(text_instruction_array && text_instruction_array->text_instruction_list){
        free(text_instruction_array->text_instruction_list);
        text_instruction_array->text_instruction_list = NULL;
        return;
    }
    fprintf(stderr, "Text_instruction_array or instructions list is already NULL!");
    abort();
}

//--------------------------------------------------
static void parse_iteration(FILE *text_asm_file, LabelTable* label_table, TextInstructionArray* text_instruction_array){
    char *line_buf = NULL;
    size_t len = 0;
    ssize_t read;

    uint32_t address = 0;
    text_instruction_array->count = 0;

    // Go to beginning of file
    fseek(text_asm_file, 0, SEEK_SET);
    
    while ((read = getline(&line_buf, &len, text_asm_file)) != -1 && address < UINT32_MAX) {

        // Reallocate text_instructions buffer if needed
        if(text_instruction_array->count == text_instruction_array->capacity){
            text_instruction_array->capacity *= 1.5;
            text_instruction_array->text_instruction_list = (TextInstruction*)calloc(text_instruction_array->capacity, sizeof(TextInstruction));
        }

        parse_line(line_buf, label_table, text_instruction_array, &address);
    }
    
    free(line_buf);
}

//___________________TESTTTTTTTTTTTTTTTTT_________________________

void print_text_instruction(TextInstruction* text_instruction){
    printf("%x %s", text_instruction->address, text_instruction->operation.operation_name);
    for(uint32_t j = 0; j < text_instruction->operation.num_of_args; j ++){
        printf(" %s %u", text_instruction->imm[j].imm_flag, text_instruction->imm[j].imm);
    }
    printf("\n");
}

void print_parsed_asm(TextInstructionArray* text_instruction_array){
    for(uint32_t i = 0; i < text_instruction_array->count; i++){
        print_text_instruction(&text_instruction_array->text_instruction_list[i]);
    }
}

void print_label_table(LabelTable* label_table){
    printf("Label List:\n");
    for (uint32_t i = 0; i < label_table->count; i ++){
        printf("%u   %s\n", label_table->label_list[i].address, label_table->label_list[i].label);
    }
}

//___________________TESTTTTTTTTTTTTTTTTT_________________________
void parse_text_asm_file(const char* file_name){
    FILE *file = fopen(file_name, "r");
    if (!file) {
        perror("Failed to open file");
    }

    // Initialization struct for text asm representation storage
    TextInstructionArray text_instr_arr;
    TextInstructionArray* text_instruction_array = &text_instr_arr;

    text_instruction_array->count = 0;
    text_instruction_array->capacity = INIT_TEXT_INSTR_ARR_SIZE;
    text_instruction_array->text_instruction_list = (TextInstruction*)calloc(text_instruction_array->capacity, sizeof(TextInstruction));

    LabelTable lab_tab;
    LabelTable* label_table = &lab_tab;

    label_table->capacity = LABEL_TABLE_INIT_SIZE;
    label_table->label_list = (Label*)calloc(label_table->capacity, sizeof(Label));
    

    // First iteration
    printf("First pass:\n");
    parse_iteration(file, label_table, text_instruction_array);

    // Second iteration
    printf("\nSecond pass:\n");
    parse_iteration(file, label_table, text_instruction_array);

    // Testttttttttttttttttttttt
    print_parsed_asm(text_instruction_array);
    printf("Num of parsed lines: %u\n", text_instruction_array->count);

    print_label_table(label_table);

    free_label_table(label_table);
    free_text_instruction_list(text_instruction_array);
    
    fclose(file);
}

int main(){
    parse_text_asm_file("../examples/fibonacci.myasm");
    return 0;
}
