#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "diksamc.h"

static Expression *
fix_identifier_expression(Block *current_block, Expression *expr,
                          Expression *parent)
{
    Declaration *decl;
    FunctionDefinition *fd;

    decl = dkc_search_declaration(expr->u.identifier.name, current_block);
    if (decl) {
        expr->type = decl->type;
        expr->u.identifier.is_function = DVM_FALSE;
        expr->u.identifier.u.declaration = decl;
        return expr;
    }
    fd = dkc_search_function(expr->u.identifier.name);
    if (fd == NULL) {
        dkc_compile_error(expr->line_number,
                          IDENTIFIER_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "name",
                          expr->u.identifier.name,
                          MESSAGE_ARGUMENT_END);
    }
    expr->type = fd->type;
    expr->u.identifier.is_function = DVM_TRUE;
    expr->u.identifier.u.function = fd;

    return expr;
}

static Expression *fix_expression(Block *current_block, Expression *expr,
                                  Expression *parent);

static Expression *
fix_comma_expression(Block *current_block, Expression *expr)
{
    expr->u.comma.left
        = fix_expression(current_block, expr->u.comma.left, expr);
    expr->u.comma.right
        = fix_expression(current_block, expr->u.comma.right, expr);
    expr->type = expr->u.comma.right->type;

    return expr;
}

static void
cast_mismatch_error(int line_number, TypeSpecifier *src, TypeSpecifier *dest)
{
    char *tmp;
    char *src_name;
    char *dest_name;

    tmp = dkc_get_type_name(src);
    src_name = dkc_strdup(tmp);
    MEM_free(tmp);

    tmp = dkc_get_type_name(dest);
    dest_name = dkc_strdup(tmp);
    MEM_free(tmp);

    dkc_compile_error(line_number,
                      CAST_MISMATCH_ERR,
                      STRING_MESSAGE_ARGUMENT, "src", src_name,
                      STRING_MESSAGE_ARGUMENT, "dest", dest_name,
                      MESSAGE_ARGUMENT_END);
}

static Expression *
alloc_cast_expression(CastType cast_type, Expression *operand)
{
    Expression *cast_expr = dkc_alloc_expression(CAST_EXPRESSION);
    cast_expr->line_number = operand->line_number;
    cast_expr->u.cast.type = cast_type;
    cast_expr->u.cast.operand = operand;

    if (cast_type == INT_TO_DOUBLE_CAST) {
        cast_expr->type = dkc_alloc_type_specifier(DVM_DOUBLE_TYPE);
    } else if (cast_type == DOUBLE_TO_INT_CAST) {
        cast_expr->type = dkc_alloc_type_specifier(DVM_INT_TYPE);
    } else if (cast_type == BOOLEAN_TO_STRING_CAST
               || cast_type == INT_TO_STRING_CAST
               || cast_type == DOUBLE_TO_STRING_CAST) {
        cast_expr->type = dkc_alloc_type_specifier(DVM_STRING_TYPE);
    }

    return cast_expr;
}

static Expression *
create_assign_cast(Expression *src, TypeSpecifier *dest)
{
    Expression *cast_expr;

    if (dkc_compare_type(src->type, dest)) {
        return src;
    }

    if (dkc_is_object(dest)
               && src->type->basic_type == DVM_NULL_TYPE) {
        DBG_assert(src->type->derive == NULL, ("derive != NULL"));
        return src;
    }

    if (src->type->basic_type == DVM_INT_TYPE
        && dest->basic_type == DVM_DOUBLE_TYPE) {
        cast_expr = alloc_cast_expression(INT_TO_DOUBLE_CAST, src);
        return cast_expr;
    } else if (src->type->basic_type == DVM_DOUBLE_TYPE
               && dest->basic_type == DVM_INT_TYPE) {
        cast_expr = alloc_cast_expression(DOUBLE_TO_INT_CAST, src);
        return cast_expr;
    } else {
        cast_mismatch_error(src->line_number, src->type, dest);
    }

    return NULL; /* make compiler happy. */
}

static Expression *
fix_assign_expression(Block *current_block, Expression *expr)
{
    Expression *operand;

    if (expr->u.assign_expression.left->kind != IDENTIFIER_EXPRESSION
        && expr->u.assign_expression.left->kind != INDEX_EXPRESSION) {
        dkc_compile_error(expr->u.assign_expression.left->line_number,
                          NOT_LVALUE_ERR, MESSAGE_ARGUMENT_END);
    }
    expr->u.assign_expression.left
        = fix_expression(current_block, expr->u.assign_expression.left,
                         expr);
    operand = fix_expression(current_block, expr->u.assign_expression.operand,
                             expr);
    expr->u.assign_expression.operand
        = create_assign_cast(operand, expr->u.assign_expression.left->type);
    expr->type = expr->u.assign_expression.left->type;

    return expr;
}

static Expression *
eval_math_expression_int(Expression *expr, int left, int right)
{
    if (expr->kind == ADD_EXPRESSION) {
        expr->u.int_value = left + right;
    } else if (expr->kind == SUB_EXPRESSION) {
        expr->u.int_value = left - right;
    } else if (expr->kind == MUL_EXPRESSION) {
        expr->u.int_value = left * right;
    } else if (expr->kind == DIV_EXPRESSION) {
        if (right == 0) {
            dkc_compile_error(expr->line_number,
                              DIVISION_BY_ZERO_IN_COMPILE_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        expr->u.int_value = left / right;
    } else if (expr->kind == MOD_EXPRESSION) {
        expr->u.int_value = left % right;
    } else {
        DBG_assert(0, ("expr->kind..%d\n", expr->kind));
    }
    expr->kind = INT_EXPRESSION;
    expr->type = dkc_alloc_type_specifier(DVM_INT_TYPE);

    return expr;
}

static Expression *
eval_math_expression_double(Expression *expr, double left, double right)
{
    if (expr->kind == ADD_EXPRESSION) {
        expr->u.double_value = left + right;
    } else if (expr->kind == SUB_EXPRESSION) {
        expr->u.double_value = left - right;
    } else if (expr->kind == MUL_EXPRESSION) {
        expr->u.double_value = left * right;
    } else if (expr->kind == DIV_EXPRESSION) {
        expr->u.double_value = left / right;
    } else if (expr->kind == MOD_EXPRESSION) {
        expr->u.double_value = fmod(left, right);
    } else {
        DBG_assert(0, ("expr->kind..%d\n", expr->kind));
    }
    expr->kind = DOUBLE_EXPRESSION;
    expr->type = dkc_alloc_type_specifier(DVM_DOUBLE_TYPE);

    return expr;
}

static Expression *
chain_string(Expression *expr)
{
    DVM_Char *left_str = expr->u.binary_expression.left->u.string_value;
    DVM_Char *right_str;
    int         len;
    DVM_Char    *new_str;

    right_str = dkc_expression_to_string(expr->u.binary_expression.right);
    if (!right_str) {
        return expr;
    }
    
    len = dvm_wcslen(left_str) + dvm_wcslen(right_str);
    new_str = MEM_malloc(sizeof(DVM_Char) * (len + 1));
    dvm_wcscpy(new_str, left_str);
    dvm_wcscat(new_str, right_str);
    MEM_free(left_str);
    MEM_free(right_str);
    expr->kind = STRING_EXPRESSION;
    expr->type = dkc_alloc_type_specifier(DVM_STRING_TYPE);
    expr->u.string_value = new_str;

    return expr;
}

static Expression *
eval_math_expression(Block *current_block, Expression *expr)
{
    if (expr->u.binary_expression.left->kind == INT_EXPRESSION
        && expr->u.binary_expression.right->kind == INT_EXPRESSION) {
        expr = eval_math_expression_int(expr,
                                        expr->u.binary_expression.left
                                        ->u.int_value,
                                        expr->u.binary_expression.right
                                        ->u.int_value);
    } else if (expr->u.binary_expression.left->kind == DOUBLE_EXPRESSION
               && expr->u.binary_expression.right->kind == DOUBLE_EXPRESSION) {
        expr = eval_math_expression_double(expr,
                                           expr->u.binary_expression.left
                                           ->u.double_value,
                                           expr->u.binary_expression.right
                                           ->u.double_value);

    } else if (expr->u.binary_expression.left->kind == INT_EXPRESSION
               && expr->u.binary_expression.right->kind == DOUBLE_EXPRESSION) {
        expr = eval_math_expression_double(expr,
                                           expr->u.binary_expression.left
                                           ->u.int_value,
                                           expr->u.binary_expression.right
                                           ->u.double_value);

    } else if (expr->u.binary_expression.left->kind == DOUBLE_EXPRESSION
               && expr->u.binary_expression.right->kind == INT_EXPRESSION) {
        expr = eval_math_expression_double(expr,
                                           expr->u.binary_expression.left
                                           ->u.double_value,
                                           expr->u.binary_expression.right
                                           ->u.int_value);

    } else if (expr->u.binary_expression.left->kind == STRING_EXPRESSION
               && expr->kind == ADD_EXPRESSION) {
        expr = chain_string(expr);
    }

    return expr;
}

static Expression *
cast_binary_expression(Expression *expr)
{
    Expression *cast_expression;

    if (dkc_is_int(expr->u.binary_expression.left->type)
        && dkc_is_double(expr->u.binary_expression.right->type)) {
        cast_expression
            = alloc_cast_expression(INT_TO_DOUBLE_CAST, 
                                    expr->u.binary_expression.left);
        expr->u.binary_expression.left = cast_expression;

    } else if (dkc_is_double(expr->u.binary_expression.left->type)
               && dkc_is_int(expr->u.binary_expression.right->type)) {
        cast_expression
            = alloc_cast_expression(INT_TO_DOUBLE_CAST, 
                                    expr->u.binary_expression.right);
        expr->u.binary_expression.right = cast_expression;

    } else if (dkc_is_string(expr->u.binary_expression.left->type)
               && dkc_is_boolean(expr->u.binary_expression.right->type)) {
        cast_expression
            = alloc_cast_expression(BOOLEAN_TO_STRING_CAST, 
                                    expr->u.binary_expression.right);
        expr->u.binary_expression.right = cast_expression;

    } else if (dkc_is_string(expr->u.binary_expression.left->type)
               && dkc_is_int(expr->u.binary_expression.right->type)) {
        cast_expression
            = alloc_cast_expression(INT_TO_STRING_CAST, 
                                    expr->u.binary_expression.right);
        expr->u.binary_expression.right = cast_expression;

    } else if (dkc_is_string(expr->u.binary_expression.left->type)
               && dkc_is_double(expr->u.binary_expression.right->type)) {
        cast_expression
            = alloc_cast_expression(DOUBLE_TO_STRING_CAST, 
                                    expr->u.binary_expression.right);
        expr->u.binary_expression.right = cast_expression;
    }

    return expr;
}

static Expression *
fix_math_binary_expression(Block *current_block, Expression *expr)
{
    expr->u.binary_expression.left
        = fix_expression(current_block, expr->u.binary_expression.left,
                         expr);
    expr->u.binary_expression.right
        = fix_expression(current_block, expr->u.binary_expression.right,
                         expr);

    expr = eval_math_expression(current_block, expr);
    if (expr->kind == INT_EXPRESSION || expr->kind == DOUBLE_EXPRESSION
        || expr->kind == STRING_EXPRESSION) {
        return expr;
    }
    expr = cast_binary_expression(expr);

    if (dkc_is_int(expr->u.binary_expression.left->type)
        && dkc_is_int(expr->u.binary_expression.right->type)) {
        expr->type = dkc_alloc_type_specifier(DVM_INT_TYPE);
    } else if (dkc_is_double(expr->u.binary_expression.left->type)
               && dkc_is_double(expr->u.binary_expression.right->type)) {
        expr->type = dkc_alloc_type_specifier(DVM_DOUBLE_TYPE);
    } else if (expr->kind == ADD_EXPRESSION
               && ((dkc_is_string(expr->u.binary_expression.left->type)
                    && dkc_is_string(expr->u.binary_expression.right->type))
                   || (dkc_is_string(expr->u.binary_expression.left->type)
                       && expr->u.binary_expression.right->kind
                       == NULL_EXPRESSION))) {
        expr->type = dkc_alloc_type_specifier(DVM_STRING_TYPE);
    } else {
        dkc_compile_error(expr->line_number,
                          MATH_TYPE_MISMATCH_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    return expr;
}

static Expression *
eval_compare_expression_boolean(Expression *expr,
                                DVM_Boolean left, DVM_Boolean right)
{
    if (expr->kind == EQ_EXPRESSION) {
        expr->u.boolean_value = (left == right);
    } else if (expr->kind == NE_EXPRESSION) {
        expr->u.boolean_value = (left != right);
    } else {
        DBG_assert(0, ("expr->kind..%d\n", expr->kind));
    }

    expr->kind = BOOLEAN_EXPRESSION;
    expr->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);

    return expr;
}

static Expression *
eval_compare_expression_int(Expression *expr, int left, int right)
{
    if (expr->kind == EQ_EXPRESSION) {
        expr->u.boolean_value = (left == right);
    } else if (expr->kind == NE_EXPRESSION) {
        expr->u.boolean_value = (left != right);
    } else if (expr->kind == GT_EXPRESSION) {
        expr->u.boolean_value = (left > right);
    } else if (expr->kind == GE_EXPRESSION) {
        expr->u.boolean_value = (left >= right);
    } else if (expr->kind == LT_EXPRESSION) {
        expr->u.boolean_value = (left < right);
    } else if (expr->kind == LE_EXPRESSION) {
        expr->u.boolean_value = (left <= right);
    } else {
        DBG_assert(0, ("expr->kind..%d\n", expr->kind));
    }

    expr->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);
    expr->kind = BOOLEAN_EXPRESSION;

    return expr;
}

static Expression *
eval_compare_expression_double(Expression *expr, int left, int right)
{
    if (expr->kind == EQ_EXPRESSION) {
        expr->u.boolean_value = (left == right);
    } else if (expr->kind == NE_EXPRESSION) {
        expr->u.boolean_value = (left != right);
    } else if (expr->kind == GT_EXPRESSION) {
        expr->u.boolean_value = (left > right);
    } else if (expr->kind == GE_EXPRESSION) {
        expr->u.boolean_value = (left >= right);
    } else if (expr->kind == LT_EXPRESSION) {
        expr->u.boolean_value = (left < right);
    } else if (expr->kind == LE_EXPRESSION) {
        expr->u.boolean_value = (left <= right);
    } else {
        DBG_assert(0, ("expr->kind..%d\n", expr->kind));
    } 

    expr->kind = BOOLEAN_EXPRESSION;
    expr->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);

    return expr;
}

static Expression *
eval_compare_expression_string(Expression *expr,
                               DVM_Char *left, DVM_Char *right)
{
    int cmp;

    cmp = dvm_wcscmp(left, right);

    if (expr->kind == EQ_EXPRESSION) {
        expr->u.boolean_value = (cmp == 0);
    } else if (expr->kind == NE_EXPRESSION) {
        expr->u.boolean_value = (cmp != 0);
    } else if (expr->kind == GT_EXPRESSION) {
        expr->u.boolean_value = (cmp > 0);
    } else if (expr->kind == GE_EXPRESSION) {
        expr->u.boolean_value = (cmp >= 0);
    } else if (expr->kind == LT_EXPRESSION) {
        expr->u.boolean_value = (cmp < 0);
    } else if (expr->kind == LE_EXPRESSION) {
        expr->u.boolean_value = (cmp <= 0);
    } else {
        DBG_assert(0, ("expr->kind..%d\n", expr->kind));
    } 

    MEM_free(left);
    MEM_free(right);

    expr->kind = BOOLEAN_EXPRESSION;
    expr->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);

    return expr;
}


static Expression *
eval_compare_expression(Expression *expr)
{
    if (expr->u.binary_expression.left->kind == BOOLEAN_EXPRESSION
        && expr->u.binary_expression.right->kind == BOOLEAN_EXPRESSION) {
        expr = eval_compare_expression_boolean(expr,
                                               expr->u.binary_expression.left
                                               ->u.boolean_value,
                                               expr->u.binary_expression.right
                                               ->u.boolean_value);

    } else if (expr->u.binary_expression.left->kind == INT_EXPRESSION
        && expr->u.binary_expression.right->kind == INT_EXPRESSION) {
        expr = eval_compare_expression_int(expr,
                                           expr->u.binary_expression.left
                                           ->u.int_value,
                                           expr->u.binary_expression.right
                                           ->u.int_value);

    } else if (expr->u.binary_expression.left->kind == DOUBLE_EXPRESSION
               && expr->u.binary_expression.right->kind == DOUBLE_EXPRESSION) {
        expr = eval_compare_expression_double(expr,
                                              expr->u.binary_expression.left
                                              ->u.double_value,
                                              expr->u.binary_expression.right
                                              ->u.double_value);

    } else if (expr->u.binary_expression.left->kind == INT_EXPRESSION
               && expr->u.binary_expression.right->kind == DOUBLE_EXPRESSION) {
        expr = eval_compare_expression_double(expr,
                                              expr->u.binary_expression.left
                                              ->u.int_value,
                                              expr->u.binary_expression.right
                                              ->u.double_value);

    } else if (expr->u.binary_expression.left->kind == DOUBLE_EXPRESSION
               && expr->u.binary_expression.right->kind == INT_EXPRESSION) {
        expr = eval_compare_expression_double(expr,
                                              expr->u.binary_expression.left
                                              ->u.double_value,
                                              expr->u.binary_expression.right
                                              ->u.int_value);

    } else if (expr->u.binary_expression.left->kind == STRING_EXPRESSION
               && expr->u.binary_expression.right->kind == STRING_EXPRESSION) {
        expr = eval_compare_expression_string(expr,
                                              expr->u.binary_expression.left
                                              ->u.string_value,
                                              expr->u.binary_expression.right
                                              ->u.string_value);
    } else if (expr->u.binary_expression.left->kind == NULL_EXPRESSION
               && expr->u.binary_expression.right->kind == NULL_EXPRESSION) {
        expr->kind = BOOLEAN_EXPRESSION;
        expr->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);
        expr->u.boolean_value = DVM_TRUE;
    }
    return expr;
}

static Expression *
fix_compare_expression(Block *current_block, Expression *expr)
{
    expr->u.binary_expression.left
        = fix_expression(current_block, expr->u.binary_expression.left,
                         expr);
    expr->u.binary_expression.right
        = fix_expression(current_block, expr->u.binary_expression.right,
                         expr);

    expr = eval_compare_expression(expr);
    if (expr->kind == BOOLEAN_EXPRESSION) {
        return expr;
    }

    expr = cast_binary_expression(expr);

    if (!(dkc_compare_type(expr->u.binary_expression.left->type,
                           expr->u.binary_expression.right->type)
          || (dkc_is_object(expr->u.binary_expression.left->type)
              && expr->u.binary_expression.right->kind == NULL_EXPRESSION)
          || (dkc_is_object(expr->u.binary_expression.right->type)
              && expr->u.binary_expression.left->kind == NULL_EXPRESSION))) {
        dkc_compile_error(expr->line_number,
                          COMPARE_TYPE_MISMATCH_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    expr->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);

    return expr;
}

static Expression *
fix_logical_and_or_expression(Block *current_block, Expression *expr)
{
    expr->u.binary_expression.left
        = fix_expression(current_block, expr->u.binary_expression.left,
                         expr);
    expr->u.binary_expression.right
        = fix_expression(current_block, expr->u.binary_expression.right,
                         expr);
    
    if (dkc_is_boolean(expr->u.binary_expression.left->type)
        && dkc_is_boolean(expr->u.binary_expression.right->type)) {
        expr->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);
    } else {
        dkc_compile_error(expr->line_number,
                          LOGICAL_TYPE_MISMATCH_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    return expr;
}

static Expression *
fix_minus_expression(Block *current_block, Expression *expr)
{
    expr->u.minus_expression
        = fix_expression(current_block, expr->u.minus_expression, expr);

    if (!dkc_is_int(expr->u.minus_expression->type)
        && !dkc_is_double(expr->u.minus_expression->type)) {
        dkc_compile_error(expr->line_number,
                          MINUS_TYPE_MISMATCH_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    expr->type = expr->u.minus_expression->type;

    if (expr->u.minus_expression->kind == INT_EXPRESSION) {
        expr->kind = INT_EXPRESSION;
        expr->u.int_value = -expr->u.minus_expression->u.int_value;

    } else if (expr->u.minus_expression->kind == DOUBLE_EXPRESSION) {
        expr->kind = DOUBLE_EXPRESSION;
        expr->u.double_value = -expr->u.minus_expression->u.double_value;
    }


    return expr;
}

static Expression *
fix_logical_not_expression(Block *current_block, Expression *expr)
{
    expr->u.logical_not
        = fix_expression(current_block, expr->u.logical_not, expr);

    if (expr->u.logical_not->kind == BOOLEAN_EXPRESSION) {
        expr->kind = BOOLEAN_EXPRESSION;
        expr->u.boolean_value = !expr->u.logical_not->u.boolean_value;
        return expr;
    }

    if (!dkc_is_boolean(expr->u.logical_not->type)) {
        dkc_compile_error(expr->line_number,
                          LOGICAL_NOT_TYPE_MISMATCH_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    expr->type = expr->u.logical_not->type;

    return expr;
}

static void
check_argument(Block *current_block, FunctionDefinition *fd, Expression *expr)
{
    ParameterList *param;
    ArgumentList *arg;

    for (param = fd->parameter,
             arg = expr->u.function_call_expression.argument;
         param && arg;
         param = param->next, arg = arg->next) {
        arg->expression
            = fix_expression(current_block, arg->expression, expr);
        arg->expression
            = create_assign_cast(arg->expression, param->type);
    }

    if (param || arg) {
        dkc_compile_error(expr->line_number,
                          ARGUMENT_COUNT_MISMATCH_ERR,
                          MESSAGE_ARGUMENT_END);
    }
}

static Expression *
fix_function_call_expression(Block *current_block, Expression *expr)
{
    Expression *func_expr;
    FunctionDefinition *fd;

    func_expr = fix_expression(current_block,
                               expr->u.function_call_expression.function,
                               expr);

    if (func_expr->kind != IDENTIFIER_EXPRESSION) {
        dkc_compile_error(expr->line_number,
                          FUNCTION_NOT_IDENTIFIER_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    fd = dkc_search_function(func_expr->u.identifier.name);
    if (fd == NULL) {
        dkc_compile_error(expr->line_number,
                          FUNCTION_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "name",
                          func_expr->u.identifier.name,
                          MESSAGE_ARGUMENT_END);
    }

    check_argument(current_block, fd, expr);

    expr->type = dkc_alloc_type_specifier(fd->type->basic_type);
    expr->type->derive = fd->type->derive;

    return expr;
}

static Expression *
fix_array_literal_expression(Block *current_block, Expression *expr)
{
    ExpressionList *literal = expr->u.array_literal;
    TypeSpecifier *elem_type;
    ExpressionList *epos;

    if (literal == NULL) {
        dkc_compile_error(expr->line_number,
                          ARRAY_LITERAL_EMPTY_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    literal->expression = fix_expression(current_block, literal->expression,
                                         expr);
    elem_type = literal->expression->type;
    for (epos = literal->next; epos; epos = epos->next) {
        epos->expression
            = fix_expression(current_block, epos->expression, expr);
        epos->expression = create_assign_cast(epos->expression, elem_type);
    }

    expr->type = dkc_alloc_type_specifier(elem_type->basic_type);
    expr->type->derive = dkc_alloc_type_derive(ARRAY_DERIVE);
    expr->type->derive->next = elem_type->derive;

    return expr;
}

static Expression *
fix_index_expression(Block *current_block, Expression *expr)
{
    IndexExpression *ie = &expr->u.index_expression;

    ie->array = fix_expression(current_block, ie->array, expr);
    ie->index = fix_expression(current_block, ie->index, expr);

    if (ie->array->type->derive == NULL
        || ie->array->type->derive->tag != ARRAY_DERIVE) {
        dkc_compile_error(expr->line_number,
                          INDEX_LEFT_OPERAND_NOT_ARRAY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (!dkc_is_int(ie->index->type)) {
        dkc_compile_error(expr->line_number,
                          INDEX_NOT_INT_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    expr->type = dkc_alloc_type_specifier(ie->array->type->basic_type);
    expr->type->derive = ie->array->type->derive->next;

    return expr;
}

static Expression *
fix_inc_dec_expression(Block *current_block, Expression *expr)
{
    expr->u.inc_dec.operand
        = fix_expression(current_block, expr->u.inc_dec.operand, expr);

    if (!dkc_is_int(expr->u.inc_dec.operand->type)) {
        dkc_compile_error(expr->line_number,
                          INC_DEC_TYPE_MISMATCH_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    expr->type = expr->u.inc_dec.operand->type;

    return expr;
}

static Expression *
fix_array_creation_expression(Block *current_block, Expression *expr)
{
    ArrayDimension *dim_pos;
    TypeDerive *derive = NULL;
    TypeDerive *tmp_derive;

    for (dim_pos = expr->u.array_creation.dimension; dim_pos;
         dim_pos = dim_pos->next) {
        if (dim_pos->expression) {
            dim_pos->expression
                = fix_expression(current_block, dim_pos->expression, expr);
            if (!dkc_is_int(dim_pos->expression->type)) {
                dkc_compile_error(expr->line_number,
                                  ARRAY_SIZE_NOT_INT_ERR,
                                  MESSAGE_ARGUMENT_END);
            }
        }
        tmp_derive = dkc_alloc_type_derive(ARRAY_DERIVE);
        tmp_derive->next = derive;
        derive = tmp_derive;
    }

    expr->type
        = dkc_alloc_type_specifier(expr->u.array_creation.type->basic_type);
    expr->type->derive = derive;

    return expr;
}

static Expression *
fix_expression(Block *current_block, Expression *expr, Expression *parent)
{
    if (expr == NULL)
        return NULL;

    switch (expr->kind) {
    case BOOLEAN_EXPRESSION:
        expr->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);
        break;
    case INT_EXPRESSION:
        expr->type = dkc_alloc_type_specifier(DVM_INT_TYPE);
        break;
    case DOUBLE_EXPRESSION:
        expr->type = dkc_alloc_type_specifier(DVM_DOUBLE_TYPE);
        break;
    case STRING_EXPRESSION:
        expr->type = dkc_alloc_type_specifier(DVM_STRING_TYPE);
        break;
    case IDENTIFIER_EXPRESSION:
        expr = fix_identifier_expression(current_block, expr, parent);
        break;
    case COMMA_EXPRESSION:
        expr = fix_comma_expression(current_block, expr);
        break;
    case ASSIGN_EXPRESSION:
        expr = fix_assign_expression(current_block, expr);
        break;
    case ADD_EXPRESSION:        /* FALLTHRU */
    case SUB_EXPRESSION:        /* FALLTHRU */
    case MUL_EXPRESSION:        /* FALLTHRU */
    case DIV_EXPRESSION:        /* FALLTHRU */
    case MOD_EXPRESSION:        /* FALLTHRU */
        expr = fix_math_binary_expression(current_block, expr);
        break;
    case EQ_EXPRESSION: /* FALLTHRU */
    case NE_EXPRESSION: /* FALLTHRU */
    case GT_EXPRESSION: /* FALLTHRU */
    case GE_EXPRESSION: /* FALLTHRU */
    case LT_EXPRESSION: /* FALLTHRU */
    case LE_EXPRESSION: /* FALLTHRU */
        expr = fix_compare_expression(current_block, expr);
        break;
    case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
    case LOGICAL_OR_EXPRESSION:         /* FALLTHRU */
        expr = fix_logical_and_or_expression(current_block, expr);
        break;
    case MINUS_EXPRESSION:
        expr = fix_minus_expression(current_block, expr);
        break;
    case LOGICAL_NOT_EXPRESSION:
        expr = fix_logical_not_expression(current_block, expr);
        break;
    case FUNCTION_CALL_EXPRESSION:
        expr = fix_function_call_expression(current_block, expr);
        break;
    case MEMBER_EXPRESSION:
        break;
    case NULL_EXPRESSION:
        expr->type = dkc_alloc_type_specifier(DVM_NULL_TYPE);
        break;
    case ARRAY_LITERAL_EXPRESSION:
        expr = fix_array_literal_expression(current_block, expr);
        break;
    case INDEX_EXPRESSION:
        expr = fix_index_expression(current_block, expr);
        break;
    case INCREMENT_EXPRESSION:  /* FALLTHRU */
    case DECREMENT_EXPRESSION:
        expr = fix_inc_dec_expression(current_block, expr);
        break;
    case CAST_EXPRESSION:
        break;
    case ARRAY_CREATION_EXPRESSION:
        expr = fix_array_creation_expression(current_block, expr);
        break;
    case EXPRESSION_KIND_COUNT_PLUS_1:
        break;
    default:
        DBG_assert(0, ("bad case. kind..%d\n", expr->kind));
    }

    return expr;
}

static void add_local_variable(FunctionDefinition *fd, Declaration *decl)
{
    fd->local_variable
        = MEM_realloc(fd->local_variable,
                      sizeof(Declaration*) * (fd->local_variable_count+1));
    fd->local_variable[fd->local_variable_count] = decl;
    decl->variable_index = fd->local_variable_count;
    fd->local_variable_count++;
}

static void fix_statement_list(Block *current_block, StatementList *list,
                               FunctionDefinition *fd);

static void
fix_if_statement(Block *current_block, IfStatement *if_s,
                 FunctionDefinition *fd)
{
    Elsif *pos;

    fix_expression(current_block, if_s->condition, NULL);
    fix_statement_list(if_s->then_block, if_s->then_block->statement_list,
                       fd);
    for (pos = if_s->elsif_list; pos; pos = pos->next) {
        fix_expression(current_block, pos->condition, NULL);
        if (pos->block) {
            fix_statement_list(pos->block, pos->block->statement_list,
                               fd);
        }
    }
    if (if_s->else_block) {
        fix_statement_list(if_s->else_block, if_s->else_block->statement_list,
                           fd);
    }
}

static void
fix_return_statement(Block *current_block, ReturnStatement *return_s,
                     FunctionDefinition *fd)
{
    Expression *return_value;
    Expression *casted_expression;

    return_value
        = fix_expression(current_block, return_s->return_value, NULL);

    if (return_value == NULL) {

        if (fd->type->derive) {
            if (fd->type->derive->tag == ARRAY_DERIVE) {
                return_value = dkc_alloc_expression(NULL_EXPRESSION);
            } else {
                DBG_assert(0, (("fd->type->derive..%d\n"), fd->type->derive));
            }
        } else {
            switch (fd->type->basic_type) {
            case DVM_BOOLEAN_TYPE:
                return_value = dkc_alloc_expression(BOOLEAN_EXPRESSION);
                return_value->u.boolean_value = DVM_FALSE;
                break;
            case DVM_INT_TYPE:
                return_value = dkc_alloc_expression(INT_EXPRESSION);
                return_value->u.int_value = 0;
                break;
            case DVM_DOUBLE_TYPE:
                return_value = dkc_alloc_expression(DOUBLE_EXPRESSION);
                return_value->u.int_value = 0.0;
                break;
            case DVM_STRING_TYPE:
                return_value = dkc_alloc_expression(NULL_EXPRESSION);
                return_value->u.string_value = L"";
                break;
            case DVM_NULL_TYPE: /* FALLTHRU */
            default:
                DBG_assert(0, ("basic_type..%d\n"));
                break;
            }
        }
        return_s->return_value = return_value;

        return;
    }
    casted_expression = create_assign_cast(return_s->return_value, fd->type);
    return_s->return_value = casted_expression;
}

static void
add_declaration(Block *current_block, Declaration *decl,
                FunctionDefinition *fd, int line_number)
{
    if (dkc_search_declaration(decl->name, current_block)) {
        dkc_compile_error(line_number,
                          VARIABLE_MULTIPLE_DEFINE_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", decl->name,
                          MESSAGE_ARGUMENT_END);
        }

    if (current_block) {
        current_block->declaration_list
            = dkc_chain_declaration(current_block->declaration_list, decl);
        add_local_variable(fd, decl);
        decl->is_local = DVM_TRUE;
    } else {
        DKC_Compiler *compiler = dkc_get_current_compiler();
        compiler->declaration_list
            = dkc_chain_declaration(compiler->declaration_list, decl);
        decl->is_local = DVM_FALSE;
    }
}

static void
fix_statement(Block *current_block, Statement *statement,
              FunctionDefinition *fd)
{
    switch (statement->type) {
    case EXPRESSION_STATEMENT:
        fix_expression(current_block, statement->u.expression_s, NULL);
        break;
    case IF_STATEMENT:
        fix_if_statement(current_block, &statement->u.if_s, fd);
        break;
    case WHILE_STATEMENT:
        fix_expression(current_block, statement->u.while_s.condition, NULL);
        fix_statement_list(statement->u.while_s.block,
                           statement->u.while_s.block->statement_list, fd);
        break;
    case FOR_STATEMENT:
        fix_expression(current_block, statement->u.for_s.init, NULL);
        fix_expression(current_block, statement->u.for_s.condition, NULL);
        fix_expression(current_block, statement->u.for_s.post, NULL);
        fix_statement_list(statement->u.for_s.block,
                           statement->u.for_s.block->statement_list, fd);
        break;
    case FOREACH_STATEMENT:
        fix_expression(current_block, statement->u.foreach_s.collection, NULL);
        fix_statement_list(statement->u.for_s.block,
                           statement->u.for_s.block->statement_list, fd);
        break;
    case RETURN_STATEMENT:
        fix_return_statement(current_block,
                             &statement->u.return_s, fd);
        break;
    case BREAK_STATEMENT:
        break;
    case CONTINUE_STATEMENT:
        break;
    case TRY_STATEMENT:
        /* BUGBUG */
        break;
    case THROW_STATEMENT:
        fix_expression(current_block, statement->u.throw_s.exception, NULL);
        /* BUGBUG */
        break;
    case DECLARATION_STATEMENT:
        add_declaration(current_block, statement->u.declaration_s, fd,
                        statement->line_number);
        fix_expression(current_block, statement->u.declaration_s->initializer,
                       NULL);
        if (statement->u.declaration_s->initializer) {
            statement->u.declaration_s->initializer
                = create_assign_cast(statement->u.declaration_s->initializer,
                                     statement->u.declaration_s->type);
        }
        break;
    case STATEMENT_TYPE_COUNT_PLUS_1: /* FALLTHRU */
    default:
        DBG_assert(0, ("bad case. type..%d\n", statement->type));
    }
}

static void
fix_statement_list(Block *current_block, StatementList *list,
                   FunctionDefinition *fd)
{
    StatementList *pos;

    for (pos = list; pos; pos = pos->next) {
        fix_statement(current_block, pos->statement,
                      fd);
    }
}

static void
add_parameter_as_declaration(FunctionDefinition *fd)
{
    Declaration *decl;
    ParameterList *param;

    for (param = fd->parameter; param; param = param->next) {
        if (dkc_search_declaration(param->name, fd->block)) {
            dkc_compile_error(param->line_number,
                              PARAMETER_MULTIPLE_DEFINE_ERR,
                              STRING_MESSAGE_ARGUMENT, "name", param->name,
                              MESSAGE_ARGUMENT_END);
        }
        decl = dkc_alloc_declaration(param->type, param->name);
        add_declaration(fd->block, decl, fd, param->line_number);
    }
}

static void
add_return_function(FunctionDefinition *fd)
{
    StatementList *last;
    StatementList **last_p;
    Statement *ret;

    if (fd->block->statement_list == NULL) {
        last_p = &fd->block->statement_list;
    } else {
        for (last = fd->block->statement_list; last->next; last = last->next)
            ;
        if (last->statement->type == RETURN_STATEMENT) {
            return;
        }
        last_p = &last->next;
    }

    ret = dkc_create_return_statement(NULL);
    ret->line_number = fd->end_line_number;
    if (ret->u.return_s.return_value) {
        ret->u.return_s.return_value->line_number = fd->end_line_number;
    }
    fix_return_statement(fd->block, &ret->u.return_s, fd);
    *last_p = dkc_create_statement_list(ret);
}

void
dkc_fix_tree(DKC_Compiler *compiler)
{
    FunctionDefinition *func_pos;
    DeclarationList *dl;
    int var_count = 0;

    fix_statement_list(NULL, compiler->statement_list, 0);
    
    for (func_pos = compiler->function_list; func_pos;
         func_pos = func_pos->next) {
        if (func_pos->block == NULL)
            continue;

        add_parameter_as_declaration(func_pos);
        fix_statement_list(func_pos->block,
                           func_pos->block->statement_list, func_pos);

        add_return_function(func_pos);
    }

    for (dl = compiler->declaration_list; dl; dl = dl->next) {
        dl->declaration->variable_index = var_count;
        var_count++;
    }
}
