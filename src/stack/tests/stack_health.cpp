#include <cerrno>
#include <stdlib.h>
#include <time.h>
#include "../stack.h"
#include "../exceptions.h"
#include "../errors.h"

#define NUM_OF_TESTS 10000

void buffer_null_ptr(){
    srand(time(NULL) ^ getpt());
    

    Stack stk;
    Stack* stack = &stk;
    stack_init(stack,100, 1);

    void* real_addr = stack->buffer;
    stack->buffer = NULL;
    TRY{stack_health_check(stack);
    }
    CATCH(ERR_STACK_BUFF_IS_NULL){
        printf("\nNull ptr on stack buffer successfully handled!\n");
        free(real_addr);
        return;
    }
    END_TRY;
    printf("\nNull ptr on stack buffer doesn't handled!\n");
    free(real_addr);
}
void left_canary_corruption(uint num_of_tests){
    srand(time(NULL) ^ getpt());

    Stack stk;
    Stack* stack = &stk;
    uint left_can_count = 0;

    for(uint j = 0; j < num_of_tests; j ++){

        stack_init(stack,100, 1);
        for(int i = 0; i < 7; i++){
            *((char*)stack->buffer + i) =
                *((char*)stack->buffer + i) ^ (char)(rand() % 256);
        }

        TRY{stack_health_check(stack);
        }
        CATCH(ERR_STACK_LEFT_CANARY_CORRUPTION){
            left_can_count ++;
            free(stack->buffer);
            continue;
        }
        END_TRY;
        printf("\nLeft canary corruption doesn't handled on %u test\n", j);
        free(stack->buffer);
        return;
    }

    printf("Successfully handled %u of %u left canary corruptions!\n", left_can_count, num_of_tests);
}

void right_canary_corruption(uint num_of_tests){
    srand(time(NULL) ^ getpt());

    Stack stk;
    Stack* stack = &stk;
    uint right_can_count = 0;

    for(uint j = 0; j < num_of_tests; j ++){

        stack_init(stack,100, 1);

        for(int i = 1; i < 9; i++){
            *((char*)stack->buffer + stack->capacity - i) =
                *((char*)stack->buffer + stack->capacity - i) ^ (char)(rand() % 256);
        }

        TRY{stack_health_check(stack);
        }
        CATCH(ERR_STACK_RIGHT_CANARY_CORRUPTION){
            right_can_count ++;
            free(stack->buffer);
            continue;
        }
        END_TRY;
        printf("Right canary corruption doesn't handled on %u test\n", j);
        free(stack->buffer);
        return;
    }
    printf("Successfully handled %u of %u right canary corruptions!\n", right_can_count, num_of_tests);


}

void rand_buf_change(uint num_of_tests){
    srand(time(NULL) ^ getpt());

    uint rand_buf_count = 0;

    for(uint i = 0; i < num_of_tests; i ++){
        Stack stk;
        Stack* stack = &stk;
        stack_init(stack,100, 1);

        int random_el = rand() % (stack->capacity - 17) + 8;
        int range     = rand() % (stack->capacity - random_el - 8) + 1 ;

        memset((char*)stack->buffer + random_el, 'A', range);

        TRY{
            stack_health_check(stack);
            }

        CATCH(ERR_STACK_HASH){
            rand_buf_count ++;
            free(stack->buffer);
            continue;
        }
        END_TRY;
        stack_dump(stack);
        printf("Random buffer changes doesn't handled on %u test!\n", i);

        free(stack->buffer);

    }
    printf("Successfully handled %u of %u stack buffer changes!\n", rand_buf_count, num_of_tests);
}

int main(){
    buffer_null_ptr();
    left_canary_corruption(NUM_OF_TESTS);
    right_canary_corruption(NUM_OF_TESTS);
    rand_buf_change(NUM_OF_TESTS);
    return 0;
}
