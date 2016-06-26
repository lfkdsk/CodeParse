#ifndef PRIVATE_HOBBY_H_INCLUDE
#define PRIVATE_HOBBY_H_INCLUDE

#include <stdio.h>
#include "MEM.h"
#include "CRB.h"
#include "CRB_DEV.h"

#define smaller(a, b) ((a) < (b) ? (a) : (b))
#define larger(a, b) ((a) > (b) ? (b) : (a))

#define MESSAGE_ARGUMENT_MAX (256)
#define LINE_BUF_SIZE (1024)

typedef enum {
        PARSE_ERROR = 1,
        CHARACTER_INVALID_ERROR,
        FUNCTION_MULTIPLE_DEFINE_ERROR,
        COMPILE_ERROR_COUNT_PLUS_1
} CompileError;

typedef enum {
        VARIABLE_NOT_FOUND_ERROR = 1,
        FUNCTION_NOT_FOUND_ERROR,
        ARGUMENT_TOO_MANY_ERROR,
        ARGUMENT_TOO_FEW_ERROR,
        NOT_BOOLEAN_TYPE_ERROR,
        MUNUS_OPERAND_TYPE_ERROR,
        BAD_OPERAND_TYPE_ERROR,
        NOT_BOOLEAN_OPERATER_ERROR,
        FOPEN_ARGUMENT_TYPE_ERROR,
        FCLOSE_ARGUMENT_TYPE_ERROR,
        FGETS_ARGUMENT_TYPE_ERROR,
        FPUTS_ARGUMENT_TYPE_ERRPR,
        NOT_NULL_OPERATOR_ERROR,
        DIVISION_BY_ZERO,
        GLOBAL_VARIABLE_NOT_FOUND_ERROR,
        GLOBAL_STATEMENT_IN_TOPLEVEL_ERROR,
        BAD_OPERATOR_FOR_STRING_ERROR,
        RUNTIME_ERROR_COUNT_PLUE_1
} RuntimeError;



#endif
