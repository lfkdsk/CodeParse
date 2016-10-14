#ifndef PRIVATE_CROWBAR_H_INCLUDED
#define PRIVATE_CROWBAR_H_INCLUDED
#include <stdio.h>
#include <setjmp.h>
#include <wchar.h>
#include <oniguruma.h>
#include "MEM.h"
#include "CRB.h"
#include "CRB_dev.h"

#define smaller(a, b) ((a) < (b) ? (a) : (b))
#define larger(a, b) ((a) > (b) ? (a) : (b))

#define MESSAGE_ARGUMENT_MAX    (256)
#define LINE_BUF_SIZE           (1024)
#define STACK_ALLOC_SIZE        (256)
#define ARRAY_ALLOC_SIZE        (256)
#define HEAP_THRESHOLD_SIZE     (1024 * 256)
#define LONGJMP_ARG             (1)
#define REGEXP_GROUP_INDEX_MAX_COLUMN  (3)

#define EXCEPTION_MEMBER_MESSAGE        ("message")
#define EXCEPTION_MEMBER_STACK_TRACE    ("stack_trace")
#define EXCEPTION_MEMBER_LINE_NUMBER    ("line_number")
#define EXCEPTION_MEMBER_FUNCTION_NAME  ("function_name")
#define EXCEPTION_MEMBER_PRINT_STACK_TRACE      ("print_stack_trace")
#define EXCEPTION_CREATE_METHOD_NAME    ("create")

#define ARRAY_ITERATOR_METHOD_NAME ("__create_array_iterator")
#define ITERATOR_METHOD_NAME    ("iterator")
#define IS_DONE_METHOD_NAME     ("is_done")
#define NEXT_METHOD_NAME        ("next")
#define CURRENT_ITEM_METHOD_NAME        ("current_item")

#define ARRAY_SIZE(array)  (sizeof(array) / sizeof((array)[0]))

typedef enum {
    PARSE_ERR = 1,
    CHARACTER_INVALID_ERR,
    FUNCTION_MULTIPLE_DEFINE_ERR,
    BAD_MULTIBYTE_CHARACTER_IN_COMPILE_ERR,
    CR_IN_REGEXP_ERR,
    CAN_NOT_CREATE_REGEXP_IN_COMPILE_ERR,
    UNEXPECTED_WIDE_STRING_IN_COMPILE_ERR,
    ARRAY_ELEMENT_CAN_NOT_BE_FINAL_ERR,
    COMPLEX_ASSIGNMENT_OPERATOR_TO_FINAL_ERR,
    COMPILE_ERROR_COUNT_PLUS_1
} CompileError;

typedef enum {
    VARIABLE_NOT_FOUND_ERR = 1,
    ARGUMENT_TOO_MANY_ERR,
    ARGUMENT_TOO_FEW_ERR,
    NOT_BOOLEAN_TYPE_ERR,
    MINUS_OPERAND_TYPE_ERR,
    BAD_OPERAND_TYPE_ERR,
    LOGICAL_OP_DOUBLE_OPERAND_ERR,
    LOGICAL_OP_INTEGER_OPERAND_ERR,
    NOT_BOOLEAN_OPERATOR_ERR,
    NOT_NULL_OPERATOR_ERR,
    NOT_LVALUE_ERR,
    INDEX_OPERAND_NOT_ARRAY_ERR,
    INDEX_OPERAND_NOT_INT_ERR,
    ARRAY_INDEX_OUT_OF_BOUNDS_ERR,
    NO_SUCH_METHOD_ERR,
    INC_DEC_OPERAND_TYPE_ERR,
    INC_DEC_OPERAND_NOT_EXIST_ERR,
    NOT_FUNCTION_ERR,
    NOT_OBJECT_MEMBER_UPDATE_ERR,
    NOT_OBJECT_MEMBER_ASSIGN_ERR,
    NO_SUCH_MEMBER_ERR,
    NO_MEMBER_TYPE_ERR,
    BAD_OPERATOR_FOR_STRING_ERR,
    DIVISION_BY_ZERO_ERR,
    GLOBAL_VARIABLE_NOT_FOUND_ERR,
    GLOBAL_STATEMENT_IN_TOPLEVEL_ERR,
    FUNCTION_EXISTS_ERR,
    ARRAY_RESIZE_ARGUMENT_ERR,
    ARRAY_INSERT_ARGUMENT_ERR,
    ARRAY_REMOVE_ARGUMENT_ERR,
    STRING_POS_OUT_OF_BOUNDS_ERR,
    STRING_SUBSTR_LEN_ERR,
    STRING_SUBSTR_ARGUMENT_ERR,
    EXCEPTION_HAS_NO_MESSAGE_ERR,
    EXCEPTION_MESSAGE_IS_NOT_STRING_ERR,
    EXCEPTION_HAS_NO_STACK_TRACE_ERR,
    STACK_TRACE_IS_NOT_ARRAY_ERR,
    STACK_TRACE_LINE_IS_NOT_ASSOC_ERR,
    STACK_TRACE_LINE_HAS_NO_LINE_NUMBER_ERR,
    STACK_TRACE_LINE_HAS_NO_FUNC_NAME_ERR,
    EXCEPTION_IS_NOT_ASSOC_ERR,
    EXCEPTION_HAS_NO_PRINT_STACK_TRACE_METHOD_ERR,
    PRINT_STACK_TRACE_IS_NOT_CLOSURE_ERR,
    BAD_MULTIBYTE_CHARACTER_ERR,
    EXCEPTION_CLASS_IS_NOT_ASSOC_ERR,
    EXCEPTION_CLASS_HAS_NO_CREATE_METHOD_ERR,
    ARGUMENT_TYPE_MISMATCH_ERR,
    UNEXPECTED_WIDE_STRING_ERR,
    ONIG_SEARCH_FAIL_ERR,
    GROUP_INDEX_OVERFLOW_ERR,
    NO_SUCH_GROUP_INDEX_ERR,
    BREAK_OR_CONTINUE_REACHED_TOPLEVEL_ERR,
    ASSIGN_TO_FINAL_VARIABLE_ERR,
    FUNCTION_NOT_FOUND_ERR,
    RUNTIME_ERROR_COUNT_PLUS_1
} RuntimeError;

typedef struct Expression_tag Expression;

typedef enum {
    BOOLEAN_EXPRESSION = 1,
    INT_EXPRESSION,
    DOUBLE_EXPRESSION,
    STRING_EXPRESSION,
    REGEXP_EXPRESSION,
    IDENTIFIER_EXPRESSION,
    COMMA_EXPRESSION,
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
    LOGICAL_NOT_EXPRESSION,
    FUNCTION_CALL_EXPRESSION,
    MEMBER_EXPRESSION,
    NULL_EXPRESSION,
    ARRAY_EXPRESSION,
    INDEX_EXPRESSION,
    INCREMENT_EXPRESSION,
    DECREMENT_EXPRESSION,
    CLOSURE_EXPRESSION,
    EXPRESSION_TYPE_COUNT_PLUS_1
} ExpressionType;

#define crb_is_numeric_type(type)\
  ((type) == CRB_INT_VALUE || (type) == CRB_DOUBLE_VALUE)
#define crb_is_math_operator(operator) \
  ((operator) == ADD_EXPRESSION || (operator) == SUB_EXPRESSION\
   || (operator) == MUL_EXPRESSION || (operator) == DIV_EXPRESSION\
   || (operator) == MOD_EXPRESSION)

#define crb_is_compare_operator(operator) \
  ((operator) == EQ_EXPRESSION || (operator) == NE_EXPRESSION\
   || (operator) == GT_EXPRESSION || (operator) == GE_EXPRESSION\
   || (operator) == LT_EXPRESSION || (operator) == LE_EXPRESSION)

#define crb_is_logical_operator(operator) \
  ((operator) == LOGICAL_AND_EXPRESSION || (operator) == LOGICAL_OR_EXPRESSION)

typedef struct ArgumentList_tag {
    Expression *expression;
    struct ArgumentList_tag *next;
} ArgumentList;

typedef struct {
    Expression  *left;
    Expression  *right;
} CommaExpression;

typedef enum {
    NORMAL_ASSIGN = 1,
    ADD_ASSIGN,
    SUB_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN
} AssignmentOperator;

typedef struct {
    CRB_Boolean is_final;
    AssignmentOperator  operator;
    Expression  *left;
    Expression  *operand;
} AssignExpression;

typedef struct {
    Expression  *left;
    Expression  *right;
} BinaryExpression;

typedef struct {
    Expression          *function;
    ArgumentList        *argument;
} FunctionCallExpression;

typedef struct ExpressionList_tag {
    Expression          *expression;
    struct ExpressionList_tag   *next;
} ExpressionList;

typedef struct {
    Expression  *array;
    Expression  *index;
} IndexExpression;

typedef struct {
    Expression          *expression;
    char                *member_name;
} MemberExpression;

typedef struct {
    Expression  *operand;
} IncrementOrDecrement;

typedef struct {
    CRB_FunctionDefinition *function_definition;
} ClosureExpression;

struct CRB_Regexp_tag {
    CRB_Boolean is_literal;
    regex_t     *regexp;
    struct CRB_Regexp_tag *next;
};

struct Expression_tag {
    ExpressionType type;
    int line_number;
    union {
        CRB_Boolean             boolean_value;
        int                     int_value;
        double                  double_value;
        CRB_Char                *string_value;
        CRB_Regexp              *regexp_value;
        char                    *identifier;
        CommaExpression         comma;
        AssignExpression        assign_expression;
        BinaryExpression        binary_expression;
        Expression              *minus_expression;
        Expression              *logical_not;
        FunctionCallExpression  function_call_expression;
        MemberExpression        member_expression;
        ExpressionList          *array_literal;
        IndexExpression         index_expression;
        IncrementOrDecrement    inc_dec;
        ClosureExpression       closure;
    } u;
};

typedef struct Statement_tag Statement;

typedef struct StatementList_tag {
    Statement   *statement;
    struct StatementList_tag    *next;
} StatementList;

struct CRB_Block_tag {
    StatementList       *statement_list;
};

typedef struct IdentifierList_tag {
    char        *name;
    struct IdentifierList_tag   *next;
} IdentifierList;

typedef struct {
    IdentifierList      *identifier_list;
} GlobalStatement;

typedef struct Elsif_tag {
    Expression  *condition;
    CRB_Block   *block;
    struct Elsif_tag    *next;
} Elsif;

typedef struct {
    Expression  *condition;
    CRB_Block   *then_block;
    Elsif       *elsif_list;
    CRB_Block   *else_block;
} IfStatement;

typedef struct {
    char        *label;
    Expression  *condition;
    CRB_Block   *block;
} WhileStatement;

typedef struct {
    char        *label;
    Expression  *init;
    Expression  *condition;
    Expression  *post;
    CRB_Block   *block;
} ForStatement;

typedef struct {
    char        *label;
    char        *variable;
    Expression  *collection;
    CRB_Block   *block;
} ForeachStatement;

typedef struct {
    Expression *return_value;
} ReturnStatement;

typedef struct {
    char        *label;
} BreakStatement;

typedef struct {
    char        *label;
} ContinueStatement;

typedef struct {
    CRB_Block   *try_block;
    CRB_Block   *catch_block;
    char        *exception;
    CRB_Block   *finally_block;
} TryStatement;

typedef struct {
    Expression  *exception;
} ThrowStatement;

typedef enum {
    EXPRESSION_STATEMENT = 1,
    GLOBAL_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    FOREACH_STATEMENT,
    RETURN_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    TRY_STATEMENT,
    THROW_STATEMENT,
    STATEMENT_TYPE_COUNT_PLUS_1
} StatementType;

struct Statement_tag {
    StatementType       type;
    int                 line_number;
    union {
        Expression      *expression_s;
        GlobalStatement global_s;
        IfStatement     if_s;
        WhileStatement  while_s;
        ForStatement    for_s;
        ForeachStatement        foreach_s;
        BreakStatement  break_s;
        ContinueStatement       continue_s;
        ReturnStatement return_s;
        TryStatement    try_s;
        ThrowStatement  throw_s;
    } u;
};

struct CRB_ParameterList_tag {
    char        *name;
    struct CRB_ParameterList_tag *next;
};


typedef struct Variable_tag {
    CRB_Boolean is_final;
    char        *name;
    CRB_Value   value;
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
        CRB_Value       return_value;
        char            *label;
    } u;
} StatementResult;

typedef struct GlobalVariableRef_tag {
    char        *name;
    Variable    *variable;
    struct GlobalVariableRef_tag *next;
} GlobalVariableRef;

typedef struct RefInNativeFunc_tag {
    CRB_Object  *object;
    struct RefInNativeFunc_tag *next;
} RefInNativeFunc;

struct CRB_LocalEnvironment_tag {
    char                *current_function_name;
    int                 caller_line_number;
    CRB_Object          *variable;      /* ScopeChain */
    GlobalVariableRef   *global_variable;
    RefInNativeFunc     *ref_in_native_method;
    struct CRB_LocalEnvironment_tag     *next;
};

typedef struct {
    int         stack_alloc_size;
    int         stack_pointer;
    CRB_Value   *stack;
} Stack;

typedef struct {
    int         current_heap_size;
    int         current_threshold;
    CRB_Object  *header;
} Heap;

typedef struct {
    jmp_buf     environment;
} RecoveryEnvironment;

typedef enum {
    EUC_ENCODING = 1,
    SHIFT_JIS_ENCODING,
    UTF_8_ENCODING
} Encoding;

struct CRB_Interpreter_tag {
    MEM_Storage         interpreter_storage;
    MEM_Storage         execute_storage;
    Variable            *variable;
    CRB_FunctionDefinition      *function_list;
    StatementList       *statement_list;
    int                 current_line_number;
    Stack               stack;
    Heap                heap;
    CRB_LocalEnvironment        *top_environment;
    CRB_Value           current_exception;
    RecoveryEnvironment current_recovery_environment;
    CRB_InputMode       input_mode;
    CRB_Regexp          *regexp_literals;
    Encoding            source_encoding;
};

struct CRB_Array_tag {
    int         size;
    int         alloc_size;
    CRB_Value   *array;
};

struct CRB_String_tag {
    CRB_Boolean is_literal;
    CRB_Char    *string;
};

typedef struct {
    char        *name;
    CRB_Value   value;
    CRB_Boolean is_final;
} AssocMember;

struct CRB_Assoc_tag {
    int         member_count;
    AssocMember *member;
};

typedef struct {
    CRB_Object  *frame; /* CRB_Assoc */
    CRB_Object  *next;  /* ScopeChain */
} ScopeChain;

typedef struct {
    void                        *pointer;
    CRB_NativePointerInfo       *info;
} NativePointer;

typedef enum {
    ARRAY_OBJECT = 1,
    STRING_OBJECT,
    ASSOC_OBJECT,
    SCOPE_CHAIN_OBJECT,
    NATIVE_POINTER_OBJECT,
    OBJECT_TYPE_COUNT_PLUS_1
} ObjectType;

#define crb_is_object_value(type) \
  ((type) == CRB_STRING_VALUE || (type) == CRB_ARRAY_VALUE\
   || (type) == CRB_ASSOC_VALUE || (type) == CRB_NATIVE_POINTER_VALUE\
   || (type) == CRB_SCOPE_CHAIN_VALUE)

struct CRB_Object_tag {
    ObjectType  type;
    unsigned int        marked:1;
    union {
        CRB_Array       array;
        CRB_String      string;
        CRB_Assoc       assoc;
        ScopeChain      scope_chain;
        NativePointer   native_pointer;
    } u;
    struct CRB_Object_tag *prev;
    struct CRB_Object_tag *next;
};

typedef struct {
    CRB_Char    *string;
} VString;

/* crowbar.l */
void crb_set_source_string(char **source);

/* create.c */
void crb_function_define(char *identifier, CRB_ParameterList *parameter_list,
                         CRB_Block *block);
CRB_ParameterList *crb_create_parameter(char *identifier);
CRB_ParameterList *crb_chain_parameter(CRB_ParameterList *list,
                                       char *identifier);
ArgumentList *crb_create_argument_list(Expression *expression);
ArgumentList *crb_chain_argument_list(ArgumentList *list, Expression *expr);
ExpressionList *crb_create_expression_list(Expression *expression);
ExpressionList *crb_chain_expression_list(ExpressionList *list,
                                          Expression *expr);
StatementList *crb_create_statement_list(Statement *statement);
StatementList *crb_chain_statement_list(StatementList *list,
                                        Statement *statement);
Expression *crb_alloc_expression(ExpressionType type);
Expression *crb_create_comma_expression(Expression *left, Expression *right);
Expression *crb_create_assign_expression(CRB_Boolean is_final,
                                         Expression *left,
                                         AssignmentOperator operator,
                                         Expression *operand);
Expression *crb_create_binary_expression(ExpressionType operator,
                                         Expression *left,
                                         Expression *right);
Expression *crb_create_minus_expression(Expression *operand);
Expression *crb_create_logical_not_expression(Expression *operand);
Expression *crb_create_index_expression(Expression *array, Expression *index);
Expression *crb_create_incdec_expression(Expression *operand,
                                         ExpressionType inc_or_dec);
Expression *crb_create_identifier_expression(char *identifier);
Expression *crb_create_function_call_expression(Expression *function,
                                                ArgumentList *argument);
Expression *crb_create_member_expression(Expression *expression,
                                         char *member_name);
Expression *crb_create_boolean_expression(CRB_Boolean value);
Expression *crb_create_null_expression(void);
Expression *crb_create_array_expression(ExpressionList *list);
Expression *crb_create_closure_definition(char *identifier,
                                          CRB_ParameterList *parameter_list,
                                          CRB_Block *block);
Statement *crb_create_global_statement(IdentifierList *identifier_list);
IdentifierList *crb_create_global_identifier(char *identifier);
IdentifierList *crb_chain_identifier(IdentifierList *list, char *identifier);
Statement *crb_create_if_statement(Expression *condition,
                                   CRB_Block *then_block, Elsif *elsif_list,
                                   CRB_Block *else_block);
Elsif *crb_chain_elsif_list(Elsif *list, Elsif *add);
Elsif *crb_create_elsif(Expression *expr, CRB_Block *block);
Statement *crb_create_while_statement(char *label,
                                      Expression *condition, CRB_Block *block);
Statement *
crb_create_foreach_statement(char *label, char *variable,
                             Expression *collection, CRB_Block *block);
Statement *crb_create_for_statement(char *label,
                                    Expression *init, Expression *cond,
                                    Expression *post, CRB_Block *block);
CRB_Block *crb_create_block(StatementList *statement_list);
Statement *crb_create_expression_statement(Expression *expression);
Statement *crb_create_return_statement(Expression *expression);
Statement *crb_create_break_statement(char *label);
Statement *crb_create_continue_statement(char *label);
Statement *crb_create_try_statement(CRB_Block *try_block, char *exception,
                                    CRB_Block *catch_block,
                                    CRB_Block *finally_block);
Statement *crb_create_throw_statement(Expression *expression);

/* string.c */
char *crb_create_identifier(char *str);
void crb_open_string_literal(void);
void crb_add_string_literal(int letter);
void crb_reset_string_literal_buffer(void);
CRB_Char *crb_close_string_literal(void);
void crb_set_regexp_start_char(char ch);
char crb_regexp_start_char(void);

/* execute.c */
StatementResult
crb_execute_statement_list(CRB_Interpreter *inter,
                           CRB_LocalEnvironment *env, StatementList *list);

/* eval.c */
int crb_get_stack_pointer(CRB_Interpreter *inter);
void crb_set_stack_pointer(CRB_Interpreter *inter, int stack_pointer);
CRB_Value *crb_get_identifier_lvalue(CRB_Interpreter *inter,
                                     CRB_LocalEnvironment *env,
                                     int line_number, char *identifier);
CRB_Value crb_eval_binary_expression(CRB_Interpreter *inter,
                                     CRB_LocalEnvironment *env,
                                     ExpressionType operator,
                                     Expression *left, Expression *right);
CRB_Value crb_eval_minus_expression(CRB_Interpreter *inter,
                                    CRB_LocalEnvironment *env,
                                    Expression *operand);
CRB_Value crb_eval_expression(CRB_Interpreter *inter,
                              CRB_LocalEnvironment *env, Expression *expr);

CRB_Value *
crb_eval_and_peek_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                             Expression *expr);
/* heap.c */
CRB_Object *crb_create_crowbar_string_i(CRB_Interpreter *inter, CRB_Char *str);
CRB_Object *crb_create_array_i(CRB_Interpreter *inter, int size);
CRB_Object *crb_create_assoc_i(CRB_Interpreter *inter);
CRB_Object *crb_literal_to_crb_string_i(CRB_Interpreter *inter, CRB_Char *str);
CRB_Object *crb_string_substr_i(CRB_Interpreter *inter,
                                CRB_LocalEnvironment *env,
                                CRB_Object *str,
                                int form, int len, int line_number);
CRB_Object *crb_create_native_pointer_i(CRB_Interpreter *inter,
                                        void *pointer,
                                        CRB_NativePointerInfo *info);
CRB_Object *crb_create_scope_chain(CRB_Interpreter *inter);
void crb_garbage_collect(CRB_Interpreter *inter);


/* util.c */
CRB_Interpreter *crb_get_current_interpreter(void);
void crb_set_current_interpreter(CRB_Interpreter *inter);
void *crb_malloc(size_t size);
void *crb_execute_malloc(CRB_Interpreter *inter, size_t size);
Variable *crb_search_global_variable(CRB_Interpreter *inter, char *identifier);
CRB_NativeFunctionProc *
crb_search_native_function(CRB_Interpreter *inter, char *name);
CRB_FunctionDefinition *crb_search_function_in_compile(char *name);
char *crb_get_operator_string(ExpressionType type);
void crb_vstr_clear(VString *v);
void crb_vstr_append_string(VString *v, CRB_Char *str);
void crb_vstr_append_character(VString *v, CRB_Char ch);

/* error.c */
void crb_compile_error(CompileError id, ...);
void crb_runtime_error(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                       int line_number, RuntimeError id, ...);

/* native.c */
void crb_add_native_functions(CRB_Interpreter *inter);
void crb_add_std_fp(CRB_Interpreter *inter);

/* regexp.c */
CRB_Regexp *crb_create_regexp_in_compile(CRB_Char *str);
void crb_dispose_regexp_literals(CRB_Interpreter *inter);
CRB_NativePointerInfo *crb_get_regexp_info(void);
void crb_add_regexp_functions(CRB_Interpreter *inter);


#endif /* PRIVATE_CROWBAR_H_INCLUDED */
