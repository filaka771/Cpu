#pragma once

enum error{
    // stack_init errors
    ERR_INVALID_PARAM,
    ERR_STACK_ALLOC,
    ERR_STACK_BUFF_ALLOC,

    //stack_health_check errors
    ERR_STACK_BUFF_IS_NULL,
    ERR_STACK_HASH,
    ERR_STACK_LEFT_CANARY_CORRUPTION,
    ERR_STACK_RIGHT_CANARY_CORRUPTION,

    //stack_pop errors
    ERR_POP_EMPTY_STACK,

    //stack_get_element errors
    ERR_ELEM_INDEX_OUT_OF_RANGE
};
