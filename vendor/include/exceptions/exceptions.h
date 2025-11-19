// exceptions.h
#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>

typedef struct{
    int code;
    const char* message;
} Exception;

typedef struct ExceptionContext{
    int line;
    const char* function;
    jmp_buf env;
    Exception exception;
    struct ExceptionContext* prev;
} ExceptionContext;

static __thread ExceptionContext* exception_stack = NULL;

#define TRY {\
    ExceptionContext _context; \
    _context.prev = exception_stack; \
    exception_stack = &_context; \
    int _exception = setjmp(_context.env); \
    if (_exception == 0)

#define CATCH(code)\
    else if (_exception == (code))

#define CATCH_ALL\
    else

#define THROW(error_code, msg) {\
    if (exception_stack){\
    exception_stack->exception.code = (error_code);\
    exception_stack->exception.message = (msg);\
    exception_stack->line = __LINE__;\
    exception_stack->function = __FUNCTION__;\
    longjmp(exception_stack->env, (error_code));\
    } else{ \
        fprintf(stderr, "In function %s (line: %d): Unhandled exception: %d - %s\n",__FUNCTION__, __LINE__, (error_code), (msg)); \
        abort(); \
        } \
    }

#define END_TRY \
    exception_stack = _context.prev; \
    } 

#endif // EXCEPTIONS_H
