#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "stack/stack.h"
#include "exceptions/exceptions.h"
#include "errors/errors.h"



#define HASH_SIZE SHA256_DIGEST_LENGTH
#define ELEM_TYPE char

//-------------------------------------DEBUG-------------------------------------
void stack_dump(Stack* stack) {
    printf("\nSTACK DUMP: \n");

    printf("Stack buffer: \n");
    for(size_t i = 0; i < stack->capacity; i++) {
        printf("%02x", ((unsigned char*)stack->buffer)[i]);
    }
    printf("\nHash: ");
    for(size_t i = 0; i < HASH_SIZE; i++) {
        printf("%02x", stack->hash[i]);
    }
    printf("\nBuffer size: %zu\nElement size: %zu\nNumber of elements: %zu\n", 
           stack->capacity, stack->elem_size, stack->count);
    printf("\nLeft canary: %p\n", stack->buffer);
    //printf("Left part of buffer: %p\n", *(void**)stack->buffer);
    printf("Right canary: %p\n", (char*)stack->buffer + stack->capacity);
}

//-------------------------------------Stack_health_check-------------------------------------

static int stack_hashes_compare(const unsigned char* hash_1, const unsigned char* hash_2){
    return memcmp(hash_1, hash_2, HASH_SIZE);
}

void stack_health_check(Stack* stack){
    if(stack->buffer == NULL)
        THROW(ERR_STACK_BUFF_IS_NULL, "Stack structure damaged, NULL is invalid value for pointer on stack buffer.");

    unsigned char current_hash[HASH_SIZE];
    SHA256((const unsigned char*)stack->buffer + 8, stack->capacity - 16, current_hash);
    if(stack_hashes_compare(current_hash, stack->hash) != 0)
        THROW(ERR_STACK_HASH, "Wrong hash. Stack damaged or unauthorized modification happens.");

    if(stack->buffer != *(void**)stack->buffer)
        THROW(ERR_STACK_LEFT_CANARY_CORRUPTION, "Left canary damaged.");

    if((char*)stack->buffer + stack->capacity != *(void**)((char*)stack->buffer + stack->capacity - 8))
        THROW(ERR_STACK_RIGHT_CANARY_CORRUPTION, "Right canary damaged.");

    return;
}

//-------------------------------------Aditional_functions-------------------------------------
static void stack_poison(Stack* stack) {
    // Calculate unused space after elements (excluding header and footer)
    size_t data_end = 8 + stack->elem_size * (stack->count);
    size_t poison_size = stack->capacity - data_end - 8;
    memset((char*)stack->buffer + data_end, 0x00, poison_size);
}

static void stack_hash(Stack* stack) {
    SHA256((const unsigned char*)stack->buffer + 8, stack->capacity - 16, stack->hash);
}

static void stack_canary_set(Stack* stack){
    *(void**)stack->buffer = stack->buffer;

    void* end_of_buffer = (char*)stack->buffer + stack->capacity;
    *(void**)((char*)stack->buffer + stack->capacity - 8) = end_of_buffer;
}

static void stack_realloc(Stack* stack, size_t new_size) {
    size_t new_capacity = new_size;
    stack->buffer = realloc(stack->buffer, new_capacity);
    stack->capacity = new_capacity;

    stack_canary_set(stack);

}

//-------------------------------------Stack_init-------------------------------------

void stack_init(Stack* stack, size_t el_num, size_t el_size) {
    // Check initial parameters
    if (el_num == 0 || el_size == 0){
        THROW(1, "Invalid stack parameters");
    }

    if (!stack){
        THROW(2, "Failed to allocate Stack buffer");
    }

    stack->elem_size = el_size;
    stack->capacity = el_num * el_size + 16;
    stack->count = 0;

    stack->buffer = calloc(1, stack->capacity);
    // Stack buffer allocation failure
    if (!stack->buffer){
        free(stack);
        THROW(3, "Failed to allocate Stack buffer");
    }

    stack_canary_set(stack);

    stack_poison(stack);
    stack_hash(stack);
}


void stack_init_from_file(Stack* stack, size_t el_size, const char* file_name) {
    // Open with low-level API
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Error while opening file!");
        abort();
    }

    // Get file size using fstat
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        fprintf(stderr, "Failed to get file size!");
        close(fd);
        abort();
    }
    
    size_t file_size = sb.st_size;

    if(file_size % el_size != 0){
        fprintf(stderr, "File could not be interpreted as list of elements with size %zu!", el_size);
        close(fd);
        abort();
    }

    // Map the file
    void* mapped = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd); 

    if (mapped == MAP_FAILED){
        fprintf(stderr, "ERROR WHILE FILE MAPPING!");
        abort();
    }

    // Stack initialization
    stack->elem_size = el_size;
    stack->capacity  = (size_t)((file_size + 16) * 1.5);
    stack->count     = file_size / el_size;

    stack->buffer = calloc(1, stack->capacity);

    if (!stack->buffer){
        munmap(mapped, file_size);
        free(stack);
        THROW(3, "Failed to allocate Stack buffer");
    }

    memcpy(((uint8_t*)stack->buffer + 8), mapped, file_size);
    
    // Clean up
    munmap(mapped, file_size);


    stack_canary_set(stack);

    stack_poison(stack);
    stack_hash(stack);
}


//-------------------------------------Stack_operations-------------------------------------
void stack_free(Stack* stack) {
    stack_health_check(stack);
    free(stack->buffer);
}


void stack_copy(Stack* new_stack, Stack* old_stack){
    stack_health_check(old_stack);
    if (new_stack->capacity < old_stack->capacity)
        stack_realloc(new_stack, old_stack->capacity);

    //TODO: Make stack_init doesn't hash stack before buffer will be copied.
    memcpy((char*)new_stack->buffer + 8,(char*)old_stack->buffer + 8, old_stack->capacity - 16);

    new_stack->count = old_stack->count;
    stack_canary_set(new_stack);
    stack_hash(new_stack);

}

void stack_pop(Stack* stack){
    stack_health_check(stack);

    if(stack->count == 0)
        THROW(ERR_POP_EMPTY_STACK, "Nothing to pop on empty stack.");

    stack->count --;
    stack_poison(stack);
    stack_hash(stack);
}

void stack_push(Stack* stack, void* elem){
    stack_health_check(stack);
    if(stack->capacity - stack->count * stack->elem_size - 16 <= stack->elem_size)
        stack_realloc(stack, stack->capacity * 1.5);

    char* dest_addr = (char*)stack->buffer + stack->count * stack->elem_size + 8;
    memcpy((void*)dest_addr, elem, stack->elem_size);
    stack->count ++;

    stack_hash(stack);
}

void* stack_get_element(Stack* stack, size_t position){
    if(position >= stack->count)
        THROW(ERR_ELEM_INDEX_OUT_OF_RANGE, "Element index out of stack range.");

    void* elem_ptr = (char*)stack->buffer + position * stack->elem_size + 8;
    return elem_ptr;
}

void stack_modify_element(Stack* stack, void* elem, size_t position){
    stack_health_check(stack);

    // Reallocate stack buffer, if needed
    if(position >= (stack->capacity - 16) * stack->elem_size){
        stack->capacity = (position * stack->elem_size) * 1.5;
        stack_realloc(stack, stack->capacity);
    }

    // Modify element
    char* dest_addr = (char*)stack->buffer + position * stack->elem_size + 8;
    memcpy((void*)dest_addr, elem, stack->elem_size);

    // Change end of stack if needed
    if(stack->count <= position){
        stack->count = position + 1;
    }

    // Hash stack
    stack_poison(stack);
    stack_hash(stack);

}
