#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "diksamc.h"

static int
reserve_function_index(DKC_Compiler *compiler, FunctionDefinition *src)
{
    int i;
    DVM_Function *dest;
    char *src_package_name;

    if (src->class_definition && src->block == NULL) {
        return ABSTRACT_METHOD_INDEX;
    }

    src_package_name = dkc_package_name_to_string(src->package_name);
    for (i = 0; i < compiler->dvm_function_count; i++) {
        if (dvm_compare_package_name(src_package_name,
                                     compiler->dvm_function[i].package_name)
            && !strcmp(src->name,
                       compiler->dvm_function[i].name)) {
            MEM_free(src_package_name);
            return i;
        }
    }
    compiler->dvm_function
        = MEM_realloc(compiler->dvm_function,
                      sizeof(DVM_Function) * (compiler->dvm_function_count+1));
    dest = &compiler->dvm_function[compiler->dvm_function_count];
    compiler->dvm_function_count++;

    dest->package_name = src_package_name;
    if (src->class_definition) {
        dest->name
            = dvm_create_method_function_name(src->class_definition->name,
                                              src->name);
    } else {
        dest->name = MEM_strdup(src->name);
    }

    return compiler->dvm_function_count - 1;
}

static ClassDefinition *
search_class_and_add(int line_number, char *name, int *class_index_p);

static int
add_class(ClassDefinition *src)
{
    int i;
    DVM_Class *dest;
    char *src_package_name;
    DKC_Compiler *compiler = dkc_get_current_compiler();
    ExtendsList *sup_pos;
    int ret;

    src_package_name = dkc_package_name_to_string(src->package_name);
    for (i = 0; i < compiler->dvm_class_count; i++) {
        if (dvm_compare_package_name(src_package_name,
                                     compiler->dvm_class[i].package_name)
            && !strcmp(src->name,
                       compiler->dvm_class[i].name)) {
            MEM_free(src_package_name);
            return i;
        }
    }
    compiler->dvm_class
        = MEM_realloc(compiler->dvm_class,
                      sizeof(DVM_Class) * (compiler->dvm_class_count+1));
    dest = &compiler->dvm_class[compiler->dvm_class_count];
    ret = compiler->dvm_class_count;
    compiler->dvm_class_count++;

    dest->package_name = src_package_name;
    dest->name = MEM_strdup(src->name);
    dest->is_implemented = DVM_FALSE;

    for (sup_pos = src->extends; sup_pos; sup_pos = sup_pos->next) {
        int dummy;
        search_class_and_add(src->line_number, sup_pos->identifier, &dummy);
    }

    return ret;
}

static DVM_Boolean
check_type_compatibility(TypeSpecifier *super_t, TypeSpecifier *sub_t);

static DVM_Boolean
is_super_interface(ClassDefinition *child, ClassDefinition *parent,
                   int *interface_index_out)
{
    ExtendsList *pos;
    int interface_index = 0;

    for (pos = child->interface_list; pos; pos = pos->next) {
        if (pos->class_definition == parent) {
            *interface_index_out = interface_index;
            return DVM_TRUE;
        }
        interface_index++;
    }
    return DVM_FALSE;
}

static DVM_Boolean
is_super_class(ClassDefinition *child, ClassDefinition *parent,
               DVM_Boolean *is_interface, int *interface_index)
{
    ClassDefinition *pos;

    for (pos = child->super_class; pos; pos = pos->super_class) {
        if (pos == parent) {
            *is_interface = DVM_FALSE;
            return DVM_TRUE;
        }
    }

    *is_interface = DVM_TRUE;
    return is_super_interface(child, parent, interface_index);
}

static void fix_type_specifier(TypeSpecifier *type);

static void
fix_parameter_list(ParameterList *parameter_list)
{
    ParameterList *param;

    for (param = parameter_list; param; param = param->next) {
        fix_type_specifier(param->type);
    }
}

static void
fix_type_specifier(TypeSpecifier *type)
{
    ClassDefinition *cd;
    TypeDerive *derive_pos;
    DKC_Compiler *compiler = dkc_get_current_compiler();

    /* BUGBUG for delegate? */
    for (derive_pos = type->derive; derive_pos;
         derive_pos = derive_pos->next) {
        if (derive_pos->tag == FUNCTION_DERIVE) {
            fix_parameter_list(derive_pos->u.function_d.parameter_list);
        }
    }

    if (type->basic_type == DVM_CLASS_TYPE
        && type->class_ref.class_definition == NULL) {

        cd = dkc_search_class(type->class_ref.identifier);
        if (cd) {
            if (!dkc_compare_package_name(cd->package_name,
                                          compiler->package_name)
                && cd->access_modifier != DVM_PUBLIC_ACCESS) {
                dkc_compile_error(type->line_number,
                                  PACKAGE_CLASS_ACCESS_ERR,
                                  STRING_MESSAGE_ARGUMENT, "class_name",
                                  cd->name,
                                  MESSAGE_ARGUMENT_END);
            }
            type->class_ref.class_definition = cd;
            type->class_ref.class_index = add_class(cd);
            return;
        }
        dkc_compile_error(type->line_number,
                          TYPE_NAME_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "name",
                          type->class_ref.identifier,
                          MESSAGE_ARGUMENT_END);
    }
}

static TypeSpecifier *
create_function_derive_type(FunctionDefinition *fd)
{
    TypeSpecifier *ret;

    ret = dkc_alloc_type_specifier(fd->type->basic_type);
    *ret = *fd->type;
    ret->derive = dkc_alloc_type_derive(FUNCTION_DERIVE);
    ret->derive->u.function_d.parameter_list = fd->parameter;
    ret->derive->next = fd->type->derive;

    return ret;
}

static Expression *
fix_identifier_expression(Block *current_block, Expression *expr)
{
    Declaration *decl;
    FunctionDefinition *fd;
    DKC_Compiler *compiler = dkc_get_current_compiler();

    decl = dkc_search_declaration(expr->u.identifier.name, current_block);
    if (decl) {
        expr->type = decl->type;
        expr->u.identifier.kind = VARIABLE_IDENTIFIER;
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
    /* expr->type is not used. function_call_expression's type
     * is gotten from FunctionDefinition in fix_function_call_expression().
     */
    expr->type = create_function_derive_type(fd);
    expr->u.identifier.kind = FUNCTION_IDENTIFIER;
    expr->u.identifier.u.function.function_definition = fd;
    expr->u.identifier.u.function.function_index
        = reserve_function_index(compiler, fd);
    fix_type_specifier(expr->type);

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
    } else {
        DBG_panic(("cast_type..%d\n", cast_type));
    }

    return cast_expr;
}

static Expression *
create_up_cast(Expression *src,
               ClassDefinition *dest_interface, int interface_index)
{
    TypeSpecifier *type;
    Expression *cast_expr;
    
    type = dkc_alloc_type_specifier(DVM_CLASS_TYPE);
    type->class_ref.identifier = dest_interface->name;
    type->class_ref.class_definition = dest_interface;
    type->class_ref.class_index = interface_index;

    cast_expr = dkc_alloc_expression(UP_CAST_EXPRESSION);
    cast_expr->type = type;
    cast_expr->u.up_cast.interface_definition = dest_interface;
    cast_expr->u.up_cast.operand = src;
    cast_expr->u.up_cast.interface_index = interface_index;

    return cast_expr;
}

static DVM_Boolean
check_type_compatibility(TypeSpecifier *super_t, TypeSpecifier *sub_t)
{
    DVM_Boolean is_interface_dummy;
    int interface_index_dummy;

    if (!dkc_is_class_object(super_t)) {
        return dkc_compare_type(super_t, sub_t);
    }
    if (!dkc_is_class_object(sub_t)) {
        return DVM_FALSE;
    }

    if (super_t->class_ref.class_definition
        == sub_t->class_ref.class_definition
        || is_super_class(sub_t->class_ref.class_definition,
                          super_t->class_ref.class_definition,
                          &is_interface_dummy,
                          &interface_index_dummy)) {
        return DVM_TRUE;
    }
    return DVM_FALSE;
}

static void
check_func_compati_sub(int line_number, char *name,
                       TypeSpecifier *type1, ParameterList *param1,
                       TypeSpecifier *type2, ParameterList *param2)
{
    ParameterList *param1_pos;
    ParameterList *param2_pos;
    int param_idx = 1;

    for (param1_pos = param1, param2_pos = param2;
         param1_pos != NULL && param2_pos != NULL;
         param1_pos = param1_pos->next, param2_pos = param2_pos->next) {
        if (!check_type_compatibility(param2_pos->type, param1_pos->type)) {
            dkc_compile_error(line_number,
                              BAD_PARAMETER_TYPE_ERR,
                              STRING_MESSAGE_ARGUMENT, "func_name", name,
                              INT_MESSAGE_ARGUMENT, "index", param_idx,
                              STRING_MESSAGE_ARGUMENT, "param_name",
                              param2_pos->name,
                              MESSAGE_ARGUMENT_END);
        }
        param_idx++;
    }

    if (param1_pos != NULL || param2_pos != NULL) {
        dkc_compile_error(line_number,
                          BAD_PARAMETER_COUNT_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", name,
                          MESSAGE_ARGUMENT_END);
    }

    if (!check_type_compatibility(type1, type2)) {
        dkc_compile_error(line_number,
                          BAD_RETURN_TYPE_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", name,
                          MESSAGE_ARGUMENT_END);
    }
}

static void
check_function_compatibility(FunctionDefinition *fd1, FunctionDefinition *fd2)
{
    check_func_compati_sub(fd2->end_line_number, fd2->name,
                           fd1->type, fd1->parameter,
                           fd2->type, fd2->parameter);
}

static Expression *
create_to_string_cast(Expression *src)
{
    Expression *cast = NULL;

    if (dkc_is_boolean(src->type)) {
        cast = alloc_cast_expression(BOOLEAN_TO_STRING_CAST, src);

    } else if (dkc_is_int(src->type)) {
        cast = alloc_cast_expression(INT_TO_STRING_CAST, src);

    } else if (dkc_is_double(src->type)) {
        cast = alloc_cast_expression(DOUBLE_TO_STRING_CAST, src);
    }

    return cast;
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

    if (dkc_is_class_object(src->type) && dkc_is_class_object(dest)) {
        DVM_Boolean is_interface;
        int interface_index;
        if (is_super_class(src->type->class_ref.class_definition,
                           dest->class_ref.class_definition,
                           &is_interface, &interface_index)) {
            if (is_interface) {
                cast_expr
                    = create_up_cast(src, dest->class_ref.class_definition,
                                     interface_index);
                return cast_expr;
            }
            return src;
        } else {
            cast_mismatch_error(src->line_number, src->type, dest);
        }
    }
    if (dkc_is_int(src->type) && dkc_is_double(dest)) {
        cast_expr = alloc_cast_expression(INT_TO_DOUBLE_CAST, src);
        return cast_expr;
    } else if (dkc_is_double(src->type) && dkc_is_int(dest)) {
        cast_expr = alloc_cast_expression(DOUBLE_TO_INT_CAST, src);
        return cast_expr;
    } else if (dkc_is_string(dest)) {
        cast_expr = create_to_string_cast(src);
        if (cast_expr) {
            return cast_expr;
        }
    }

    cast_mismatch_error(src->line_number, src->type, dest);

    return NULL; /* make compiler happy. */
}

static Expression *
fix_assign_expression(Block *current_block, Expression *expr)
{
    Expression *left;
    Expression *operand;

    if (expr->u.assign_expression.left->kind != IDENTIFIER_EXPRESSION
        && expr->u.assign_expression.left->kind != INDEX_EXPRESSION
        && expr->u.assign_expression.left->kind != MEMBER_EXPRESSION) {
        dkc_compile_error(expr->u.assign_expression.left->line_number,
                          NOT_LVALUE_ERR, MESSAGE_ARGUMENT_END);
    }

    expr->u.assign_expression.left
        = fix_expression(current_block, expr->u.assign_expression.left,
                         expr);

    left = expr->u.assign_expression.left;

    operand = fix_expression(current_block, expr->u.assign_expression.operand,
                             expr);
    expr->u.assign_expression.operand
        = create_assign_cast(operand, expr->u.assign_expression.left->type);
    expr->type = left->type;

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
    Expression *cast_expression = NULL;

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
    }
    if (cast_expression) {
        return expr;
    }

    if (dkc_is_string(expr->u.binary_expression.left->type)
        && expr->kind == ADD_EXPRESSION) {

        cast_expression
            = create_to_string_cast(expr->u.binary_expression.right);
        if (cast_expression) {
            expr->u.binary_expression.right = cast_expression;
        }
    }

    return expr;
}

static Expression *
fix_math_binary_expression(Block *current_block, Expression *expr)
{
    expr->u.binary_expression.left
        = fix_expression(current_block, expr->u.binary_expression.left, expr);
    expr->u.binary_expression.right
        = fix_expression(current_block, expr->u.binary_expression.right, expr);

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
        = fix_expression(current_block, expr->u.binary_expression.left, expr);
    expr->u.binary_expression.right
        = fix_expression(current_block, expr->u.binary_expression.right, expr);

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
        = fix_expression(current_block, expr->u.binary_expression.left, expr);
    expr->u.binary_expression.right
        = fix_expression(current_block, expr->u.binary_expression.right, expr);
    
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
        expr->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);
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
check_argument(Block *current_block, int line_number,
               ParameterList *param_list, ArgumentList *arg,
               TypeSpecifier *array_base)
{
    ParameterList *param;
    TypeSpecifier *temp_type;

    for (param = param_list;
         param && arg;
         param = param->next, arg = arg->next) {
        arg->expression
            = fix_expression(current_block, arg->expression, NULL);

        if (param->type->basic_type == DVM_BASE_TYPE) {
            DBG_assert(array_base != NULL, ("array_base == NULL\n"));
            temp_type = array_base;
        } else {
            temp_type = param->type;
        }
        arg->expression
            = create_assign_cast(arg->expression, temp_type);
    }

    if (param || arg) {
        dkc_compile_error(line_number,
                          ARGUMENT_COUNT_MISMATCH_ERR,
                          MESSAGE_ARGUMENT_END);
    }
}

static ClassDefinition *
search_class_and_add(int line_number, char *name, int *class_index_p)
{
    ClassDefinition *cd;

    cd = dkc_search_class(name);
    if (cd == NULL) {
        dkc_compile_error(line_number,
                          CLASS_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", name,
                          MESSAGE_ARGUMENT_END);
    }
    *class_index_p = add_class(cd);

    return cd;
}

static Expression *
fix_function_call_expression(Block *current_block, Expression *expr)
{
    Expression *func_expr;
    FunctionDefinition *fd = NULL;
    DKC_Compiler *compiler = dkc_get_current_compiler();
    TypeSpecifier *array_base_p = NULL;
    TypeSpecifier array_base;
    TypeSpecifier *func_type;
    ParameterList *func_param;

    func_expr
        = fix_expression(current_block,
                         expr->u.function_call_expression.function, expr);
    expr->u.function_call_expression.function = func_expr;

    if (func_expr->kind == IDENTIFIER_EXPRESSION) {
        fd = dkc_search_function(func_expr->u.identifier.name);

    } else if (func_expr->kind == MEMBER_EXPRESSION) {
        if (dkc_is_array(func_expr->u.member_expression.expression->type)) {
            fd = &compiler->array_method[func_expr->u.member_expression
                                         .method_index];
            array_base = *func_expr->u.member_expression.expression->type;
            array_base.derive = func_expr->u.member_expression.expression
                ->type->derive->next;
            array_base_p = &array_base;
        } else if (dkc_is_string(func_expr->u.member_expression.expression
                                 ->type)) {
            fd = &compiler->string_method[func_expr->u.member_expression
                                          .method_index];
        } else {
            if (func_expr->u.member_expression.declaration->kind
                == FIELD_MEMBER) {
                dkc_compile_error(expr->line_number, FIELD_CAN_NOT_CALL_ERR,
                                  STRING_MESSAGE_ARGUMENT, "member_name",
                                  func_expr->u.member_expression.declaration
                                  ->u.field.name,
                                  MESSAGE_ARGUMENT_END);
            } 
            if (func_expr->u.member_expression.declaration
                ->u.method.is_constructor) {
                Expression *obj = func_expr->u.member_expression.expression;
                if (obj->kind != SUPER_EXPRESSION
                    && obj->kind != THIS_EXPRESSION) {
                    dkc_compile_error(expr->line_number,
                                      CONSTRUCTOR_CALLED_ERR,
                                      STRING_MESSAGE_ARGUMENT, "member_name",
                                      func_expr->u.member_expression
                                      .declaration->u.field.name,
                                      MESSAGE_ARGUMENT_END);
                }
            }
            fd = func_expr->u.member_expression.declaration
                ->u.method.function_definition;
        }
    }
    if (fd == NULL) {
        dkc_compile_error(expr->line_number,
                          FUNCTION_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "name",
                          func_expr->u.identifier.name,
                          MESSAGE_ARGUMENT_END);
    }
    func_type = fd->type;
    func_param = fd->parameter;

    check_argument(current_block, expr->line_number,
                   func_param, expr->u.function_call_expression.argument,
                   array_base_p);
    expr->type = dkc_alloc_type_specifier(func_type->basic_type);
    *expr->type = *func_type;
    expr->type->derive = func_type->derive;
    if (func_type->basic_type == DVM_CLASS_TYPE) {
        expr->type->class_ref.identifier = func_type->class_ref.identifier;
        fix_type_specifier(expr->type);
    }

    return expr;
}

static DVM_Boolean
is_interface_method(ClassDefinition *cd, MemberDeclaration *member,
                    ClassDefinition **target_interface,
                    int *interface_index_out)
{
    ExtendsList *e_pos;
    MemberDeclaration *m_pos;
    int interface_index;
    
    for (e_pos = cd->interface_list, interface_index = 0;
         e_pos; e_pos = e_pos->next, interface_index++) {
        for (m_pos = e_pos->class_definition->member; m_pos;
             m_pos = m_pos->next) {
            if (m_pos->kind != METHOD_MEMBER)
                continue;

            if (!strcmp(member->u.method.function_definition->name,
                        m_pos->u.method.function_definition->name)) {
                *target_interface = e_pos->class_definition;
                *interface_index_out = interface_index;
                return DVM_TRUE;
            }
        }
    }
    return DVM_FALSE;
}

static void
check_member_accessibility(int line_number,
                           ClassDefinition *target_class,
                           MemberDeclaration *member,
                           char *member_name)
{
    DKC_Compiler *compiler = dkc_get_current_compiler();

    if (compiler->current_class_definition == NULL
        || compiler->current_class_definition != target_class) {
        if (member->access_modifier == DVM_PRIVATE_ACCESS) {
            dkc_compile_error(line_number,
                              PRIVATE_MEMBER_ACCESS_ERR,
                              STRING_MESSAGE_ARGUMENT, "member_name",
                              member_name,
                              MESSAGE_ARGUMENT_END);
        }
    }
    if (!dkc_compare_package_name(compiler->package_name,
                                  target_class->package_name)
        && member->access_modifier != DVM_PUBLIC_ACCESS) {
        dkc_compile_error(line_number,
                          PACKAGE_MEMBER_ACCESS_ERR,
                          STRING_MESSAGE_ARGUMENT, "member_name",
                          member_name,
                          MESSAGE_ARGUMENT_END);
    }
}

static Expression *
fix_class_member_expression(Expression *expr,
                            Expression *obj, char *member_name)
{
    MemberDeclaration *member;
    ClassDefinition *target_interface;
    int interface_index;

    fix_type_specifier(obj->type);
    member = dkc_search_member(obj->type->class_ref.class_definition,
                               member_name);
    if (member == NULL) {
        dkc_compile_error(expr->line_number,
                          MEMBER_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "class_name",
                          obj->type->class_ref.class_definition->name,
                          STRING_MESSAGE_ARGUMENT, "member_name", member_name,
                          MESSAGE_ARGUMENT_END);
    }
    check_member_accessibility(obj->line_number,
                               obj->type->class_ref.class_definition,
                               member, member_name);

    expr->u.member_expression.declaration = member;
    if (member->kind == METHOD_MEMBER) {
        expr->type
            = create_function_derive_type(member
                                          ->u.method.function_definition);

        if (obj->type->class_ref.class_definition->class_or_interface
            == DVM_CLASS_DEFINITION
            && is_interface_method(obj->type->class_ref.class_definition,
                                   member,
                                   &target_interface, &interface_index)) {
            expr->u.member_expression.expression
                = create_up_cast(obj, target_interface, interface_index);
        }
    } else if (member->kind == FIELD_MEMBER) {
        if (obj->kind == SUPER_EXPRESSION) {
            dkc_compile_error(expr->line_number,
                              FIELD_OF_SUPER_REFERENCED_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        expr->type = member->u.field.type;
    }

    return expr;
}

static Expression *
fix_array_method_expression(Expression *expr,
                            Expression *obj, char *member_name)
{
    DKC_Compiler *compiler = dkc_get_current_compiler();
    FunctionDefinition *fd;
    int i;
    
    for (i = 0; i < compiler->array_method_count; i++) {
        if (!strcmp(compiler->array_method[i].name, member_name))
            break;
    }
    if (i == compiler->array_method_count) {
        dkc_compile_error(expr->line_number,
                          ARRAY_METHOD_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", member_name,
                          MESSAGE_ARGUMENT_END);
    }
    fd = &compiler->array_method[i];
    expr->u.member_expression.method_index = i;
    expr->type = create_function_derive_type(fd);

    return expr;
}

static Expression *
fix_string_method_expression(Expression *expr,
                             Expression *obj, char *member_name)
{
    DKC_Compiler *compiler = dkc_get_current_compiler();
    FunctionDefinition *fd;
    int i;
    
    for (i = 0; i < compiler->string_method_count; i++) {
        if (!strcmp(compiler->string_method[i].name, member_name))
            break;
    }
    if (i == compiler->string_method_count) {
        dkc_compile_error(expr->line_number,
                          STRING_METHOD_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", member_name,
                          MESSAGE_ARGUMENT_END);
    }
    fd = &compiler->string_method[i];
    expr->u.member_expression.method_index = i;
    expr->type = create_function_derive_type(fd);

    return expr;
}

static Expression *
fix_member_expression(Block *current_block, Expression *expr)
{
    Expression *obj;

    obj = expr->u.member_expression.expression
        = fix_expression(current_block, expr->u.member_expression.expression,
                         expr);
    if (dkc_is_class_object(obj->type)) {
        return fix_class_member_expression(expr, obj,
                                           expr->u.member_expression
                                           .member_name);
    } else if (dkc_is_array(obj->type)) {
        return
            fix_array_method_expression(expr, obj,
                                        expr->u.member_expression.member_name);
    } else if (dkc_is_string(obj->type)) {
        return
            fix_string_method_expression(expr, obj,
                                         expr
                                         ->u.member_expression.member_name);
    } else {
        dkc_compile_error(expr->line_number,
                          MEMBER_EXPRESSION_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    return NULL; /* make compiler happy */
}

static Expression *
fix_this_expression(Expression *expr)
{
    TypeSpecifier *type;
    ClassDefinition *cd;

    cd = dkc_get_current_compiler()->current_class_definition;
    if (cd == NULL) {
        dkc_compile_error(expr->line_number,
                          THIS_OUT_OF_CLASS_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    type = dkc_alloc_type_specifier(DVM_CLASS_TYPE);
    type->class_ref.identifier = cd->name;
    type->class_ref.class_definition = cd;
    expr->type = type;

    return expr;
}

static Expression *
fix_super_expression(Expression *expr, Expression *parent)
{
    TypeSpecifier *type;
    ClassDefinition *cd;

    cd = dkc_get_current_compiler()->current_class_definition;
    if (cd == NULL) {
        dkc_compile_error(expr->line_number,
                          SUPER_OUT_OF_CLASS_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    if (cd->super_class == NULL) {
        dkc_compile_error(expr->line_number,
                          HASNT_SUPER_CLASS_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (parent == NULL || parent->kind != MEMBER_EXPRESSION) {
        dkc_compile_error(parent->line_number,
                          SUPER_NOT_IN_MEMBER_EXPRESSION_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    type = dkc_alloc_type_specifier(DVM_CLASS_TYPE);
    type->class_ref.identifier = cd->super_class->name;
    type->class_ref.class_definition = cd->super_class;
    expr->type = type;

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
    *expr->type = *elem_type;
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

    if (ie->array->type->derive != NULL
        && ie->array->type->derive->tag == ARRAY_DERIVE) {
        expr->type = dkc_alloc_type_specifier2(ie->array->type);
        expr->type->derive = ie->array->type->derive->next;
    } else {
        dkc_compile_error(expr->line_number,
                          INDEX_LEFT_OPERAND_NOT_ARRAY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (!dkc_is_int(ie->index->type)) {
        dkc_compile_error(expr->line_number,
                          INDEX_NOT_INT_ERR,
                          MESSAGE_ARGUMENT_END);
    }

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
fix_instanceof_expression(Block *current_block, Expression *expr)
{
    DVM_Boolean is_interface_dummy;
    int interface_index_dummy;
    Expression *operand;
    TypeSpecifier *target;

    expr->u.instanceof.operand
        = fix_expression(current_block, expr->u.instanceof.operand, expr);
    fix_type_specifier(expr->u.instanceof.type);

    operand = expr->u.instanceof.operand;
    target = expr->u.instanceof.type;

    if (!dkc_is_object(operand->type)) {
        dkc_compile_error(expr->line_number,
                          INSTANCEOF_OPERAND_NOT_REFERENCE_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    if (!dkc_is_object(target)) {
        dkc_compile_error(expr->line_number,
                          INSTANCEOF_TYPE_NOT_REFERENCE_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    if (!dkc_is_class_object(operand->type) || !dkc_is_class_object(target)) {
        dkc_compile_error(expr->line_number,
                          INSTANCEOF_FOR_NOT_CLASS_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    if (dkc_compare_type(operand->type, target)) {
        dkc_compile_error(expr->line_number,
                          INSTANCEOF_MUST_RETURN_TRUE_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    if (is_super_class(operand->type->class_ref.class_definition,
                       target->class_ref.class_definition,
                       &is_interface_dummy, &interface_index_dummy)) {
        dkc_compile_error(expr->line_number,
                          INSTANCEOF_MUST_RETURN_TRUE_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    if (target->class_ref.class_definition->class_or_interface
        == DVM_CLASS_DEFINITION
        && !is_super_class(target->class_ref.class_definition,
                           operand->type->class_ref.class_definition,
                           &is_interface_dummy, &interface_index_dummy)) {
        dkc_compile_error(expr->line_number,
                          INSTANCEOF_MUST_RETURN_FALSE_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    expr->type = dkc_alloc_type_specifier(DVM_BOOLEAN_TYPE);

    return expr;
}

static Expression *
fix_down_cast_expression(Block *current_block, Expression *expr)
{
    DVM_Boolean is_interface_dummy;
    int interface_index_dummy;
    TypeSpecifier *org_type;
    TypeSpecifier *target_type;

    expr->u.down_cast.operand
        = fix_expression(current_block, expr->u.down_cast.operand, expr);
    fix_type_specifier(expr->u.down_cast.type);
    org_type = expr->u.down_cast.operand->type;
    target_type = expr->u.down_cast.type;

    if (!dkc_is_class_object(org_type)) {
        dkc_compile_error(expr->line_number,
                          DOWN_CAST_OPERAND_IS_NOT_CLASS_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (!dkc_is_class_object(target_type)) {
        dkc_compile_error(expr->line_number,
                          DOWN_CAST_TARGET_IS_NOT_CLASS_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (dkc_compare_type(org_type, target_type)) {
        dkc_compile_error(expr->line_number,
                          DOWN_CAST_DO_NOTHING_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (target_type->class_ref.class_definition->class_or_interface
        == DVM_CLASS_DEFINITION
        && !is_super_class(target_type->class_ref.class_definition,
                           org_type->class_ref.class_definition,
                           &is_interface_dummy, &interface_index_dummy)) {
        dkc_compile_error(expr->line_number,
                          DOWN_CAST_TO_BAD_CLASS_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    
    expr->type = target_type;

    return expr;
}

static Expression *
fix_new_expression(Block *current_block, Expression *expr)
{
    MemberDeclaration *member;
    TypeSpecifier *type;

    expr->u.new_e.class_definition
        = search_class_and_add(expr->line_number,
                               expr->u.new_e.class_name,
                               &expr->u.new_e.class_index);
    if (expr->u.new_e.class_definition->is_abstract) {
        dkc_compile_error(expr->line_number,
                          NEW_ABSTRACT_CLASS_ERR,
                          STRING_MESSAGE_ARGUMENT, "name",
                          expr->u.new_e.class_name,
                          MESSAGE_ARGUMENT_END);
    }
    if (!expr->u.new_e.method_name) {
        expr->u.new_e.method_name = DEFAULT_CONSTRUCTOR_NAME;
    }
    member = dkc_search_member(expr->u.new_e.class_definition,
                               expr->u.new_e.method_name);
    if (member == NULL) {
        dkc_compile_error(expr->line_number,
                          MEMBER_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "class_name",
                          expr->u.new_e.class_name,
                          STRING_MESSAGE_ARGUMENT, "member_name",
                          expr->u.new_e.method_name,
                          MESSAGE_ARGUMENT_END);
    }
    if (member->kind != METHOD_MEMBER) {
        dkc_compile_error(expr->line_number,
                          CONSTRUCTOR_IS_FIELD_ERR,
                          STRING_MESSAGE_ARGUMENT, "member_name",
                          expr->u.new_e.method_name,
                          MESSAGE_ARGUMENT_END);
    }
    if (!member->u.method.is_constructor) {
        dkc_compile_error(expr->line_number,
                          NOT_CONSTRUCTOR_ERR,
                          STRING_MESSAGE_ARGUMENT, "member_name",
                          expr->u.new_e.method_name,
                          MESSAGE_ARGUMENT_END);
    }
    check_member_accessibility(expr->line_number,
                               expr->u.new_e.class_definition,
                               member,
                               expr->u.new_e.method_name);
    DBG_assert(member->u.method.function_definition->type->derive == NULL
               && member->u.method.function_definition->type->basic_type
               == DVM_VOID_TYPE,
               ("constructor is not void.\n"));

    check_argument(current_block, expr->line_number,
                   member->u.method.function_definition->parameter,
                   expr->u.new_e.argument, NULL);

    expr->u.new_e.method_declaration = member;
    type = dkc_alloc_type_specifier(DVM_CLASS_TYPE);
    type->class_ref.identifier = expr->u.new_e.class_definition->name;
    type->class_ref.class_definition = expr->u.new_e.class_definition;
    expr->type = type;

    return expr;
}

static Expression *
fix_array_creation_expression(Block *current_block, Expression *expr)
{
    ArrayDimension *dim_pos;
    TypeDerive *derive = NULL;
    TypeDerive *tmp_derive;

    fix_type_specifier(expr->u.array_creation.type);

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

    expr->type = dkc_alloc_type_specifier2(expr->u.array_creation.type);
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
        expr = fix_identifier_expression(current_block, expr);
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
    case MOD_EXPRESSION:
        expr = fix_math_binary_expression(current_block, expr);
        break;
    case EQ_EXPRESSION: /* FALLTHRU */
    case NE_EXPRESSION: /* FALLTHRU */
    case GT_EXPRESSION: /* FALLTHRU */
    case GE_EXPRESSION: /* FALLTHRU */
    case LT_EXPRESSION: /* FALLTHRU */
    case LE_EXPRESSION:
        expr = fix_compare_expression(current_block, expr);
        break;
    case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
    case LOGICAL_OR_EXPRESSION:
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
        expr = fix_member_expression(current_block, expr);
        break;
    case NULL_EXPRESSION:
        expr->type = dkc_alloc_type_specifier(DVM_NULL_TYPE);
        break;
    case THIS_EXPRESSION:
        expr = fix_this_expression(expr);
        break;
    case SUPER_EXPRESSION:
        expr = fix_super_expression(expr, parent);
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
    case INSTANCEOF_EXPRESSION:
        expr = fix_instanceof_expression(current_block, expr);
        break;
    case DOWN_CAST_EXPRESSION:
        expr = fix_down_cast_expression(current_block, expr);
        break;
    case CAST_EXPRESSION:
        break;
    case UP_CAST_EXPRESSION:
        break;
    case NEW_EXPRESSION:
        expr = fix_new_expression(current_block, expr);
        break;
    case ARRAY_CREATION_EXPRESSION:
        expr = fix_array_creation_expression(current_block, expr);
        break;
    case EXPRESSION_KIND_COUNT_PLUS_1:
        break;
    default:
        DBG_assert(0, ("bad case. kind..%d\n", expr->kind));
    }
    fix_type_specifier(expr->type);

    return expr;
}

static void add_local_variable(FunctionDefinition *fd, Declaration *decl,
                               DVM_Boolean is_parameter)
{
    fd->local_variable
        = MEM_realloc(fd->local_variable,
                      sizeof(Declaration*) * (fd->local_variable_count+1));
    fd->local_variable[fd->local_variable_count] = decl;
    if (fd->class_definition && !is_parameter) {
        decl->variable_index = fd->local_variable_count + 1; /* for this */
    } else {
        decl->variable_index = fd->local_variable_count;
    }

    fd->local_variable_count++;
}

static void fix_statement_list(Block *current_block, StatementList *list,
                               FunctionDefinition *fd);

static void
fix_if_statement(Block *current_block, IfStatement *if_s,
                 FunctionDefinition *fd)
{
    Elsif *pos;

    if_s->condition
        = fix_expression(current_block, if_s->condition, NULL);
    if (!dkc_is_boolean(if_s->condition->type)) {
        dkc_compile_error(if_s->condition->line_number,
                          IF_CONDITION_NOT_BOOLEAN_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    fix_statement_list(if_s->then_block, if_s->then_block->statement_list,
                       fd);
    for (pos = if_s->elsif_list; pos; pos = pos->next) {
        pos->condition
            = fix_expression(current_block, pos->condition, NULL);
        if (pos->block) {
            fix_statement_list(pos->block, pos->block->statement_list, fd);
        }
    }
    if (if_s->else_block) {
        fix_statement_list(if_s->else_block, if_s->else_block->statement_list,
                           fd);
    }
}

static void
fix_while_statement(Block *current_block, WhileStatement *while_s,
                    FunctionDefinition *fd)
{
    while_s->condition
        = fix_expression(current_block, while_s->condition, NULL);
    if (!dkc_is_boolean(while_s->condition->type)) {
        dkc_compile_error(while_s->condition->line_number,
                          WHILE_CONDITION_NOT_BOOLEAN_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    fix_statement_list(while_s->block, while_s->block->statement_list, fd);
}

static void
fix_for_statement(Block *current_block, ForStatement *for_s,
                  FunctionDefinition *fd)
{
    for_s->init = fix_expression(current_block, for_s->init, NULL);
    for_s->condition = fix_expression(current_block, for_s->condition, NULL);
    if (for_s->condition != NULL
        && !dkc_is_boolean(for_s->condition->type)) {
        dkc_compile_error(for_s->condition->line_number,
                          FOR_CONDITION_NOT_BOOLEAN_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    for_s->post = fix_expression(current_block, for_s->post, NULL);
    fix_statement_list(for_s->block,
                       for_s->block->statement_list, fd);
}

static void
fix_do_while_statement(Block *current_block, DoWhileStatement *do_while_s,
                       FunctionDefinition *fd)
{
    do_while_s->condition
        = fix_expression(current_block, do_while_s->condition, NULL);
    if (!dkc_is_boolean(do_while_s->condition->type)) {
        dkc_compile_error(do_while_s->condition->line_number,
                          DO_WHILE_CONDITION_NOT_BOOLEAN_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    fix_statement_list(do_while_s->block, do_while_s->block->statement_list,
                       fd);
}

static void
fix_return_statement(Block *current_block, Statement *statement,
                     FunctionDefinition *fd)
{
    Expression *return_value;
    Expression *casted_expression;

    return_value
        = fix_expression(current_block,
                         statement->u.return_s.return_value, NULL);
    if (fd->type->derive == NULL && fd->type->basic_type == DVM_VOID_TYPE
        && return_value != NULL) {
        dkc_compile_error(statement->line_number,
                          RETURN_IN_VOID_FUNCTION_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    if (return_value == NULL) {
        if (fd->type->derive) {
            if (fd->type->derive->tag == ARRAY_DERIVE) {
                return_value = dkc_alloc_expression(NULL_EXPRESSION);
            } else {
                DBG_assert(0, (("fd->type->derive..%d\n"), fd->type->derive));
            }
        } else {
            switch (fd->type->basic_type) {
            case DVM_VOID_TYPE:
                return_value = dkc_alloc_expression(INT_EXPRESSION);
                return_value->u.int_value = 0;
                break;
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
                return_value->u.double_value = 0.0;
                break;
            case DVM_STRING_TYPE: /* FALLTHRU */
            case DVM_CLASS_TYPE:
                return_value = dkc_alloc_expression(NULL_EXPRESSION);
                break;
            case DVM_NULL_TYPE: /* FALLTHRU */
            case DVM_BASE_TYPE: /* FALLTHRU */
            default:
                DBG_assert(0, ("basic_type..%d\n"));
                break;
            }
        }
        statement->u.return_s.return_value = return_value;

        return;
    }
    casted_expression
        = create_assign_cast(statement->u.return_s.return_value, fd->type);
    statement->u.return_s.return_value = casted_expression;
}

static void
add_declaration(Block *current_block, Declaration *decl,
                FunctionDefinition *fd, int line_number,
                DVM_Boolean is_parameter)
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
    }
    if (fd) {
        decl->is_local = DVM_TRUE;
        add_local_variable(fd, decl, is_parameter);
    } else {
        DKC_Compiler *compiler = dkc_get_current_compiler();
        decl->is_local = DVM_FALSE;
        compiler->declaration_list
            = dkc_chain_declaration(compiler->declaration_list, decl);
    }
}

static void
fix_statement(Block *current_block, Statement *statement,
              FunctionDefinition *fd)
{
    switch (statement->type) {
    case EXPRESSION_STATEMENT:
        statement->u.expression_s
            = fix_expression(current_block, statement->u.expression_s,
                             NULL);
        break;
    case IF_STATEMENT:
        fix_if_statement(current_block, &statement->u.if_s, fd);
        break;
    case WHILE_STATEMENT:
        fix_while_statement(current_block, &statement->u.while_s, fd);
        break;
    case FOR_STATEMENT:
        fix_for_statement(current_block, &statement->u.for_s, fd);
        break;
    case DO_WHILE_STATEMENT:
        fix_do_while_statement(current_block, &statement->u.do_while_s, fd);
        break;
    case FOREACH_STATEMENT:
        statement->u.foreach_s.collection
            = fix_expression(current_block, statement->u.foreach_s.collection,
                             NULL);
        fix_statement_list(statement->u.for_s.block,
                           statement->u.for_s.block->statement_list, fd);
        break;
    case RETURN_STATEMENT:
        fix_return_statement(current_block, statement, fd);
        break;
    case BREAK_STATEMENT:
        break;
    case CONTINUE_STATEMENT:
        break;
    case DECLARATION_STATEMENT:
        add_declaration(current_block, statement->u.declaration_s, fd,
                        statement->line_number, DVM_FALSE);
        fix_type_specifier(statement->u.declaration_s->type);
        if (statement->u.declaration_s->initializer) {
            statement->u.declaration_s->initializer
                = fix_expression(current_block,
                                 statement->u.declaration_s->initializer,
                                 NULL);
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
        fix_statement(current_block, pos->statement, fd);
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
        fix_type_specifier(param->type);
        decl = dkc_alloc_declaration(param->type, param->name);
        if (fd == NULL || fd->block) {
            add_declaration(fd->block, decl, fd, param->line_number,
                            DVM_TRUE);
        }
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
    fix_return_statement(fd->block, ret, fd);
    *last_p = dkc_create_statement_list(ret);
}

static void
fix_function(FunctionDefinition *fd)
{
    add_parameter_as_declaration(fd);
    fix_type_specifier(fd->type);
    if (fd->block) {
        fix_statement_list(fd->block, fd->block->statement_list, fd);
        add_return_function(fd);
    }
}

static void
add_super_interfaces(ClassDefinition *cd)
{
    ClassDefinition *super_pos;
    ExtendsList *tail = NULL;
    ExtendsList *if_pos;

    /* BUGBUG need duplicate check */
    if (cd->interface_list) {
        for (tail = cd->interface_list; tail->next; tail = tail->next)
            ;
    }
    for (super_pos = cd->super_class; super_pos;
         super_pos = super_pos->super_class) {
        for (if_pos = super_pos->interface_list; if_pos;
             if_pos = if_pos->next) {
            ExtendsList *new_extends = dkc_malloc(sizeof(ExtendsList));
            *new_extends  = *if_pos;
            new_extends->next = NULL;
            if (tail) {
                tail->next = new_extends;
            } else {
                cd->interface_list = new_extends;
            }
            tail = new_extends;
        }
    }
}

static void
fix_extends(ClassDefinition *cd)
{
    ExtendsList *e_pos;
    ExtendsList *last_el = NULL;
    ClassDefinition *super;
    int dummy_class_index;
    ExtendsList *new_el;

    if (cd->class_or_interface == DVM_INTERFACE_DEFINITION
        && cd->extends != NULL) {
        dkc_compile_error(cd->line_number,
                          INTERFACE_INHERIT_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    cd->interface_list = NULL;

    for (e_pos = cd->extends; e_pos; e_pos = e_pos->next) {
        super = search_class_and_add(cd->line_number,
                                     e_pos->identifier,
                                     &dummy_class_index);
        e_pos->class_definition = super;

        new_el = dkc_malloc(sizeof(ExtendsList));
        *new_el = *e_pos;
        new_el->next = NULL;

        if (super->class_or_interface == DVM_CLASS_DEFINITION) {
            if (cd->super_class) {
                dkc_compile_error(cd->line_number,
                                  MULTIPLE_INHERITANCE_ERR,
                                  STRING_MESSAGE_ARGUMENT, "name", super->name,
                                  MESSAGE_ARGUMENT_END);
            }
            if (!super->is_abstract) {
                dkc_compile_error(cd->line_number,
                                  INHERIT_CONCRETE_CLASS_ERR,
                                  STRING_MESSAGE_ARGUMENT, "name", super->name,
                                  MESSAGE_ARGUMENT_END);
            }
            cd->super_class = super;
        } else {
            DBG_assert(super->class_or_interface == DVM_INTERFACE_DEFINITION,
                       ("super..%d", super->class_or_interface));
            if (cd->interface_list == NULL) {
                cd->interface_list = new_el;
            } else {
                last_el->next = new_el;
            }
            last_el = new_el;
        }
    }
}

static void
add_default_constructor(ClassDefinition *cd)
{
    MemberDeclaration *member_pos;
    MemberDeclaration *tail = NULL;
    TypeSpecifier *type;
    FunctionDefinition *fd;
    Block *block;
    Statement *statement;
    Expression *super_e;
    Expression *member_e;
    Expression *func_call_e;
    ClassOrMemberModifierList modifier;
    ClassOrMemberModifierList modifier2;
    ClassOrMemberModifierList modifier3;

    for (member_pos = cd->member; member_pos;
         member_pos = member_pos->next) {
        if (member_pos->kind == METHOD_MEMBER
            && member_pos->u.method.is_constructor) {
            return;
        }
        tail = member_pos;
    }

    type = dkc_alloc_type_specifier(DVM_VOID_TYPE);
    block = dkc_alloc_block();
    modifier = dkc_create_class_or_member_modifier(VIRTUAL_MODIFIER);
    if (cd->super_class) {
        statement = dkc_alloc_statement(EXPRESSION_STATEMENT);
        super_e = dkc_create_super_expression();
        member_e = dkc_create_member_expression(super_e,
                                                DEFAULT_CONSTRUCTOR_NAME);
        func_call_e = dkc_create_function_call_expression(member_e, NULL);
        statement->u.expression_s = func_call_e;
        block->statement_list = dkc_create_statement_list(statement);
        modifier = dkc_create_class_or_member_modifier(VIRTUAL_MODIFIER);
        modifier2 = dkc_create_class_or_member_modifier(OVERRIDE_MODIFIER);
        modifier = dkc_chain_class_or_member_modifier(modifier, modifier2);
        modifier3 = dkc_create_class_or_member_modifier(PUBLIC_MODIFIER);
        modifier = dkc_chain_class_or_member_modifier(modifier, modifier3);
    } else {
        block->statement_list = NULL;
    }
    fd = dkc_create_function_definition(type, DEFAULT_CONSTRUCTOR_NAME,
                                        NULL, block);
    if (tail) {
        tail->next = dkc_create_method_member(&modifier, fd, DVM_TRUE);
    } else {
        cd->member = dkc_create_method_member(&modifier, fd, DVM_TRUE);
    }
}

static void
get_super_field_method_count(ClassDefinition *cd,
                             int *field_index_out, int *method_index_out)
{
    int field_index = -1;
    int method_index = -1;
    ClassDefinition *cd_pos;
    MemberDeclaration *member_pos;
    
    for (cd_pos = cd->super_class; cd_pos; cd_pos = cd_pos->super_class) {
        for (member_pos = cd_pos->member; member_pos;
             member_pos = member_pos->next) {
            if (member_pos->kind == METHOD_MEMBER) {
                if (member_pos->u.method.method_index > method_index) {
                    method_index = member_pos->u.method.method_index;
                }
            } else {
                DBG_assert(member_pos->kind == FIELD_MEMBER,
                           ("member_pos->kind..%d", member_pos->kind));
                if (member_pos->u.field.field_index > field_index) {
                    field_index = member_pos->u.field.field_index;
                }
            }
        }
    }
    *field_index_out = field_index + 1;
    *method_index_out = method_index + 1;
}

static MemberDeclaration *
search_member_in_super(ClassDefinition *class_def, char *member_name)
{
    MemberDeclaration *member = NULL;
    ExtendsList *extends_p;

    if (class_def->super_class) {
        member = dkc_search_member(class_def->super_class, member_name);
    }
    if (member) {
        return member;
    }
    for (extends_p = class_def->interface_list; extends_p;
         extends_p = extends_p->next) {
        member = dkc_search_member(extends_p->class_definition, member_name);
        if (member) {
            return member;
        }
    }
    return NULL;
}

static void
check_method_override(MemberDeclaration *super_method,
                      MemberDeclaration *sub_method)
{
    if ((super_method->access_modifier == DVM_PUBLIC_ACCESS
         && sub_method->access_modifier != DVM_PUBLIC_ACCESS)
        || (super_method->access_modifier == DVM_FILE_ACCESS
            && sub_method->access_modifier == DVM_PRIVATE_ACCESS)) {
        dkc_compile_error(sub_method->line_number,
                          OVERRIDE_METHOD_ACCESSIBILITY_ERR,
                          STRING_MESSAGE_ARGUMENT, "name",
                          sub_method->u.method.function_definition->name,
                          MESSAGE_ARGUMENT_END);
    }
    if (!sub_method->u.method.is_constructor) {
        check_function_compatibility(super_method
                                     ->u.method.function_definition,
                                     sub_method->u.method.function_definition);
    }
}

static void
fix_class_list(DKC_Compiler *compiler)
{
    ClassDefinition *class_pos;
    MemberDeclaration *member_pos;
    MemberDeclaration *super_member;
    int field_index;
    int method_index;
    char *abstract_method_name;

    for (class_pos = compiler->class_definition_list;
         class_pos; class_pos = class_pos->next) {
        add_class(class_pos);
        fix_extends(class_pos);
    }
    for (class_pos = compiler->class_definition_list;
         class_pos; class_pos = class_pos->next) {
        add_super_interfaces(class_pos);
    }
    for (class_pos = compiler->class_definition_list;
         class_pos; class_pos = class_pos->next) {
        if (class_pos->class_or_interface != DVM_CLASS_DEFINITION)
            continue;
        compiler->current_class_definition = class_pos;
        add_default_constructor(class_pos);
        compiler->current_class_definition = NULL;
    }

    for (class_pos = compiler->class_definition_list;
         class_pos; class_pos = class_pos->next) {

        compiler->current_class_definition = class_pos;

        get_super_field_method_count(class_pos, &field_index, &method_index);
        abstract_method_name = NULL;
        for (member_pos = class_pos->member; member_pos;
             member_pos = member_pos->next) {
            if (member_pos->kind == METHOD_MEMBER) {
                fix_function(member_pos->u.method.function_definition);

                super_member
                    = search_member_in_super(class_pos,
                                             member_pos->u.method
                                             .function_definition->name);
                if (super_member) {
                    if (super_member->kind != METHOD_MEMBER) {
                        dkc_compile_error(member_pos->line_number,
                                          FIELD_OVERRIDED_ERR,
                                          STRING_MESSAGE_ARGUMENT, "name",
                                          super_member->u.field.name,
                                          MESSAGE_ARGUMENT_END);
                    }
                    if (!super_member->u.method.is_virtual) {
                        dkc_compile_error(member_pos->line_number,
                                          NON_VIRTUAL_METHOD_OVERRIDED_ERR,
                                          STRING_MESSAGE_ARGUMENT, "name",
                                          member_pos->u.method
                                          .function_definition->name,
                                          MESSAGE_ARGUMENT_END);
                    }
                    if (!member_pos->u.method.is_override) {
                        dkc_compile_error(member_pos->line_number,
                                          NEED_OVERRIDE_ERR,
                                          STRING_MESSAGE_ARGUMENT, "name",
                                          member_pos->u.method
                                          .function_definition->name,
                                          MESSAGE_ARGUMENT_END);
                    }
                    check_method_override(super_member, member_pos);

                    member_pos->u.method.method_index
                        = super_member->u.method.method_index;
                } else {
                    member_pos->u.method.method_index = method_index;
                    method_index++;
                }
                if (member_pos->u.method.is_abstract) {
                    abstract_method_name = member_pos->u.method
                        .function_definition->name;
                }
            } else {
                DBG_assert(member_pos->kind == FIELD_MEMBER,
                           ("member_pos->kind..%d", member_pos->kind));
                fix_type_specifier(member_pos->u.field.type);
                super_member
                    = search_member_in_super(class_pos,
                                             member_pos->u.field.name);
                if (super_member) {
                    dkc_compile_error(member_pos->line_number,
                                      FIELD_NAME_DUPLICATE_ERR,
                                      STRING_MESSAGE_ARGUMENT, "name",
                                      member_pos->u.field.name,
                                      MESSAGE_ARGUMENT_END);
                } else {
                    member_pos->u.field.field_index = field_index;
                    field_index++;
                }
            }
        }
        if (abstract_method_name && !class_pos->is_abstract) {
            dkc_compile_error(class_pos->line_number,
                              ABSTRACT_METHOD_IN_CONCRETE_CLASS_ERR,
                              STRING_MESSAGE_ARGUMENT,
                              "method_name", abstract_method_name,
                              MESSAGE_ARGUMENT_END);
        }
        compiler->current_class_definition = NULL;
    }
}

void
dkc_fix_tree(DKC_Compiler *compiler)
{
    FunctionDefinition *func_pos;
    DeclarationList *dl;
    int var_count = 0;

    fix_class_list(compiler);

    for (func_pos = compiler->function_list; func_pos;
         func_pos = func_pos->next) {
        reserve_function_index(compiler, func_pos);
    }

    fix_statement_list(NULL, compiler->statement_list, NULL);
    
    for (func_pos = compiler->function_list; func_pos;
         func_pos = func_pos->next) {
        if (func_pos->class_definition == NULL) {
            fix_function(func_pos);
        }
    }

    for (dl = compiler->declaration_list; dl; dl = dl->next) {
        dl->declaration->variable_index = var_count;
        var_count++;
    }
}
