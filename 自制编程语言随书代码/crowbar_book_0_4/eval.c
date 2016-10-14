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

int
crb_get_stack_pointer(CRB_Interpreter *inter)
{
    return inter->stack.stack_pointer;
}

void
crb_set_stack_pointer(CRB_Interpreter *inter, int stack_pointer)
{
    inter->stack.stack_pointer = stack_pointer;
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
    v.u.object = crb_literal_to_crb_string_i(inter, string_value);
    push_value(inter, &v);
}

static void
eval_regexp_expression(CRB_Interpreter *inter, CRB_Regexp *regexp_value)
{
    CRB_Value   v;

    v.type = CRB_NATIVE_POINTER_VALUE;
    v.u.object = crb_create_native_pointer_i(inter, regexp_value,
                                             crb_get_regexp_info());
    push_value(inter, &v);
}

static void
eval_null_expression(CRB_Interpreter *inter)
{
    CRB_Value   v;

    v.type = CRB_NULL_VALUE;

    push_value(inter, &v);
}

static CRB_Value *
search_global_variable_from_env(CRB_Interpreter *inter,
                                CRB_LocalEnvironment *env, char *name,
                                CRB_Boolean *is_final)
{
    GlobalVariableRef *pos;

    if (env == NULL) {
        return CRB_search_global_variable_w(inter, name, is_final);
    }

    for (pos = env->global_variable; pos; pos = pos->next) {
        if (!strcmp(pos->name, name)) {
            return &pos->variable->value;
        }
    }

    return NULL;
}

static void
eval_identifier_expression(CRB_Interpreter *inter,
                           CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value *vp;
    CRB_FunctionDefinition *func;
    CRB_Boolean is_final; /* dummy */

    vp = CRB_search_local_variable(env, expr->u.identifier);
    if (vp != NULL) {
        push_value(inter, vp);
        return;
    }

    vp = search_global_variable_from_env(inter, env, expr->u.identifier,
                                         &is_final);
    if (vp != NULL) {
        push_value(inter, vp);
        return;
    }

    func = CRB_search_function(inter, expr->u.identifier);
    if (func != NULL) {
        CRB_Value       v;
        v.type = CRB_CLOSURE_VALUE;
        v.u.closure.function = func;
        v.u.closure.environment = NULL;
        push_value(inter, &v);
        return;
    }

    crb_runtime_error(inter, env, expr->line_number, VARIABLE_NOT_FOUND_ERR,
                      CRB_STRING_MESSAGE_ARGUMENT,
                      "name", expr->u.identifier,
                      CRB_MESSAGE_ARGUMENT_END);
}

static void eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                            Expression *expr);

CRB_Value *
crb_get_identifier_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          int line_number, char *identifier)
{
    CRB_Value *left;
    CRB_Boolean is_final = CRB_FALSE;

    left = CRB_search_local_variable_w(env, identifier, &is_final);
    if (left == NULL) {
        left = search_global_variable_from_env(inter, env, identifier,
                                               &is_final);
    }
    if (is_final) {
        crb_runtime_error(inter, env, line_number,
                          ASSIGN_TO_FINAL_VARIABLE_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "name", identifier,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    return left;
}

static void
eval_comma_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                      Expression *expr)
{
    eval_expression(inter, env, expr->u.comma.left);
    pop_value(inter);
    eval_expression(inter, env, expr->u.comma.right);
}

static CRB_Value *
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
        crb_runtime_error(inter, env, expr->line_number,
                          INDEX_OPERAND_NOT_ARRAY_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    if (index.type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, expr->line_number,
                          INDEX_OPERAND_NOT_INT_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    if (index.u.int_value < 0
        || index.u.int_value >= array.u.object->u.array.size) {
        crb_runtime_error(inter, env, expr->line_number,
                          ARRAY_INDEX_OUT_OF_BOUNDS_ERR,
                          CRB_INT_MESSAGE_ARGUMENT,
                          "size", array.u.object->u.array.size,
                          CRB_INT_MESSAGE_ARGUMENT, "index", index.u.int_value,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    return &array.u.object->u.array.array[index.u.int_value];
}

static CRB_Value *
get_member_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                  Expression *expr)
{
    CRB_Value assoc;
    CRB_Value *dest;
    CRB_Boolean is_final = CRB_FALSE;

    eval_expression(inter, env, expr->u.member_expression.expression);
    assoc = pop_value(inter);

    if (assoc.type != CRB_ASSOC_VALUE) {
        crb_runtime_error(inter, env, expr->line_number,
                          NOT_OBJECT_MEMBER_UPDATE_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    dest = CRB_search_assoc_member_w(assoc.u.object,
                                     expr->u.member_expression.member_name,
                                     &is_final);
    if (is_final) {
        crb_runtime_error(inter, env, expr->line_number,
                          ASSIGN_TO_FINAL_VARIABLE_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "name",
                          expr->u.member_expression.member_name,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    return dest;
}

static CRB_Value *
get_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
               Expression *expr)
{
    CRB_Value   *dest;

    if (expr->type == IDENTIFIER_EXPRESSION) {
        dest = crb_get_identifier_lvalue(inter, env, expr->line_number,
                                         expr->u.identifier);
    } else if (expr->type == INDEX_EXPRESSION) {
        dest = get_array_element_lvalue(inter, env, expr);
    } else if (expr->type == MEMBER_EXPRESSION) {
        dest = get_member_lvalue(inter, env, expr);
    } else {
        crb_runtime_error(inter, env, expr->line_number, NOT_LVALUE_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    return dest;
}

static void
eval_binary_int(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                ExpressionType operator,
                int left, int right,
                CRB_Value *result, int line_number)
{
    if (crb_is_math_operator(operator)) {
        result->type = CRB_INT_VALUE;
    } else if (crb_is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    } else {
        DBG_assert(crb_is_logical_operator(operator),
                   ("operator..%d\n", operator));
        crb_runtime_error(inter, env, line_number,
                          LOGICAL_OP_INTEGER_OPERAND_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    switch (operator) {
    case BOOLEAN_EXPRESSION:    /* FALLTHRU */
    case INT_EXPRESSION:        /* FALLTHRU */
    case DOUBLE_EXPRESSION:     /* FALLTHRU */
    case STRING_EXPRESSION:     /* FALLTHRU */
    case REGEXP_EXPRESSION:     /* FALLTHRU */
    case IDENTIFIER_EXPRESSION: /* FALLTHRU */
    case COMMA_EXPRESSION:      /* FALLTHRU */
    case ASSIGN_EXPRESSION:
        DBG_assert(0, ("bad case...%d", operator));
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
        if (right == 0) {
            crb_runtime_error(inter, env, line_number, DIVISION_BY_ZERO_ERR,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        result->u.int_value = left / right;
        break;
    case MOD_EXPRESSION:
        if (right == 0) {
            crb_runtime_error(inter, env, line_number, DIVISION_BY_ZERO_ERR,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        result->u.int_value = left % right;
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
    case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
    case LOGICAL_OR_EXPRESSION: /* FALLTHRU */
    case MINUS_EXPRESSION:      /* FALLTHRU */
    case LOGICAL_NOT_EXPRESSION:        /* FALLTHRU */
    case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
    case MEMBER_EXPRESSION:     /* FALLTHRU */
    case NULL_EXPRESSION:       /* FALLTHRU */
    case ARRAY_EXPRESSION:      /* FALLTHRU */
    case CLOSURE_EXPRESSION:    /* FALLTHRU */
    case INDEX_EXPRESSION:      /* FALLTHRU */
    case INCREMENT_EXPRESSION:  /* FALLTHRU */
    case DECREMENT_EXPRESSION:  /* FALLTHRU */
    case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
    default:
        DBG_assert(0, ("bad case...%d", operator));
    }
}

static void
eval_binary_double(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                   ExpressionType operator,
                   double left, double right,
                   CRB_Value *result, int line_number)
{
    if (crb_is_math_operator(operator)) {
        result->type = CRB_DOUBLE_VALUE;
    } else if (crb_is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    } else {
        DBG_assert(crb_is_logical_operator(operator),
                   ("operator..%d\n", operator));
        crb_runtime_error(inter, env, line_number,
                          LOGICAL_OP_DOUBLE_OPERAND_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    switch (operator) {
    case BOOLEAN_EXPRESSION:    /* FALLTHRU */
    case INT_EXPRESSION:        /* FALLTHRU */
    case DOUBLE_EXPRESSION:     /* FALLTHRU */
    case STRING_EXPRESSION:     /* FALLTHRU */
    case REGEXP_EXPRESSION:     /* FALLTHRU */
    case IDENTIFIER_EXPRESSION: /* FALLTHRU */
    case COMMA_EXPRESSION:      /* FALLTHRU */
    case ASSIGN_EXPRESSION:
        DBG_assert(0, ("bad case...%d", operator));
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
    case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
    case LOGICAL_OR_EXPRESSION:         /* FALLTHRU */
    case MINUS_EXPRESSION:              /* FALLTHRU */
    case LOGICAL_NOT_EXPRESSION:        /* FALLTHRU */
    case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
    case MEMBER_EXPRESSION:     /* FALLTHRU */
    case NULL_EXPRESSION:               /* FALLTHRU */
    case ARRAY_EXPRESSION:      /* FALLTHRU */
    case CLOSURE_EXPRESSION:    /* FALLTHRU */
    case INDEX_EXPRESSION:      /* FALLTHRU */
    case INCREMENT_EXPRESSION:
    case DECREMENT_EXPRESSION:
    case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
    default:
        DBG_assert(0, ("bad default...%d", operator));
    }
}

void
eval_binary_numeric(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    ExpressionType operator,
                    CRB_Value *left_val, CRB_Value *right_val,
                    CRB_Value *result, int line_number)
{
    if (left_val->type == CRB_INT_VALUE
        && right_val->type == CRB_INT_VALUE) {
        eval_binary_int(inter, env, operator,
                        left_val->u.int_value, right_val->u.int_value,
                        result, line_number);
    } else if (left_val->type == CRB_DOUBLE_VALUE
               && right_val->type == CRB_DOUBLE_VALUE) {
        eval_binary_double(inter, env, operator,
                           left_val->u.double_value, right_val->u.double_value,
                           result, line_number);
    } else if (left_val->type == CRB_INT_VALUE
               && right_val->type == CRB_DOUBLE_VALUE) {
        eval_binary_double(inter, env, operator,
                           (double)left_val->u.int_value,
                           right_val->u.double_value,
                           result, line_number);
    } else if (left_val->type == CRB_DOUBLE_VALUE
               && right_val->type == CRB_INT_VALUE) {
        eval_binary_double(inter, env, operator,
                           left_val->u.double_value,
                           (double)right_val->u.int_value,
                           result, line_number);
    }
}

void
chain_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
             int line_number, CRB_Value *left, CRB_Value *right,
             CRB_Value *result)
{
    CRB_Char    *right_str;
    CRB_Object *right_obj;
    int         len;
    CRB_Char    *str;

    right_str = CRB_value_to_string(inter, env, line_number, right);
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
do_assign(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
          CRB_Value *src, CRB_Value *dest,
          AssignmentOperator operator, int line_number)
{
    ExpressionType expr_type;
    CRB_Value result;

    if (operator == NORMAL_ASSIGN) {
        *dest = *src;
    } else {
        switch (operator) {
        case NORMAL_ASSIGN:
            DBG_panic(("NORMAL_ASSIGN.\n"));
        case ADD_ASSIGN:
            expr_type = ADD_EXPRESSION;
            break;
        case SUB_ASSIGN:
            expr_type = SUB_EXPRESSION;
            break;
        case MUL_ASSIGN:
            expr_type = MUL_EXPRESSION;
            break;
        case DIV_ASSIGN:
            expr_type = DIV_EXPRESSION;
            break;
        case MOD_ASSIGN:
            expr_type = MOD_EXPRESSION;
            break;
        default:
            DBG_panic(("bad default.\n"));
        }
        if (dest->type == CRB_STRING_VALUE
            && expr_type == ADD_EXPRESSION) {
            chain_string(inter, env, line_number, dest, src, &result);
        } else {
            eval_binary_numeric(inter, env, expr_type,
                                dest, src, &result, line_number);
        }
        *dest = result;
    }
}

static void
assign_to_member(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                 Expression *expr, CRB_Value *src)
{
    CRB_Value *assoc;
    CRB_Value *dest;
    Expression *left = expr->u.assign_expression.left;
    CRB_Boolean is_final;

    eval_expression(inter, env, left->u.member_expression.expression);
    assoc = peek_stack(inter, 0);
    if (assoc->type != CRB_ASSOC_VALUE) {
        crb_runtime_error(inter, env, expr->line_number,
                          NOT_OBJECT_MEMBER_ASSIGN_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    dest = CRB_search_assoc_member_w(assoc->u.object,
                                     left->u.member_expression.member_name,
                                     &is_final);
    if (dest == NULL) {
        if (expr->u.assign_expression.operator != NORMAL_ASSIGN) {
            crb_runtime_error(inter, env, expr->line_number,
                              NO_SUCH_MEMBER_ERR,
                              CRB_STRING_MESSAGE_ARGUMENT, "member_name",
                              left->u.member_expression.member_name,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        CRB_add_assoc_member(inter, assoc->u.object,
                             left->u.member_expression.member_name,
                             src, expr->u.assign_expression.is_final);
    } else {
        if (is_final) {
            crb_runtime_error(inter, env, expr->line_number,
                              ASSIGN_TO_FINAL_VARIABLE_ERR,
                              CRB_STRING_MESSAGE_ARGUMENT, "name",
                              left->u.member_expression.member_name,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        do_assign(inter, env, src, dest, expr->u.assign_expression.operator,
                  expr->line_number);
    }
    pop_value(inter);
}

static void
eval_assign_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                       Expression *expr)
{
    CRB_Value   *src;
    CRB_Value   *dest;
    Expression  *left = expr->u.assign_expression.left;

    eval_expression(inter, env, expr->u.assign_expression.operand);
    src = peek_stack(inter, 0);

    if (left->type == MEMBER_EXPRESSION) {
        assign_to_member(inter, env, expr, src);
        return;
    }

    dest = get_lvalue(inter, env, left);
    if (left->type == IDENTIFIER_EXPRESSION && dest == NULL) {
        if (expr->u.assign_expression.operator != NORMAL_ASSIGN) {
            crb_runtime_error(inter, env, expr->line_number,
                              VARIABLE_NOT_FOUND_ERR,
                              CRB_STRING_MESSAGE_ARGUMENT, "name",
                              left->u.identifier,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        if (env != NULL) {
            CRB_add_local_variable(inter, env, left->u.identifier, src,
                                   expr->u.assign_expression.is_final);
        } else {
            if (CRB_search_function(inter, left->u.identifier)) {
                crb_runtime_error(inter, env, expr->line_number,
                                  FUNCTION_EXISTS_ERR,
                                  CRB_STRING_MESSAGE_ARGUMENT, "name",
                                  left->u.identifier,
                                  CRB_MESSAGE_ARGUMENT_END);
            }
            CRB_add_global_variable(inter, left->u.identifier, src,
                                    expr->u.assign_expression.is_final);
        }
    } else {
        DBG_assert(dest != NULL, ("dest == NULL.\n"));
        do_assign(inter, env, src, dest, expr->u.assign_expression.operator,
                  expr->line_number);
    }
}

static CRB_Boolean
eval_binary_boolean(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    ExpressionType operator,
                    CRB_Boolean left, CRB_Boolean right, int line_number)
{
    CRB_Boolean result;

    if (operator == EQ_EXPRESSION) {
        result = left == right;
    } else if (operator == NE_EXPRESSION) {
        result = left != right;
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(inter, env, line_number, NOT_BOOLEAN_OPERATOR_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    return result;
}

static CRB_Boolean
eval_compare_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    ExpressionType operator,
                    CRB_Value *left, CRB_Value *right, int line_number)
{
    CRB_Boolean result;
    int cmp;

    cmp = CRB_wcscmp(left->u.object->u.string.string,
                     right->u.object->u.string.string);

    if (operator == EQ_EXPRESSION) {
        return cmp == 0;
    } else if (operator == NE_EXPRESSION) {
        return cmp != 0;
    } else if (operator == GT_EXPRESSION) {
        return cmp > 0;
    } else if (operator == GE_EXPRESSION) {
        return cmp >= 0;
    } else if (operator == LT_EXPRESSION) {
        return cmp < 0;
    } else if (operator == LE_EXPRESSION) {
        return cmp <= 0;
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(inter, env, line_number, BAD_OPERATOR_FOR_STRING_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    return result;
}

static CRB_Boolean
eval_binary_null(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                 ExpressionType operator,
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
        crb_runtime_error(inter, env, line_number, NOT_NULL_OPERATOR_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    return result;
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

    if (crb_is_numeric_type(left_val->type)
        && crb_is_numeric_type(right_val->type)) {
        eval_binary_numeric(inter, env, operator, left_val, right_val,
                            &result, left->line_number);
    } else if (left_val->type == CRB_BOOLEAN_VALUE
               && right_val->type == CRB_BOOLEAN_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value
            = eval_binary_boolean(inter, env, operator,
                                  left_val->u.boolean_value,
                                  right_val->u.boolean_value,
                                  left->line_number);
    } else if (left_val->type == CRB_STRING_VALUE
               && operator == ADD_EXPRESSION) {
        chain_string(inter, env, right->line_number,
                     left_val, right_val, &result);
    } else if (left_val->type == CRB_STRING_VALUE
               && right_val->type == CRB_STRING_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value
            = eval_compare_string(inter, env, operator, left_val, right_val,
                                  left->line_number);
    } else if (left_val->type == CRB_NULL_VALUE
               || right_val->type == CRB_NULL_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value
            = eval_binary_null(inter, env, operator, left_val, right_val,
                               left->line_number);
    } else if (crb_is_object_value(left_val->type)
               && crb_is_object_value(right_val->type)) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value
            = (left_val->u.object == right_val->u.object);
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(inter, env, left->line_number, BAD_OPERAND_TYPE_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          CRB_MESSAGE_ARGUMENT_END);
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
        crb_runtime_error(inter, env, left->line_number, NOT_BOOLEAN_TYPE_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
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
    CRB_Value   *operand_val;

    eval_expression(inter, env, operand);
    operand_val = peek_stack(inter, 0);
    if (operand_val->type == CRB_INT_VALUE) {
        operand_val->u.int_value = -operand_val->u.int_value;
    } else if (operand_val->type == CRB_DOUBLE_VALUE) {
        operand_val->u.double_value = -operand_val->u.double_value;
    } else {
        crb_runtime_error(inter, env, operand->line_number,
                          MINUS_OPERAND_TYPE_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
}

CRB_Value
crb_eval_minus_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          Expression *operand)
{
    eval_minus_expression(inter, env, operand);
    return pop_value(inter);
}

static void
eval_logical_not_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                            Expression *operand)
{
    CRB_Value   *operand_val;

    eval_expression(inter, env, operand);
    operand_val = peek_stack(inter, 0);

    if (operand_val->type != CRB_BOOLEAN_VALUE) {
        crb_runtime_error(inter, env, operand->line_number,
                          NOT_BOOLEAN_TYPE_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    operand_val->u.boolean_value = !operand_val->u.boolean_value;
}

static CRB_LocalEnvironment *
alloc_local_environment(CRB_Interpreter *inter, char *func_name,
                        int caller_line_number, CRB_Object *closure_env)
{
    CRB_LocalEnvironment *ret;

    ret = MEM_malloc(sizeof(CRB_LocalEnvironment));
    ret->next = inter->top_environment;
    inter->top_environment = ret;

    ret->current_function_name = func_name;
    ret->caller_line_number = caller_line_number;
    ret->ref_in_native_method = NULL; /* to stop marking by GC */
    ret->variable = NULL; /* to stop marking by GC */
    ret->variable = crb_create_scope_chain(inter);
    ret->variable->u.scope_chain.frame = crb_create_assoc_i(inter);
    ret->variable->u.scope_chain.next = closure_env;
    ret->global_variable = NULL;

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
    GlobalVariableRef *ref;

    CRB_LocalEnvironment *temp = inter->top_environment;
    inter->top_environment = inter->top_environment->next;

    while (temp->global_variable) {
        ref = temp->global_variable;
        temp->global_variable = ref->next;
        MEM_free(ref);
    }
    dispose_ref_in_native_method(temp);

    MEM_free(temp);
}

static void
call_crowbar_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                      CRB_LocalEnvironment *caller_env,
                      Expression *expr, CRB_Value *func)
{
    CRB_Value   value;
    StatementResult     result;
    ArgumentList        *arg_p;
    CRB_ParameterList   *param_p;

    for (arg_p = expr->u.function_call_expression.argument,
             param_p = func->u.closure.function->u.crowbar_f.parameter;
         arg_p;

         arg_p = arg_p->next, param_p = param_p->next) {
        CRB_Value *arg_val;

        if (param_p == NULL) {
            crb_runtime_error(inter, caller_env, expr->line_number,
                              ARGUMENT_TOO_MANY_ERR,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        eval_expression(inter, caller_env, arg_p->expression);
        arg_val = peek_stack(inter, 0);
        CRB_add_local_variable(inter, env, param_p->name, arg_val,
                               CRB_FALSE);
        pop_value(inter);
    }
    if (param_p) {
        crb_runtime_error(inter, caller_env, expr->line_number,
                          ARGUMENT_TOO_FEW_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    result = crb_execute_statement_list(inter, env,
                                        func->u.closure.function
                                        ->u.crowbar_f.block->statement_list);

    if (result.type == RETURN_STATEMENT_RESULT) {
        value = result.u.return_value;
    } else {
        value.type = CRB_NULL_VALUE;
    }
    
    push_value(inter, &value);
}

static void
call_native_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                     CRB_LocalEnvironment *caller_env,
                     Expression *expr, CRB_NativeFunctionProc *proc)
{
    CRB_Value   value;
    int         arg_count;
    ArgumentList        *arg_p;
    CRB_Value   *args = NULL;

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
check_method_argument_count(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                            int line_number,
                            int arg_count, int param_count)
{
    if (arg_count < param_count) {
        crb_runtime_error(inter, env, line_number, ARGUMENT_TOO_FEW_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    } else if (arg_count > param_count) {
        crb_runtime_error(inter, env, line_number, ARGUMENT_TOO_MANY_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
}

typedef struct {
    ObjectType  type;
    char        *name;
    int         argument_count;
    void        (*func)(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                        CRB_Object *obj, CRB_Value *result);
} FakeMethodTable;

static void
array_add_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                 CRB_Object *obj, CRB_Value *result)
{
    CRB_Value *add;

    add = peek_stack(inter, 0);
    CRB_array_add(inter, obj, add);
    result->type = CRB_NULL_VALUE;
}

static void
array_size_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                  CRB_Object *obj, CRB_Value *result)
{
    result->type = CRB_INT_VALUE;
    result->u.int_value = obj->u.array.size;
}

static void
array_resize_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    CRB_Object *obj, CRB_Value *result)
{
    CRB_Value *new_size;

    new_size = peek_stack(inter, 0);
    if (new_size->type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, __LINE__,
                          ARRAY_RESIZE_ARGUMENT_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT,
                          "type", CRB_get_type_name(new_size->type),
                          CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_array_resize(inter, obj, new_size->u.int_value);
    result->type = CRB_NULL_VALUE;
}

static void
array_insert_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    CRB_Object *obj, CRB_Value *result)
{
    CRB_Value *new_value;
    CRB_Value *pos;

    new_value = peek_stack(inter, 0);
    pos = peek_stack(inter, 1);
    if (pos->type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, __LINE__,
                          ARRAY_INSERT_ARGUMENT_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT,
                          "type", CRB_get_type_name(pos->type),
                          CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_array_insert(inter, env, obj, pos->u.int_value,
                     new_value, __LINE__);
    result->type = CRB_NULL_VALUE;
}

static void
array_remove_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    CRB_Object *obj, CRB_Value *result)
{
    CRB_Value *pos;

    pos = peek_stack(inter, 0);
    if (pos->type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, __LINE__,
                          ARRAY_REMOVE_ARGUMENT_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT,
                          "type", CRB_get_type_name(pos->type),
                          CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_array_remove(inter, env, obj, pos->u.int_value,
                     __LINE__);
    result->type = CRB_NULL_VALUE;
}

static void
array_iterator_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    CRB_Object *obj, CRB_Value *result)
{
    CRB_Value ret;
    CRB_Value array;

    array.type = CRB_ARRAY_VALUE;
    array.u.object = obj;

    ret = CRB_call_function_by_name(inter, env, __LINE__,
                                    ARRAY_ITERATOR_METHOD_NAME,
                                    1, &array);
    *result = ret;
}

static void
string_length_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                     CRB_Object *obj, CRB_Value *result)
{
    result->type = CRB_INT_VALUE;
    result->u.int_value = CRB_wcslen(obj->u.string.string);
}

static void
string_substr_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                     CRB_Object *obj, CRB_Value *result)
{
    CRB_Value *arg1;
    CRB_Value *arg2;

    arg1 = peek_stack(inter, 1);
    arg2 = peek_stack(inter, 0);

    if (arg1->type != CRB_INT_VALUE || arg2->type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, __LINE__,
                          STRING_SUBSTR_ARGUMENT_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT,
                          "type1", CRB_get_type_name(arg1->type),
                          CRB_STRING_MESSAGE_ARGUMENT,
                          "type2", CRB_get_type_name(arg2->type),
                          CRB_MESSAGE_ARGUMENT_END);
    }
    result->type = CRB_STRING_VALUE;
    result->u.object
        = crb_string_substr_i(inter, env, obj,
                              arg1->u.int_value, arg2->u.int_value,
                              __LINE__);
}

static FakeMethodTable st_fake_method_table[] = {
    {ARRAY_OBJECT, "add", 1, array_add_method},
    {ARRAY_OBJECT, "size", 0, array_size_method},
    {ARRAY_OBJECT, "resize", 1, array_resize_method},
    {ARRAY_OBJECT, "insert", 2, array_insert_method},
    {ARRAY_OBJECT, "remove", 1, array_remove_method},
    {ARRAY_OBJECT, "iterator", 0, array_iterator_method},
    {STRING_OBJECT, "length", 0, string_length_method},
    {STRING_OBJECT, "substr", 2, string_substr_method},
};

static FakeMethodTable *
search_fake_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                   int line_number, CRB_FakeMethod *fm)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(st_fake_method_table); i++) {
        if (fm->object->type == st_fake_method_table[i].type
            && !strcmp(fm->method_name, st_fake_method_table[i].name)) {
            break;
        }
    }
    if (i == ARRAY_SIZE(st_fake_method_table)) {
        crb_runtime_error(inter, env, line_number, NO_SUCH_METHOD_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "method_name",
                          fm->method_name,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    return &st_fake_method_table[i];
}

static void
call_fake_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                 CRB_LocalEnvironment *caller_env,
                 Expression *expr, CRB_FakeMethod *fm)
{
    CRB_Value           result;
    ArgumentList        *arg_p;
    int arg_count = 0;
    FakeMethodTable *fmt;

    fmt = search_fake_method(inter, env, expr->line_number, fm);

    for (arg_p = expr->u.function_call_expression.argument;
         arg_p; arg_p = arg_p->next) {
        arg_count++;
    }
    check_method_argument_count(inter, env, expr->line_number,
                                arg_count,
                                fmt->argument_count);
    for (arg_p = expr->u.function_call_expression.argument;
         arg_p; arg_p = arg_p->next) {
        eval_expression(inter, caller_env, arg_p->expression);
    }
    fmt->func(inter, env, fm->object, &result);
    shrink_stack(inter, arg_count);
    push_value(inter, &result);
}

static void
do_function_call(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                 CRB_LocalEnvironment *caller_env, Expression *expr,
                 CRB_Value *func)
{
    if (func->type == CRB_FAKE_METHOD_VALUE) {
        call_fake_method(inter, env, caller_env, expr, &func->u.fake_method);
        return;
    }

    DBG_assert(func->type == CRB_CLOSURE_VALUE,
               ("func->type..%d\n", func->type));
    switch (func->u.closure.function->type) {
    case CRB_CROWBAR_FUNCTION_DEFINITION:
        call_crowbar_function(inter, env, caller_env, expr, func);
        break;
    case CRB_NATIVE_FUNCTION_DEFINITION:
        call_native_function(inter, env, caller_env, expr,
                             func->u.closure.function->u.native_f.proc);
        break;
    case CRB_FUNCTION_DEFINITION_TYPE_COUNT_PLUS_1:
    default:
        DBG_assert(0, ("bad case..%d\n", func->u.closure.function->type));
    }

}

static void
eval_function_call_expression(CRB_Interpreter *inter,
                              CRB_LocalEnvironment *env,
                              Expression *expr)
{
    CRB_Value   *func;
    CRB_LocalEnvironment        *local_env;
    CRB_Object                  *closure_env;
    char                        *func_name;
    RecoveryEnvironment env_backup;
    CRB_Value   return_value;
    int stack_pointer_backup;

    eval_expression(inter, env,
                    expr->u.function_call_expression.function);
    func = peek_stack(inter, 0);
    if (func->type == CRB_CLOSURE_VALUE) {
        func_name = func->u.closure.function->name;
        closure_env = func->u.closure.environment;
    } else if (func->type == CRB_FAKE_METHOD_VALUE) {
        func_name = func->u.fake_method.method_name;
        closure_env = NULL;
    } else {
        crb_runtime_error(inter, env, expr->line_number,
                          NOT_FUNCTION_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    
    local_env = alloc_local_environment(inter, func_name, expr->line_number,
                                        closure_env);
    if (func->type == CRB_CLOSURE_VALUE
        && func->u.closure.function->is_closure
        && func->u.closure.function->name) {
        CRB_add_assoc_member(inter,
                             local_env->variable->u.scope_chain.frame,
                             func->u.closure.function->name,
                             func, CRB_TRUE);
    }

    stack_pointer_backup = crb_get_stack_pointer(inter);
    env_backup = inter->current_recovery_environment;
    if (setjmp(inter->current_recovery_environment.environment) == 0) {
        do_function_call(inter, local_env, env, expr, func);
    } else {
        dispose_local_environment(inter);
        crb_set_stack_pointer(inter, stack_pointer_backup);
        longjmp(env_backup.environment, LONGJMP_ARG);
    }
    inter->current_recovery_environment = env_backup;
    dispose_local_environment(inter);

    return_value = pop_value(inter);
    pop_value(inter); /* func */
    push_value(inter, &return_value);
}

static CRB_Value
call_crowbar_function_from_native(CRB_Interpreter *inter,
                                  CRB_LocalEnvironment *env,
                                  int line_number,
                                  CRB_LocalEnvironment *caller_env,
                                  CRB_Value *func,
                                  int arg_count, CRB_Value *args)
{
    CRB_Value   value;
    StatementResult     result;
    int arg_idx;
    CRB_ParameterList   *param_p;

    for (arg_idx = 0,
             param_p = func->u.closure.function->u.crowbar_f.parameter;
         arg_idx < arg_count;
         arg_idx++, param_p = param_p->next) {
        if (param_p == NULL) {
            crb_runtime_error(inter, caller_env, line_number,
                              ARGUMENT_TOO_MANY_ERR,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        CRB_add_local_variable(inter, env, param_p->name, &args[arg_idx],
                               CRB_FALSE);
    }
    if (param_p) {
        crb_runtime_error(inter, caller_env, line_number,
                          ARGUMENT_TOO_FEW_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    result = crb_execute_statement_list(inter, env,
                                        func->u.closure.function
                                        ->u.crowbar_f.block->statement_list);

    if (result.type == RETURN_STATEMENT_RESULT) {
        value = result.u.return_value;
    } else {
        value.type = CRB_NULL_VALUE;
    }
    
    return value;
}

static CRB_Value
call_native_function_from_native(CRB_Interpreter *inter,
                                 CRB_LocalEnvironment *env,
                                 int line_number,
                                 CRB_LocalEnvironment *caller_env,
                                 CRB_Value *func,
                                 int arg_count, CRB_Value *args)
{
    CRB_Value value;
    int i;
    CRB_Value *arg_p;

    for (i = 0; i < arg_count; i++) {
        push_value(inter, &args[i]);
    }
    arg_p = &inter->stack.stack[inter->stack.stack_pointer-arg_count];
    value = func->u.closure.function->u.native_f.proc(inter, env,
                                                      arg_count, arg_p);
    shrink_stack(inter, arg_count);

    return value;
}

static CRB_Value
call_fake_method_from_native(CRB_Interpreter *inter,
                             CRB_LocalEnvironment *env,
                             int line_number,
                             CRB_LocalEnvironment *caller_env,
                             CRB_Value *func,
                             int arg_count, CRB_Value *args)
{
    CRB_Value value;
    FakeMethodTable *fmt;
    int i;

    fmt = search_fake_method(inter, env, line_number, &func->u.fake_method);
    check_method_argument_count(inter, env, line_number, arg_count,
                                fmt->argument_count);
    for (i = 0; i < arg_count; i++) {
        push_value(inter, &args[i]);
    }
    fmt->func(inter, env, func->u.fake_method.object, &value);
    shrink_stack(inter, arg_count);

    return value;
}


/* 
 * See also eval_function_call_expression().
 */
CRB_Value
CRB_call_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                  int line_number, CRB_Value *func,
                  int arg_count, CRB_Value *args)
{
    CRB_Value ret;
    CRB_LocalEnvironment        *local_env;
    CRB_Object                  *closure_env;
    char                        *func_name;
    RecoveryEnvironment env_backup;
    int stack_pointer_backup;

    if (func->type == CRB_CLOSURE_VALUE) {
        func_name = func->u.closure.function->name;
        closure_env = func->u.closure.environment;
    } else if (func->type == CRB_FAKE_METHOD_VALUE) {
        func_name = func->u.fake_method.method_name;
        closure_env = NULL;
    } else {
        DBG_panic(("func->type..%d\n", func->type));
    }
    local_env
        = alloc_local_environment(inter, func_name, line_number, closure_env);
    if (func->type == CRB_CLOSURE_VALUE
        && func->u.closure.function->is_closure
        && func->u.closure.function->name) {
        CRB_add_assoc_member(inter,
                             local_env->variable->u.scope_chain.frame,
                             func->u.closure.function->name,
                             func, CRB_TRUE);
    }

    stack_pointer_backup = crb_get_stack_pointer(inter);
    env_backup = inter->current_recovery_environment;
    if (setjmp(inter->current_recovery_environment.environment) == 0) {
        if (func->type == CRB_CLOSURE_VALUE) {
            switch (func->u.closure.function->type) {
            case CRB_CROWBAR_FUNCTION_DEFINITION:
                ret = call_crowbar_function_from_native(inter, local_env,
                                                        line_number, env, func,
                                                        arg_count, args);
                break;
            case CRB_NATIVE_FUNCTION_DEFINITION:
                ret = call_native_function_from_native(inter, local_env,
                                                       line_number, env,
                                                       func, arg_count, args);
                break;
            case CRB_FUNCTION_DEFINITION_TYPE_COUNT_PLUS_1:
            default:
                DBG_assert(0, ("bad case..%d\n",
                               func->u.closure.function->type));
            }
        } else if (func->type == CRB_FAKE_METHOD_VALUE) {
            ret = call_fake_method_from_native(inter, local_env,
                                               line_number, env,
                                               func, arg_count, args);
        } else {
            DBG_panic(("func->type..%d\n", func->type));
        }
    } else {
        dispose_local_environment(inter);
        crb_set_stack_pointer(inter, stack_pointer_backup);
        longjmp(env_backup.environment, LONGJMP_ARG);
    }
    inter->current_recovery_environment = env_backup;
    dispose_local_environment(inter);

    return ret;
}

CRB_Value
CRB_call_function_by_name(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          int line_number, char *func_name,
                          int arg_count, CRB_Value *args)
{
    CRB_FunctionDefinition *fd;
    CRB_Value func;
    CRB_Value ret;

    fd = CRB_search_function(inter, func_name);
    if (fd == NULL) {
        crb_runtime_error(inter, env, line_number,
                          FUNCTION_NOT_FOUND_ERR, "name", func_name,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    func = CRB_create_closure(env, fd);

    ret = CRB_call_function(inter, env, line_number, &func, arg_count, args);

    return ret;
}

CRB_Value
CRB_call_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                int line_number, CRB_Object *obj, char *method_name,
                int arg_count, CRB_Value *args)
{
    CRB_Value func;
    CRB_Value result;

    if (obj->type == STRING_OBJECT || obj->type == ARRAY_OBJECT) {
        func.type = CRB_FAKE_METHOD_VALUE;
        func.u.fake_method.method_name = method_name;
        func.u.fake_method.object = obj;
    } else if (obj->type == ASSOC_OBJECT) {
        CRB_Value *func_p;
        func_p = CRB_search_assoc_member(obj, method_name);
        if (func_p->type != CRB_CLOSURE_VALUE) {
            crb_runtime_error(inter, env, line_number,
                              NOT_FUNCTION_ERR,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        func = *func_p;
    }
    push_value(inter, &func);

    result = CRB_call_function(inter, env, line_number, &func,
                               arg_count, args);

    pop_value(inter);

    return result;
}

static void
eval_member_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                       Expression *expr)
{
    CRB_Value left;

    eval_expression(inter,env, expr->u.member_expression.expression);
    left = pop_value(inter);
    
    if (left.type == CRB_ASSOC_VALUE) {
        CRB_Value *v;
        v = CRB_search_assoc_member(left.u.object,
                                    expr->u.member_expression.member_name);
        if (v == NULL) {
            crb_runtime_error(inter, env, expr->line_number,
                              NO_SUCH_MEMBER_ERR,
                              CRB_STRING_MESSAGE_ARGUMENT, "member_name",
                              expr->u.member_expression.member_name,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        push_value(inter, v);
    } else if (left.type == CRB_STRING_VALUE
               || left.type == CRB_ARRAY_VALUE) {
        CRB_Value v;
        v.type = CRB_FAKE_METHOD_VALUE;
        v.u.fake_method.method_name = expr->u.member_expression.member_name;
        v.u.fake_method.object = left.u.object;
        push_value(inter, &v);
    } else {
        crb_runtime_error(inter, env, expr->line_number,
                          NO_MEMBER_TYPE_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
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
    if (operand == NULL) {
        crb_runtime_error(inter, env, expr->line_number,
                          INC_DEC_OPERAND_NOT_EXIST_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    if (operand->type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, expr->line_number,
                          INC_DEC_OPERAND_TYPE_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
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
eval_closure_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                        Expression *expr)
{
    CRB_Value   result;

    result.type = CRB_CLOSURE_VALUE;
    result.u.closure.function = expr->u.closure.function_definition;
    if (env) {
        result.u.closure.environment = env->variable;
    } else {
        result.u.closure.environment = NULL;
    }

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
    case REGEXP_EXPRESSION:
        eval_regexp_expression(inter, expr->u.regexp_value);
        break;
    case IDENTIFIER_EXPRESSION:
        eval_identifier_expression(inter, env, expr);
        break;
    case COMMA_EXPRESSION:
        eval_comma_expression(inter, env, expr);
        break;
    case ASSIGN_EXPRESSION:
        eval_assign_expression(inter, env, expr);
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
    case LE_EXPRESSION:
        eval_binary_expression(inter, env, expr->type,
                               expr->u.binary_expression.left,
                               expr->u.binary_expression.right);
        break;
    case LOGICAL_AND_EXPRESSION:/* FALLTHRU */
    case LOGICAL_OR_EXPRESSION: /* FALLTHRU */
        eval_logical_and_or_expression(inter, env, expr->type,
                                       expr->u.binary_expression.left,
                                       expr->u.binary_expression.right);
        break;
    case MINUS_EXPRESSION:
        eval_minus_expression(inter, env, expr->u.minus_expression);
        break;
    case LOGICAL_NOT_EXPRESSION:
        eval_logical_not_expression(inter, env, expr->u.minus_expression);
        break;
    case FUNCTION_CALL_EXPRESSION:
        eval_function_call_expression(inter, env, expr);
        break;
    case MEMBER_EXPRESSION:
        eval_member_expression(inter, env, expr);
        break;
    case NULL_EXPRESSION:
        eval_null_expression(inter);
        break;
    case ARRAY_EXPRESSION:
        eval_array_expression(inter, env, expr->u.array_literal);
        break;
    case CLOSURE_EXPRESSION:
        eval_closure_expression(inter, env, expr);
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
        DBG_assert(0, ("bad case. type..%d\n", expr->type));
    }
}

CRB_Value
crb_eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    Expression *expr)
{
    eval_expression(inter, env, expr);
    return pop_value(inter);
}

CRB_Value *
crb_eval_and_peek_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                             Expression *expr)
{
    eval_expression(inter, env, expr);
    return peek_stack(inter, 0);
}

void
CRB_push_value(CRB_Interpreter *inter, CRB_Value *value)
{
    push_value(inter, value);
}

CRB_Value
CRB_pop_value(CRB_Interpreter *inter)
{
    return pop_value(inter);
}

CRB_Value *
CRB_peek_stack(CRB_Interpreter *inter, int index)
{
    return peek_stack(inter, index);
}

void
CRB_shrink_stack(CRB_Interpreter *inter, int shrink_size)
{
    shrink_stack(inter, shrink_size);
}
