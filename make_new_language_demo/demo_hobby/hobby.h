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

typedef enum {
        INT_MESSAGE_ARGUMENT = 1,
        DOUBLE_MESSAGE_ARGUMENT,
        STRING_MESSAGE_ARGUMENT,
        CHARACTER_MESSAGE_ARGUMENT,
        POINT_MESSAGE_ARGUMENT,
        MESSAGE_ARGUMENT_END
} MessageArgumentType;

typedef struct {
        char *format;
} MessageFormat;

typedef struct Expression_tag Expression;

typedef enum {
        BOOLEAN_EXPRESSION = 1,
        INT_EXPRESSION,
        DOUBLE_EXPRESSION,
        STRING_EXPRESSION,
        IDENTIFIER_EXPRESSION,
        ASSIGN_EXPRESSION,
        ADD_EXPRESSION,
        SUB_EXPRESSION,
        MUL_EXPRESSION,
        DIV_EXPRESSION,
        MOD_EXPRESSION,
        EQ_EXPRESSION,
        NE_EXPRESSION,
        GT_EXPRESSION,
        GE_EXPRESSION,
        LT_EXPRESSION,
        LE_EXPRESSION,
        LOGICAL_AND_EXPRESSION,
        LOGICAL_OR_EXPRESSION,
        MINUS_EXPRESSION,
        FUNCTION_CALL_EXPRESSION,
        NULL_EXPRESSION,
        EXPRESSION_TYPE_COUNT_PLUS_1
} ExpressionType;

#define hbb_is_math_operator(operator) \
        ((operator) == ADD_EXPRESSION || (operator) == SUB_EXPRESSION \
         || (operator) == MUL_EXPRESSION || (operator) == DIV_EXPRESSION \
         || (operator) == MOD_EXPRESSION)

#define hbb_is_compare_operator(operator) \
        ((operator) == EQ_EXPRESSION || (operator) == NE_EXPRESSION \
         || (operator) == GT_EXPRESSION || (operator) == GE_EXPRESSION \
         || (operator) == LT_EXPRESSION || (operator) == LE_EXPRESSION)

#define hbb_is_logical_operator(operator) \
        ((operator) == LOGICAL_AND_EXPRESSION || (operator) == LOGICAL_OR_EXPRESSION)

typedef struct ArgumentList_tag {
        Expression *expression;
        struct ArgumentList_tag *next;
} AssignExpression;

typedef struct {
        char *variable;
        Expression *operand;
} AssignExpression;

typedef struct {
        Expression *left;
        Expression *right;
} BinaryExpression;

typedef struct {
        char *identifier;
        ArgumentList *argument;
} FunctionCallExpression;

struct Expression_tag {
        ExpressionType type;
        int line_number;
        union {
                HBB_Boolean boolean_value;
                int int_value;
                double double_value;
                char *string_value;
                char *identifier;
                AssignExpression assign_expression;
                BinaryExpression binary_expression;
                Expression *minus_expression;
                FunctionCallExpression function_call_expression;
        } u;
};

typedef struct Statement_tag Statement;

typedef struct StatementList_tag {
        Statement *statement;
        struct StatementList_tag *next;
} StatementList;

typedef struct {
        StatementList *statement_list;
} Block;

typedef struct IdentifierList_tag {
        char *name;
        struct IdentifierList_tag *next;
} IdentifierList;

typedef struct {
        IdentifierList *identifier_list;
} GlobalStatement;

typedef struct ElseIf_tag {
        Expression *condition;
        Block *block;
        struct ElseIf_tag *next;
} IfStatement;

typedef struct {
        Expression *condition;
        Block *then_block;
        ElseIf *elseif_list;
        Block *else_block;
} IfStatement;

typedef struct {
        Expression *condition;
        Block *block;
} WhileStatement;

typedef struct {
        Expression *initial;
        Expression *condition;
        Expression *post;
        Block *block;
} ForStatement;

typedef struct {
        Expression *return_value;
} ReturnStatement;

typedef enum {
        EXPRESSION_STATEMENT = 1,
        GLOBAL_STATEMENT,
        IF_STATEMENT,
        WHILE_STATEMENT,
        FOR_STATEMENT,
        RETURN_STATEMENT,
        BREAK_STATEMENT,
        CONTINUE_STATEMENT,
        STATEMENT_TYPE_COUNT_PLUS_1
} StatementType;

struct Statement_tag {
        StatementType type;
        int line_number;
        union {
                Expression *expression_s;
                GlobalStatement global_s;
                IfStatement if_s;
                WhileStatement while_s;
                ForStatement for_s;
                ReturnStatement return_s;
        } u;
};

typedef struct ParameterList_tag {
        char *name;
        struct ParameterList_tag *next;
} ParameterList;

typedef enum {
        HBB_FUNCTION_DEFINITION = 1,
        NATIVE_FUNCTION_DEFINITION
} FunctionDefinitionType;

typedef struct FunctionDefinition_tag {
        char *name;
        FunctionDefinitionType type;
        union {
                struct {
                        ParameterList *parameter;
                        Block *block;
                }hbb_f;

                struct {
                        HBB_NativeFunctionProc *proc;
                }native_f;
        } u;
        struct FunctionDefinition_tag *next;
} FunctionDefinition;

typedef struct Variable_tag {
        char *name;
        HBB_Value value;
        struct Variable_tag *next;
} Variable;

typedef enum {
        NORMAL_STATEMENT_RESULT = 1,
        RETURN_STATEMENT_RESULT,
        BREAK_STATEMENT_RESULT,
        CONTINUE_STATEMENT_RESULT,
        STATEMENT_RESULT_TYPE_COUNT_PLUS_1
} StatementResultType;

typedef struct {
        StatementResultType type;
        union {
                HBB_Value return_value;
        } u;
} StatementResult;

typedef struct GlobalVariableRef_tag {
        Variable *variable;
        struct GlobalVariableRef_tag *next;
} GlobalVariableRef;

typedef struct {
        Variable *variable;
        GlobalVariableRef *global_vriable;
} LocalEnvironment;

struct HBB_String_tag {
        int ref_count;
        char *string;
        HBB_Boolean is_literal;
};

typedef struct {
        HBB_String *strings;
} StringPool;

struct HBB_Interpreter_tag {
        MEM_Storage interpreter_storage;
        MEM_Storage execute_storge;
        Variable  *variable;
        FunctionDefinition *function_list;
        StatementList *statement_list;
        int current_line_number;
};

/* function form create.c */

void hbb_function_define(char *identifier,
                         ParameterList *parameter_list
                         , Block *block);

ParameterList *hbb_create_parameter(char *identifier);

ParameterList *hbb_chain_parameter(ParameterList *list, char *identifier);

ArgumentList *hbb_create_argument_list(Expression *expression);

ArgumentList *hbb_chain_argument_list(ArgumentList *list, Expression *exp);

StatementList *hbb_create_statement_list(Statement *statement);

StatementList *hbb_chain_statement_list(StatementList *list, Statement *st);

Expression *hbb_alloc_expression(ExpressionType type);

Expression *hbb_create_assign_expression(char *variable,Expression *operand);

Expression *hbb_create_binary_expression(ExpressionType type,
                                         Expression *left,Expression *right);

Expression *hbb_create_minus_expression(Expression *operand);

Expression *hbb_create_identifier_expression(char *identifier); Expression *hbb_create_function_call_expresssion(char *func_name,
                                                                                                                 ArgumentList *argument);
Expression *hbb_create_boolean_expression(HBB_Boolean value);

Expression *hbb_create_null_expression(void);

Statement *hbb_create_global_statement(IdentifierList *identifierList);

IdentifierList *hbb_create_global_identifier(char *identifier);

IdentifierList *hbb_chain_identifier(IdentifierList *list, char *identifier);

Statement *hbb_create_if_statement(Expression *condition, Block *then_block,
                                   ElseIf *elseif_list, Block *else_block);

ElseIf *hbb_chain_elseif_list(ElseIf *list, ElseIf *add);

ElseIf *hbb_create_elseif(Expression *exp, Block *block);

Statement *hbb_create_while_statement(Expression *condition, Block *block);

Statement *hbb_create_for_statement(Expression *initial, Expression *condition,
                                    Expression *post, Block *block);

Block *hbb_create_block(StatementList *statement_list);

Statement *hbb_create_expression_statement(Expression *exp);

Statement *hbb_create_return_statement(Expression *exp);

Statement *hbb_create_break_statement(void);

Statement *hbb_create_continue_statement(void);



#endif
