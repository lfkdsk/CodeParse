#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static void
push_value(CRB_Interpreter *inter, CRB_Value *value)
{
    DBG_assert(inter->stack.stack_pointer <= inter->stack.stack_alloc_size,
               ("stack_pointer..%d, stack_alloc_size..%d\n",
                inter->stack.stack_pointer, inter->stack.stack_alloc_size));

    if (inter->stack.stack_pointer == inter->stack.stack_alloc_size) {
        inter->stack.stack_alloc_size += STACK_ALLOC_SIZE;
        inter->stack.stack
            = MEM_realloc(inter->stack.stack,
                          sizeof(CRB_Value) * inter->stack.stack_alloc_size);
    }
    inter->stack.stack[inter->stack.stack_pointer] = *value;
    inter->stack.stack_pointer++;
}

static CRB_Value
pop_value(CRB_Interpreter *inter)
{
    CRB_Value ret;

    ret = inter->stack.stack[inter->stack.stack_pointer-1];
    inter->stack.stack_pointer--;

    return ret;
}

static CRB_Value *
peek_stack(CRB_Interpreter *inter, int index)
{
    return &inter->stack.stack[inter->stack.stack_pointer - index - 1];
}

static void
shrink_stack(CRB_Interpreter *inter, int shrink_size)
{
    inter->stack.stack_pointer -= shrink_size;
}


static void
eval_boolean_expression(CRB_Interpreter *inter, CRB_Boolean boolean_value)
{
    CRB_Value   v;

    v.type = CRB_BOOLEAN_VALUE;
    v.u.boolean_value = boolean_value;
    push_value(inter, &v);
}

static void
eval_int_expression(CRB_Interpreter *inter, int int_value)
{
    CRB_Value   v;

    v.type = CRB_INT_VALUE;
    v.u.int_value = int_value;
    push_value(inter, &v);
}

static void
eval_double_expression(CRB_Interpreter *inter, double double_value)
{
    CRB_Value   v;

    v.type = CRB_DOUBLE_VALUE;
    v.u.double_value = double_value;
    push_value(inter, &v);
}

static void
eval_string_expression(CRB_Interpreter *inter, CRB_Char *string_value)
{
    CRB_Value   v;

    v.type = CRB_STRING_VALUE;
    v.u.object = crb_literal_to_crb_string(inter, string_value);
    push_value(inter, &v);
}

static void
eval_null_expression(CRB_Interpreter *inter)
{
    CRB_Value   v;

    v.type = CRB_NULL_VALUE;

    push_value(inter, &v);
}

static Variable *
search_global_variable_from_env(CRB_Interpreter *inter,
                                CRB_LocalEnvironment *env, char *name)
{
    GlobalVariableRef *pos;

    if (env == NULL) {
        return crb_search_global_variable(inter, name);
    }

    for (pos = env->global_variable; pos; pos = pos->next) {
        if (!strcmp(pos->variable->name, name)) {
            return pos->variable;
        }
    }

    return NULL;
}

static void
eval_identifier_expression(CRB_Interpreter *inter,
                           CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value   v;
    Variable    *vp;

    vp = crb_search_local_variable(env, expr->u.identifier);
    if (vp != NULL) {
        v = vp->value;
    } else {
        vp = search_global_variable_from_env(inter, env, expr->u.identifier);
        if (vp != NULL) {
            v = vp->value;
        } else {
            crb_runtime_error(expr->line_number, VARIABLE_NOT_FOUND_ERR,
                              STRING_MESSAGE_ARGUMENT,
                              "name", expr->u.identifier,
                              MESSAGE_ARGUMENT_END);
        }
    }
    push_value(inter, &v);
}

static void eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                            Expression *expr);

static CRB_Value *
get_identifier_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                      char *identifier)
{
    Variable *new_var;
    Variable *left;

    left = crb_search_local_variable(env, identifier);
    if (left == NULL) {
        left = search_global_variable_from_env(inter, env, identifier);
    }
    if (left != NULL)
        return &left->value;

    if (env != NULL) {
        new_var = crb_add_local_variable(env, identifier);
        left = new_var;
    } else {
        new_var = crb_add_global_variable(inter, identifier);
        left = new_var;
    }

    return &left->value;
}


CRB_Value *
get_array_element_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                         Expression *expr)
{
    CRB_Value   array;
    CRB_Value   index;

    eval_expression(inter, env, expr->u.index_expression.array);
    eval_expression(inter, env, expr->u.index_expression.index);
    index = pop_value(inter);
    array = pop_value(inter);

    if (array.type != CRB_ARRAY_VALUE) {
        crb_runtime_error(expr->line_number, INDEX_OPERAND_NOT_ARRAY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (index.type != CRB_INT_VALUE) {
        crb_runtime_error(expr->line_number, INDEX_OPERAND_NOT_INT_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    if (index.u.int_value < 0
        || index.u.int_value >= array.u.object->u.array.size) {
        crb_runtime_error(expr->line_number, ARRAY_INDEX_OUT_OF_BOUNDS_ERR,
                          INT_MESSAGE_ARGUMENT,
                          "size", array.u.object->u.array.size,
                          INT_MESSAGE_ARGUMENT, "index", index.u.int_value,
                          MESSAGE_ARGUMENT_END);
    }
    return &array.u.object->u.array.array[index.u.int_value];
}

CRB_Value *
get_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
           Expression *expr)
{
    CRB_Value   *dest;

    if (expr->type == IDENTIFIER_EXPRESSION) {
        dest = get_identifier_lvalue(inter, env, expr->u.identifier);
    } else if (expr->type == INDEX_EXPRESSION) {
        dest = get_array_element_lvalue(inter, env, expr);
    } else {
        crb_runtime_error(expr->line_number, NOT_LVALUE_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    return dest;
}


static void
eval_assign_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                       Expression *left, Expression *expression)
{
    CRB_Value   *src;
    CRB_Value   *dest;

    eval_expression(inter, env, expression);
    src = peek_stack(inter, 0);

    dest = get_lvalue(inter, env, left);
    *dest = *src;
}


static CRB_Boolean
eval_binary_boolean(CRB_Interpreter *inter, ExpressionType operator,
                    CRB_Boolean left, CRB_Boolean right, int line_number)
{
    CRB_Boolean result;

    if (operator == EQ_EXPRESSION) {
        result = left == right;
    } else if (operator == NE_EXPRESSION) {
        result = left != right;
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(line_number, NOT_BOOLEAN_OPERATOR_ERR,
                          STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          MESSAGE_ARGUMENT_END);
    }

    return result;
}

static void
eval_binary_int(CRB_Interpreter *inter, ExpressionType operator,
                int left, int right,
                CRB_Value *result, int line_number)
{
    if (dkc_is_math_operator(operator)) {
        result->type = CRB_INT_VALUE;
    } else if (dkc_is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    } else {
        DBG_panic(("operator..%d\n", operator));
    }

    switch (operator) {
    case BOOLEAN_EXPRESSION:    /* FALLTHRU */
    case INT_EXPRESSION:        /* FALLTHRU */
    case DOUBLE_EXPRESSION:     /* FALLTHRU */
    case STRING_EXPRESSION:     /* FALLTHRU */
    case IDENTIFIER_EXPRESSION: /* FALLTHRU */
    case ASSIGN_EXPRESSION:
        DBG_panic(("bad case...%d", operator));
        break;
    case ADD_EXPRESSION:
        result->u.int_value = left + right;
        break;
    case SUB_EXPRESSION:
        result->u.int_value = left - right;
        break;
    case MUL_EXPRESSION:
        result->u.int_value = left * right;
        break;
    case DIV_EXPRESSION:
        result->u.int_value = left / right;
        break;
    case MOD_EXPRESSION:
        result->u.int_value = left % right;
        break;
    case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
    case LOGICAL_OR_EXPRESSION:
        DBG_panic(("bad case...%d", operator));
        break;
    case EQ_EXPRESSION:
        result->u.boolean_value = left == right;
        break;
    case NE_EXPRESSION:
        result->u.boolean_value = left != right;
        break;
    case GT_EXPRESSION:
        result->u.boolean_value = left > right;
        break;
    case GE_EXPRESSION:
        result->u.boolean_value = left >= right;
        break;
    case LT_EXPRESSION:
        result->u.boolean_value = left < right;
        break;
    case LE_EXPRESSION:
        result->u.boolean_value = left <= right;
        break;
    case MINUS_EXPRESSION:              /* FALLTHRU */
    case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
    case METHOD_CALL_EXPRESSION:        /* FALLTHRU */
    case NULL_EXPRESSION:       /* FALLTHRU */
    case ARRAY_EXPRESSION:      /* FALLTHRU */
    case INDEX_EXPRESSION:      /* FALLTHRU */
    case INCREMENT_EXPRESSION:  /* FALLTHRU */
    case DECREMENT_EXPRESSION:  /* FALLTHRU */
    case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
    default:
        DBG_panic(("bad case...%d", operator));
    }
}

static void
eval_binary_double(CRB_Interpreter *inter, ExpressionType operator,
                   double left, double right,
                   CRB_Value *result, int line_number)
{
    if (dkc_is_math_operator(operator)) {
        result->type = CRB_DOUBLE_VALUE;
    } else if (dkc_is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    } else {
        DBG_panic(("operator..%d\n", operator));
    }

    switch (operator) {
    case BOOLEAN_EXPRESSION:    /* FALLTHRU */
    case INT_EXPRESSION:        /* FALLTHRU */
    case DOUBLE_EXPRESSION:     /* FALLTHRU */
    case STRING_EXPRESSION:     /* FALLTHRU */
    case IDENTIFIER_EXPRESSION: /* FALLTHRU */
    case ASSIGN_EXPRESSION:
        DBG_panic(("bad case...%d", operator));
        break;
    case ADD_EXPRESSION:
        result->u.double_value = left + right;
        break;
    case SUB_EXPRESSION:
        result->u.double_value = left - right;
        break;
    case MUL_EXPRESSION:
        result->u.double_value = left * right;
        break;
    case DIV_EXPRESSION:
        result->u.double_value = left / right;
        break;
    case MOD_EXPRESSION:
        result->u.double_value = fmod(left, right);
        break;
    case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
    case LOGICAL_OR_EXPRESSION:
        DBG_panic(("bad case...%d", operator));
        break;
    case EQ_EXPRESSION:
        result->u.int_value = left == right;
        break;
    case NE_EXPRESSION:
        result->u.int_value = left != right;
        break;
    case GT_EXPRESSION:
        result->u.int_value = left > right;
        break;
    case GE_EXPRESSION:
        result->u.int_value = left >= right;
        break;
    case LT_EXPRESSION:
        result->u.int_value = left < right;
        break;
    case LE_EXPRESSION:
        result->u.int_value = left <= right;
        break;
    case MINUS_EXPRESSION:              /* FALLTHRU */
    case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
    case METHOD_CALL_EXPRESSION:        /* FALLTHRU */
    case NULL_EXPRESSION:               /* FALLTHRU */
    case ARRAY_EXPRESSION:      /* FALLTHRU */
    case INDEX_EXPRESSION:      /* FALLTHRU */
    case INCREMENT_EXPRESSION:
    case DECREMENT_EXPRESSION:
    case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
    default:
        DBG_panic(("bad default...%d", operator));
    }
}

static CRB_Boolean
eval_compare_string(ExpressionType operator,
                    CRB_Value *left, CRB_Value *right, int line_number)
{
    CRB_Boolean result;
    int cmp;

    cmp = CRB_wcscmp(left->u.object->u.string.string,
                     right->u.object->u.string.string);

    if (operator == EQ_EXPRESSION) {
        result = (cmp == 0);
    } else if (operator == NE_EXPRESSION) {
        result = (cmp != 0);
    } else if (operator == GT_EXPRESSION) {
        result = (cmp > 0);
    } else if (operator == GE_EXPRESSION) {
        result = (cmp >= 0);
    } else if (operator == LT_EXPRESSION) {
        result = (cmp < 0);
    } else if (operator == LE_EXPRESSION) {
        result = (cmp <= 0);
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(line_number, BAD_OPERATOR_FOR_STRING_ERR,
                          STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          MESSAGE_ARGUMENT_END);
    }

    return result;
}

static CRB_Boolean
eval_binary_null(CRB_Interpreter *inter, ExpressionType operator,
                 CRB_Value *left, CRB_Value *right, int line_number)
{
    CRB_Boolean result;

    if (operator == EQ_EXPRESSION) {
        result = left->type == CRB_NULL_VALUE && right->type == CRB_NULL_VALUE;
    } else if (operator == NE_EXPRESSION) {
        result =  !(left->type == CRB_NULL_VALUE
                    && right->type == CRB_NULL_VALUE);
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(line_number, NOT_NULL_OPERATOR_ERR,
                          STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          MESSAGE_ARGUMENT_END);
    }

    return result;
}

void
chain_string(CRB_Interpreter *inter, CRB_Value *left, CRB_Value *right,
             CRB_Value *result)
{
    CRB_Char    *right_str;
    CRB_Object  *right_obj;
    int         len;
    CRB_Char    *str;

    right_str = CRB_value_to_string(right);
    right_obj = crb_create_crowbar_string_i(inter, right_str);

    result->type = CRB_STRING_VALUE;
    len = CRB_wcslen(left->u.object->u.string.string)
        + CRB_wcslen(right_obj->u.string.string);
    str = MEM_malloc(sizeof(CRB_Char) * (len + 1));
    CRB_wcscpy(str, left->u.object->u.string.string);
    CRB_wcscat(str, right_obj->u.string.string);
    result->u.object = crb_create_crowbar_string_i(inter, str);
}

static void
eval_binary_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                       ExpressionType operator,
                       Expression *left, Expression *right)
{
    CRB_Value   *left_val;
    CRB_Value   *right_val;
    CRB_Value   result;

    eval_expression(inter, env, left);
    eval_expression(inter, env, right);
    left_val = peek_stack(inter, 1);
    right_val = peek_stack(inter, 0);

    if (left_val->type == CRB_INT_VALUE
        && right_val->type == CRB_INT_VALUE) {
        eval_binary_int(inter, operator,
                        left_val->u.int_value, right_val->u.int_value,
                        &result, left->line_number);
    } else if (left_val->type == CRB_DOUBLE_VALUE
               && right_val->type == CRB_DOUBLE_VALUE) {
        eval_binary_double(inter, operator,
                           left_val->u.double_value, right_val->u.double_value,
                           &result, left->line_number);
    } else if (left_val->type == CRB_INT_VALUE
               && right_val->type == CRB_DOUBLE_VALUE) {
        eval_binary_double(inter, operator,
                           (double)left_val->u.int_value,
                           right_val->u.double_value,
                           &result, left->line_number);
    } else if (left_val->type == CRB_DOUBLE_VALUE
               && right_val->type == CRB_INT_VALUE) {
        eval_binary_double(inter, operator,
                           left_val->u.double_value,
                           (double)right_val->u.int_value,
                           &result, left->line_number);
    } else if (left_val->type == CRB_BOOLEAN_VALUE
               && right_val->type == CRB_BOOLEAN_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value
            = eval_binary_boolean(inter, operator,
                                  left_val->u.boolean_value,
                                  right_val->u.boolean_value,
                                  left->line_number);
    } else if (left_val->type == CRB_STRING_VALUE
               && operator == ADD_EXPRESSION) {
        chain_string(inter, left_val, right_val, &result);
    } else if (left_val->type == CRB_STRING_VALUE
               && right_val->type == CRB_STRING_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value
            = eval_compare_string(operator, left_val, right_val,
                                  left->line_number);
    } else if (left_val->type == CRB_NULL_VALUE
               || right_val->type == CRB_NULL_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value
            = eval_binary_null(inter, operator, left_val, right_val,
                               left->line_number);
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(left->line_number, BAD_OPERAND_TYPE_ERR,
                          STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          MESSAGE_ARGUMENT_END);
    }
    pop_value(inter);
    pop_value(inter);
    push_value(inter, &result);
}

CRB_Value
crb_eval_binary_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                           ExpressionType operator,
                           Expression *left, Expression *right)
{
    eval_binary_expression(inter, env, operator, left, right);
    return pop_value(inter);
}

static void
eval_logical_and_or_expression(CRB_Interpreter *inter,
                               CRB_LocalEnvironment *env,
                               ExpressionType operator,
                               Expression *left, Expression *right)
{
    CRB_Value   left_val;
    CRB_Value   right_val;
    CRB_Value   result;

    result.type = CRB_BOOLEAN_VALUE;
    eval_expression(inter, env, left);
    left_val = pop_value(inter);
    if (left_val.type != CRB_BOOLEAN_VALUE) {
        crb_runtime_error(left->line_number, NOT_BOOLEAN_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (operator == LOGICAL_AND_EXPRESSION) {
        if (!left_val.u.boolean_value) {
            result.u.boolean_value = CRB_FALSE;
            goto FUNC_END;
        }
    } else if (operator == LOGICAL_OR_EXPRESSION) {
        if (left_val.u.boolean_value) {
            result.u.boolean_value = CRB_TRUE;
            goto FUNC_END;
        }
    } else {
        DBG_panic(("bad operator..%d\n", operator));
    }

    eval_expression(inter, env, right);
    right_val = pop_value(inter);
    result.u.boolean_value = right_val.u.boolean_value;

  FUNC_END:
    push_value(inter, &result);
}

static void
eval_minus_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                      Expression *operand)
{
    CRB_Value   operand_val;
    CRB_Value   result;

    eval_expression(inter, env, operand);
    operand_val = pop_value(inter);
    if (operand_val.type == CRB_INT_VALUE) {
        result.type = CRB_INT_VALUE;
        result.u.int_value = -operand_val.u.int_value;
    } else if (operand_val.type == CRB_DOUBLE_VALUE) {
        result.type = CRB_DOUBLE_VALUE;
        result.u.double_value = -operand_val.u.double_value;
    } else {
        crb_runtime_error(operand->line_number, MINUS_OPERAND_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    push_value(inter, &result);
}

CRB_Value
crb_eval_minus_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          Expression *operand)
{
    eval_minus_expression(inter, env, operand);
    return pop_value(inter);
}

static CRB_LocalEnvironment *
alloc_local_environment(CRB_Interpreter *inter)
{
    CRB_LocalEnvironment *ret;

    ret = MEM_malloc(sizeof(CRB_LocalEnvironment));
    ret->variable = NULL;
    ret->global_variable = NULL;
    ret->ref_in_native_method = NULL;

    ret->next = inter->top_environment;
    inter->top_environment = ret;

    return ret;
}

static void
dispose_ref_in_native_method(CRB_LocalEnvironment *env)
{
    RefInNativeFunc     *ref;

    while (env->ref_in_native_method) {
        ref = env->ref_in_native_method;
        env->ref_in_native_method = ref->next;
        MEM_free(ref);
    }
}

static void
dispose_local_environment(CRB_Interpreter *inter)
{
    CRB_LocalEnvironment *env = inter->top_environment;

    while (env->variable) {
        Variable        *temp;
        temp = env->variable;
        env->variable = temp->next;
        MEM_free(temp);
    }
    while (env->global_variable) {
        GlobalVariableRef *ref;
        ref = env->global_variable;
        env->global_variable = ref->next;
        MEM_free(ref);
    }
    dispose_ref_in_native_method(env);

    inter->top_environment = env->next;
    MEM_free(env);
}

static void
call_native_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                     CRB_LocalEnvironment *caller_env,
                     Expression *expr, CRB_NativeFunctionProc *proc)
{
    CRB_Value   value;
    int         arg_count;
    ArgumentList        *arg_p;
    CRB_Value   *args;

    for (arg_count = 0, arg_p = expr->u.function_call_expression.argument;
         arg_p; arg_p = arg_p->next) {
        eval_expression(inter, caller_env, arg_p->expression);
        arg_count++;
    }
    args = &inter->stack.stack[inter->stack.stack_pointer-arg_count];
    value = proc(inter, env, arg_count, args);
    shrink_stack(inter, arg_count);

    push_value(inter, &value);
}

static void
call_crowbar_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                      CRB_LocalEnvironment *caller_env,
                      Expression *expr, FunctionDefinition *func)
{
    CRB_Value   value;
    StatementResult     result;
    ArgumentList        *arg_p;
    ParameterList       *param_p;


    for (arg_p = expr->u.function_call_expression.argument,
             param_p = func->u.crowbar_f.parameter;
         arg_p;
         arg_p = arg_p->next, param_p = param_p->next) {
        Variable *new_var;
        CRB_Value arg_val;

         if (param_p == NULL) {
             crb_runtime_error(expr->line_number, ARGUMENT_TOO_MANY_ERR,
                               MESSAGE_ARGUMENT_END);
         }
         eval_expression(inter, caller_env, arg_p->expression);
         arg_val = pop_value(inter);
         new_var = crb_add_local_variable(env, param_p->name);
         new_var->value = arg_val;
    }
     if (param_p) {
         crb_runtime_error(expr->line_number, ARGUMENT_TOO_FEW_ERR,
                           MESSAGE_ARGUMENT_END);
     }
     result = crb_execute_statement_list(inter, env,
                                         func->u.crowbar_f.block
                                         ->statement_list);
     if (result.type == RETURN_STATEMENT_RESULT) {
         value = result.u.return_value;
     } else {
         value.type = CRB_NULL_VALUE;
     }

     push_value(inter, &value);
}

static void
eval_function_call_expression(CRB_Interpreter *inter,
                              CRB_LocalEnvironment *env,
                              Expression *expr)
{
    FunctionDefinition  *func;
    CRB_LocalEnvironment    *local_env;

    char *identifier = expr->u.function_call_expression.identifier;

    func = crb_search_function(identifier);
    if (func == NULL) {
        crb_runtime_error(expr->line_number, FUNCTION_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", identifier,
                          MESSAGE_ARGUMENT_END);
    }

    local_env = alloc_local_environment(inter);

    switch (func->type) {
    case CROWBAR_FUNCTION_DEFINITION:
        call_crowbar_function(inter, local_env, env, expr, func);
        break;
    case NATIVE_FUNCTION_DEFINITION:
        call_native_function(inter, local_env, env, expr,
                             func->u.native_f.proc);
        break;
    case FUNCTION_DEFINITION_TYPE_COUNT_PLUS_1:
    default:
        DBG_panic(("bad case..%d\n", func->type));
    }
    dispose_local_environment(inter);
}

static void
check_method_argument_count(int line_number,
                            ArgumentList *arg_list, int arg_count)
{
    ArgumentList        *arg_p;
    int count = 0;

    for (arg_p = arg_list; arg_p; arg_p = arg_p->next) {
        count++;
    }

    if (count < arg_count) {
        crb_runtime_error(line_number, ARGUMENT_TOO_FEW_ERR,
                          MESSAGE_ARGUMENT_END);
    } else if (count > arg_count) {
        crb_runtime_error(line_number, ARGUMENT_TOO_MANY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
}

static void
eval_method_call_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                            Expression *expr)
{
    CRB_Value *left;
    CRB_Value result;
    CRB_Boolean error_flag = CRB_FALSE;

    eval_expression(inter, env, expr->u.method_call_expression.expression);
    left = peek_stack(inter, 0);

    if (left->type == CRB_ARRAY_VALUE) {
        if (!strcmp(expr->u.method_call_expression.identifier, "add")) {
            CRB_Value *add;
            check_method_argument_count(expr->line_number,
                                        expr->u.method_call_expression
                                        .argument, 1);
            eval_expression(inter, env,
                            expr->u.method_call_expression.argument
                            ->expression);
            add = peek_stack(inter, 0);
            crb_array_add(inter, left->u.object, *add);
            pop_value(inter);
            result.type = CRB_NULL_VALUE;
        } else if (!strcmp(expr->u.method_call_expression.identifier,
                           "size")) {
            check_method_argument_count(expr->line_number,
                                        expr->u.method_call_expression
                                        .argument, 0);
            result.type = CRB_INT_VALUE;
            result.u.int_value = left->u.object->u.array.size;
        } else if (!strcmp(expr->u.method_call_expression.identifier,
                           "resize")) {
            CRB_Value new_size;
            check_method_argument_count(expr->line_number,
                                        expr->u.method_call_expression
                                        .argument, 1);
            eval_expression(inter, env,
                            expr->u.method_call_expression.argument
                            ->expression);
            new_size = pop_value(inter);
            if (new_size.type != CRB_INT_VALUE) {
                crb_runtime_error(expr->line_number,
                                  ARRAY_RESIZE_ARGUMENT_ERR,
                                  MESSAGE_ARGUMENT_END);
            }
            crb_array_resize(inter, left->u.object, new_size.u.int_value);
            result.type = CRB_NULL_VALUE;
        } else {
            error_flag = CRB_TRUE;
        }

    } else if (left->type == CRB_STRING_VALUE) {
        if (!strcmp(expr->u.method_call_expression.identifier, "length")) {
            check_method_argument_count(expr->line_number,
                                        expr->u.method_call_expression
                                        .argument, 0);
            result.type = CRB_INT_VALUE;
            result.u.int_value = CRB_wcslen(left->u.object->u.string.string);
        } else {
            error_flag = CRB_TRUE;
        }
    } else {
        error_flag = CRB_TRUE;
    }
    if (error_flag) {
        crb_runtime_error(expr->line_number, NO_SUCH_METHOD_ERR,
                          STRING_MESSAGE_ARGUMENT, "method_name",
                          expr->u.method_call_expression.identifier,
                          MESSAGE_ARGUMENT_END);
    }
    pop_value(inter);
    push_value(inter, &result);
}

static void
eval_array_expression(CRB_Interpreter *inter,
                      CRB_LocalEnvironment *env, ExpressionList *list)
{
    CRB_Value   v;
    int         size;
    ExpressionList *pos;
    int         i;

    size = 0;
    for (pos = list; pos; pos = pos->next) {
        size++;
    }
    v.type = CRB_ARRAY_VALUE;
    v.u.object = crb_create_array_i(inter, size);
    push_value(inter, &v);

    for (pos = list, i = 0; pos; pos = pos->next, i++) {
        eval_expression(inter, env, pos->expression);
        v.u.object->u.array.array[i] = pop_value(inter);
    }

}

static void
eval_index_expression(CRB_Interpreter *inter,
                      CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value *left;

    left = get_array_element_lvalue(inter, env, expr);

    push_value(inter, left);
}

static void
eval_inc_dec_expression(CRB_Interpreter *inter,
                        CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value   *operand;
    CRB_Value   result;
    int         old_value;
    
    operand = get_lvalue(inter, env, expr->u.inc_dec.operand);
    if (operand->type != CRB_INT_VALUE) {
        crb_runtime_error(expr->line_number, INC_DEC_OPERAND_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    old_value = operand->u.int_value;
    if (expr->type == INCREMENT_EXPRESSION) {
        operand->u.int_value++;
    } else {
        DBG_assert(expr->type == DECREMENT_EXPRESSION,
                   ("expr->type..%d\n", expr->type));
        operand->u.int_value--;
    }

    result.type = CRB_INT_VALUE;
    result.u.int_value = old_value;
    push_value(inter, &result);
}

static void
eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                Expression *expr)
{
    switch (expr->type) {
    case BOOLEAN_EXPRESSION:
        eval_boolean_expression(inter, expr->u.boolean_value);
        break;
    case INT_EXPRESSION:
        eval_int_expression(inter, expr->u.int_value);
        break;
    case DOUBLE_EXPRESSION:
        eval_double_expression(inter, expr->u.double_value);
        break;
    case STRING_EXPRESSION:
        eval_string_expression(inter, expr->u.string_value);
        break;
    case IDENTIFIER_EXPRESSION:
        eval_identifier_expression(inter, env, expr);
        break;
    case ASSIGN_EXPRESSION:
        eval_assign_expression(inter, env,
                               expr->u.assign_expression.left,
                               expr->u.assign_expression.operand);
        break;
    case ADD_EXPRESSION:        /* FALLTHRU */
    case SUB_EXPRESSION:        /* FALLTHRU */
    case MUL_EXPRESSION:        /* FALLTHRU */
    case DIV_EXPRESSION:        /* FALLTHRU */
    case MOD_EXPRESSION:        /* FALLTHRU */
    case EQ_EXPRESSION: /* FALLTHRU */
    case NE_EXPRESSION: /* FALLTHRU */
    case GT_EXPRESSION: /* FALLTHRU */
    case GE_EXPRESSION: /* FALLTHRU */
    case LT_EXPRESSION: /* FALLTHRU */
    case LE_EXPRESSION: /* FALLTHRU */
        eval_binary_expression(inter, env, expr->type,
                               expr->u.binary_expression.left,
                               expr->u.binary_expression.right);
        break;
    case LOGICAL_AND_EXPRESSION:/* FALLTHRU */
    case LOGICAL_OR_EXPRESSION:
        eval_logical_and_or_expression(inter, env, expr->type,
                                       expr->u.binary_expression.left,
                                       expr->u.binary_expression.right);
        break;
    case MINUS_EXPRESSION:
        eval_minus_expression(inter, env, expr->u.minus_expression);
        break;
    case FUNCTION_CALL_EXPRESSION:
        eval_function_call_expression(inter, env, expr);
        break;
    case METHOD_CALL_EXPRESSION:
        eval_method_call_expression(inter, env, expr);
        break;
    case NULL_EXPRESSION:
        eval_null_expression(inter);
        break;
    case ARRAY_EXPRESSION:
        eval_array_expression(inter, env, expr->u.array_literal);
        break;
    case INDEX_EXPRESSION:
        eval_index_expression(inter, env, expr);
        break;
    case INCREMENT_EXPRESSION:  /* FALLTHRU */
    case DECREMENT_EXPRESSION:
        eval_inc_dec_expression(inter, env, expr);
        break;
    case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
    default:
        DBG_panic(("bad case. type..%d\n", expr->type));
    }
}

CRB_Value
crb_eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    Expression *expr)
{
    eval_expression(inter, env, expr);
    return pop_value(inter);
}

void
CRB_shrink_stack(CRB_Interpreter *inter, int shrink_size)
{
    shrink_stack(inter, shrink_size);
}
