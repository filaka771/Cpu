#include <stdlib.h>
#include <stdlib.h>
#include "../stack.h"
#include "../exceptions.h"
#include "../errors.h"
#include <openssl/rand.h>
#include <openssl/sha.h>

#define NUM_OF_OPERATIONS 2
#define EL_NUM 10

void stack_copy_test(){
    printf("\nSTACK COPY TEST:\n");

    // Random stack generation
    Stack stk;
    Stack new_stk;

    Stack* stack = &stk;
    Stack* new_stack = & new_stk;

    stack_init(stack,1,1);
    stack_init(new_stack, 1,1);

    for (size_t i = 0; i < EL_NUM; i ++){
        stack_push(stack, (void*)(const char[]){(char)(rand() % 256)});
    }

    // Stack copy
    stack_copy(stack, new_stack);

    // Stack identity verification
    for (size_t i = 0; i < 10; i++){
        if (*(char*)stack_get_element(stack,i) != *(char*)stack_get_element(new_stack, i)){
            printf("Wrong element copy on %zu\n", i);
            stack_free(stack);
            stack_free(new_stack);
            return;
        }
    }
    printf("In direct comparison both stacks contains same elements!\n");

    // Hash comparison
    unsigned char hash [SHA256_DIGEST_LENGTH]; 
    unsigned char new_hash [SHA256_DIGEST_LENGTH];

    SHA256((unsigned char*)stack->buffer + 8, stack->count, hash);
    SHA256((unsigned char*)new_stack->buffer + 8, new_stack->count, new_hash);

    for (int i = 0; i < 32; i ++){
        if(hash[i] == new_hash[i])
            continue;
        else{
            printf("Different hash, test failed!\n");
        }
    }
    printf("Both stacks have same hashes!\n");

    
    stack_free(stack);
    stack_free(new_stack);
    printf("Test successfully passed!\n");
}

void push_pop_test(){
    printf("\nSTACK PUSH AND POP TEST:\n");
    int count = 0;    
    Stack stk;
    Stack* stack = &stk;
    stack_init(stack, 100, 1);
    for (int i = 0; i < 10000; i++){
        int operation = rand() % NUM_OF_OPERATIONS;
        if (operation == 1){
            TRY{
                stack_push(stack, (void*)(const char[]){"A"});
                count ++;
                }
            CATCH(ERR_INVALID_PARAM){
                printf("Invalid parameters while stack initialization");
            }
            CATCH(ERR_STACK_ALLOC){
                printf("Trouble with stack structure allocation");
            }
            CATCH(ERR_STACK_BUFF_ALLOC){
                printf("Trouble with stack buff allocation");
            }
            CATCH_ALL{
                perror("Stack damaged while pushing new element.");
                abort();
                }
            END_TRY;
}

        if (operation == 2){
            TRY{
                stack_pop(stack);
                count --;
            }
            CATCH(ERR_POP_EMPTY_STACK){
                continue;
                }
            CATCH_ALL{
                perror("Stack damaged while poping element.");
                printf("Iteration: %d", i);
                abort();
                }
            END_TRY;
        }
    }
    free(stack->buffer);

    printf("Actual number of elements: %zu\n", stack->count);
    printf("Counted number of elements: %d\n", count);
    printf("Test successfully passed!\n");
}

void stack_modify_element_test(){
    printf("\nSTACK MODIFY ELEMENT TEST:\n");
    Stack stk;
    Stack* stack = &stk;
    stack_init(stack, 100, 1);

    // Random positions array
    int rand_position_array [5000];

    for (int i = 0; i < 5000; i ++){
        int rand_position = rand() % 10000;
        rand_position_array[i] = rand_position;

        TRY{
            stack_modify_element(stack, (void*)(const char[]){"A"}, rand_position);
        }
        CATCH_ALL{
            fprintf(stderr, "Error While element modification!\n");
            abort();
        }
        END_TRY;
    }

    // Check that on saved positions proper elements
    for(int i = 0; i < 5000; i ++){
        if(*(char*)stack_get_element(stack, rand_position_array[i]) != 'A'){
            fprintf(stderr, "Wrong element readed after modification!");
            abort();
        }
    }

    printf("All changed elements are equals to expected values!\nTest successfully passed!\n");
    stack_free(stack);
}

int main(){
    push_pop_test();
    TRY{
        stack_copy_test();
    }
    CATCH_ALL{
        fprintf(stderr, "Error while stack copy test!");
    }
    END_TRY;

    TRY{
        stack_modify_element_test();
    }
    CATCH_ALL{
        fprintf(stderr, "Error while stack element modification test!");
    }
    END_TRY;

}
