#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "MEM.h"
#include "DBG.h"
#include "diksamc.h"

extern OpcodeInfo dvm_opcode_info[];

#define OPCODE_ALLOC_SIZE (256)
#define LABEL_TABLE_ALLOC_SIZE (256)

typedef struct {
    int label_address;
} LabelTable;

typedef struct {
    int         size;
    int         alloc_size;
    DVM_Byte    *code;
    int         label_table_size;
    int         label_table_alloc_size;
    LabelTable  *label_table;
    int         line_number_size;
    DVM_LineNumber      *line_number;
} OpcodeBuf;

static DVM_Executable *
alloc_executable(void)
{
    DVM_Executable      *exe;

    exe = MEM_malloc(sizeof(DVM_Executable));
    exe->constant_pool_count = 0;
    exe->constant_pool = NULL;
    exe->global_variable_count = 0;
    exe->global_variable = NULL;
    exe->function_count = 0;
    exe->function = NULL;
    exe->type_specifier_count = 0;
    exe->type_specifier = NULL;
    exe->code_size = 0;
    exe->code = NULL;

    return exe;
}

static int
add_constant_pool(DVM_Executable *exe, DVM_ConstantPool *cp)
{
    int ret;

    exe->constant_pool
        = MEM_realloc(exe->constant_pool,
                      sizeof(DVM_ConstantPool)
                      * (exe->constant_pool_count + 1));
    exe->constant_pool[exe->constant_pool_count] = *cp;

    ret = exe->constant_pool_count;
    exe->constant_pool_count++;

    return ret;
}

static DVM_TypeSpecifier *copy_type_specifier(TypeSpecifier *src);

static DVM_LocalVariable *
copy_parameter_list(ParameterList *src, int *param_count_p)
{
    int param_count = 0;
    ParameterList *param;
    DVM_LocalVariable *dest;
    int i;

    for (param = src; param; param = param->next) {
        param_count++;
    }
    *param_count_p = param_count;
    dest = MEM_malloc(sizeof(DVM_LocalVariable) * param_count);
    
    for (param = src, i = 0; param; param = param->next, i++) {
        dest[i].name = MEM_strdup(param->name);
        dest[i].type = copy_type_specifier(param->type);
    }

    return dest;
}

static DVM_LocalVariable *
copy_local_variables(FunctionDefinition *fd, int param_count)
{
    int i;
    int local_variable_count;
    DVM_LocalVariable *dest;

    local_variable_count = fd->local_variable_count - param_count;

    dest = MEM_malloc(sizeof(DVM_LocalVariable) * local_variable_count);

    for (i = 0; i < local_variable_count; i++) {
        dest[i].name
            = MEM_strdup(fd->local_variable[i+param_count]->name);
        dest[i].type
            = copy_type_specifier(fd->local_variable[i+param_count]->type);
    }

    return dest;
}

static void
copy_type_specifier_no_alloc(TypeSpecifier *src, DVM_TypeSpecifier *dest)
{
    int derive_count = 0;
    TypeDerive *derive;
    int param_count;
    int i;

    dest->basic_type = src->basic_type;

    for (derive = src->derive; derive; derive = derive->next) {
        derive_count++;
    }
    dest->derive_count = derive_count;
    dest->derive = MEM_malloc(sizeof(DVM_TypeDerive) * derive_count);
    for (i = 0, derive = src->derive; derive;
         derive = derive->next, i++) {
        switch (derive->tag) {
        case FUNCTION_DERIVE:
            dest->derive[i].tag = DVM_FUNCTION_DERIVE;
            dest->derive[i].u.function_d.parameter
                = copy_parameter_list(derive->u.function_d.parameter_list,
                                      &param_count);
            dest->derive[i].u.function_d.parameter_count = param_count;
            break;
        case ARRAY_DERIVE:
            dest->derive[i].tag = DVM_ARRAY_DERIVE;
            break;
        default:
            DBG_assert(0, ("derive->tag..%d\n", derive->tag));
        }
    }
}

static DVM_TypeSpecifier *
copy_type_specifier(TypeSpecifier *src)
{
    DVM_TypeSpecifier *dest;

    dest = MEM_malloc(sizeof(DVM_TypeSpecifier));

    copy_type_specifier_no_alloc(src, dest);

    return dest;
}

static void
add_global_variable(DKC_Compiler *compiler, DVM_Executable *exe)
{
    DeclarationList *dl;
    int i;
    int var_count = 0;

    for (dl = compiler->declaration_list; dl; dl = dl->next) {
        var_count++;
    }
    exe->global_variable_count = var_count;
    exe->global_variable = MEM_malloc(sizeof(DVM_Variable) * var_count);

    for (dl = compiler->declaration_list, i = 0; dl; dl = dl->next, i++) {
        exe->global_variable[i].name = MEM_strdup(dl->declaration->name);
        exe->global_variable[i].type
            = copy_type_specifier(dl->declaration->type);
    }
}

static int
add_type_specifier(TypeSpecifier *src, DVM_Executable *exe)
{
    int ret;

    exe->type_specifier
        = MEM_realloc(exe->type_specifier,
                      sizeof(DVM_TypeSpecifier)
                      * (exe->type_specifier_count + 1));
    copy_type_specifier_no_alloc(src,
                                 &exe->type_specifier
                                 [exe->type_specifier_count]);

    ret = exe->type_specifier_count;
    exe->type_specifier_count++;

    return ret;
}

static void
add_line_number(OpcodeBuf *ob, int line_number, int start_pc)
{
    if (ob->line_number == NULL
        || (ob->line_number[ob->line_number_size-1].line_number
            != line_number)) {
        ob->line_number = MEM_realloc(ob->line_number,
                                      sizeof(DVM_LineNumber)
                                      * (ob->line_number_size + 1));

        ob->line_number[ob->line_number_size].line_number = line_number;
        ob->line_number[ob->line_number_size].start_pc = start_pc;
        ob->line_number[ob->line_number_size].pc_count
            = ob->size - start_pc;
        ob->line_number_size++;

    } else {
        ob->line_number[ob->line_number_size-1].pc_count
            += ob->size - start_pc;
    }
}

static void
generate_code(OpcodeBuf *ob, int line_number, DVM_Opcode code,  ...)
{
    va_list     ap;
    int         i;
    char        *param;
    int         param_count;
    int         start_pc;

    va_start(ap, code);

    param = dvm_opcode_info[(int)code].parameter;
    param_count = strlen(param);
    if (ob->alloc_size < ob->size + 1 + (param_count * 2)) {
        ob->code = MEM_realloc(ob->code, ob->alloc_size + OPCODE_ALLOC_SIZE);
        ob->alloc_size += OPCODE_ALLOC_SIZE;
    }

    start_pc = ob->size;
    ob->code[ob->size] = code;
    ob->size++;
    for (i = 0; param[i] != '\0'; i++) {
        unsigned int value = va_arg(ap, int);
        switch (param[i]) {
        case 'b': /* byte */
            ob->code[ob->size] = (DVM_Byte)value;
            ob->size++;
            break;
        case 's': /* short(2byte int) */
            ob->code[ob->size] = (DVM_Byte)(value >> 8);
            ob->code[ob->size+1] = (DVM_Byte)(value & 0xff);
            ob->size += 2;
            break;
        case 'p': /* constant pool index */
            ob->code[ob->size] = (DVM_Byte)(value >> 8);
            ob->code[ob->size+1] = (DVM_Byte)(value & 0xff);
            ob->size += 2;
            break;
        default:
            DBG_assert(0, ("param..%s, i..%d", param, i));
        }
    }
    add_line_number(ob, line_number, start_pc);

    va_end(ap);
}

static void
generate_boolean_expression(DVM_Executable *cf, Expression *expr,
                            OpcodeBuf *ob)
{
    if (expr->u.boolean_value) {
        generate_code(ob, expr->line_number, DVM_PUSH_INT_1BYTE, 1);
    } else {
        generate_code(ob, expr->line_number, DVM_PUSH_INT_1BYTE, 0);
    }
}

static void
generate_int_expression(DVM_Executable *cf, Expression *expr,
                        OpcodeBuf *ob)
{
    DVM_ConstantPool cp;
    int cp_idx;

    if (expr->u.int_value >= 0 && expr->u.int_value < 256) {
        generate_code(ob, expr->line_number,
                      DVM_PUSH_INT_1BYTE, expr->u.int_value);
    } else if (expr->u.int_value >= 0 && expr->u.int_value < 65536) {
        generate_code(ob, expr->line_number,
                      DVM_PUSH_INT_2BYTE, expr->u.int_value);
    } else {
        cp.tag = DVM_CONSTANT_INT;
        cp.u.c_int = expr->u.int_value;
        cp_idx = add_constant_pool(cf, &cp);

        generate_code(ob, expr->line_number, DVM_PUSH_INT, cp_idx);
    }
}

static void
generate_double_expression(DVM_Executable *cf, Expression *expr,
                           OpcodeBuf *ob)
{
    DVM_ConstantPool cp;
    int cp_idx;

    if (expr->u.double_value == 0.0) {
        generate_code(ob, expr->line_number, DVM_PUSH_DOUBLE_0);
    } else if (expr->u.double_value == 1.0) {
        generate_code(ob, expr->line_number, DVM_PUSH_DOUBLE_1);
    } else {
        cp.tag = DVM_CONSTANT_DOUBLE;
        cp.u.c_double = expr->u.double_value;
        cp_idx = add_constant_pool(cf, &cp);

        generate_code(ob, expr->line_number, DVM_PUSH_DOUBLE, cp_idx);
    }
}

static void
generate_string_expression(DVM_Executable *cf, Expression *expr,
                           OpcodeBuf *ob)
{
    DVM_ConstantPool cp;
    int cp_idx;

    cp.tag = DVM_CONSTANT_STRING;
    cp.u.c_string = expr->u.string_value;
    cp_idx = add_constant_pool(cf, &cp);
    generate_code(ob, expr->line_number, DVM_PUSH_STRING, cp_idx);
}

static int
get_opcode_type_offset(TypeSpecifier *type)
{
    if (type->derive != NULL) {
        DBG_assert(type->derive->tag = ARRAY_DERIVE,
                   ("type->derive->tag..%d", type->derive->tag));
        return 2;
    }

    switch (type->basic_type) {
    case DVM_BOOLEAN_TYPE:
        return 0;
        break;
    case DVM_INT_TYPE:
        return 0;
        break;
    case DVM_DOUBLE_TYPE:
        return 1;
        break;
    case DVM_STRING_TYPE:
        return 2;
        break;
    case DVM_NULL_TYPE: /* FALLTHRU */
    default:
        DBG_assert(0, ("basic_type..%d", type->basic_type));
    }

    return 0;
}

static void
generate_identifier_expression(DVM_Executable *exe, Block *block,
                               Expression *expr, OpcodeBuf *ob)
{
    if (expr->u.identifier.is_function) {
        generate_code(ob, expr->line_number,
                      DVM_PUSH_FUNCTION,
                      expr->u.identifier.u.function->index);
        return;
    }

    if (expr->u.identifier.u.declaration->is_local) {
        generate_code(ob, expr->line_number,
                      DVM_PUSH_STACK_INT
                      + get_opcode_type_offset(expr->u.identifier
                                               .u.declaration->type),
                      expr->u.identifier.u.declaration->variable_index);
    } else {
        generate_code(ob, expr->line_number,
                      DVM_PUSH_STATIC_INT
                      + get_opcode_type_offset(expr->u.identifier
                                               .u.declaration->type),
                      expr->u.identifier.u.declaration->variable_index);
    }
}

static void generate_expression(DVM_Executable *exe, Block *current_block,
                                Expression *expr, OpcodeBuf *ob);

static void
generate_pop_to_identifier(Declaration *decl, int line_number,
                           OpcodeBuf *ob)
{
    if (decl->is_local) {
        generate_code(ob, line_number,
                      DVM_POP_STACK_INT
                      + get_opcode_type_offset(decl->type),
                      decl->variable_index);
    } else {
        generate_code(ob, line_number,
                      DVM_POP_STATIC_INT
                      + get_opcode_type_offset(decl->type),
                      decl->variable_index);
    }
}

static void
generate_pop_to_lvalue(DVM_Executable *exe, Block *block,
                       Expression *expr, OpcodeBuf *ob)
{
    if (expr->kind == IDENTIFIER_EXPRESSION) {
        generate_pop_to_identifier(expr->u.identifier.u.declaration,
                                   expr->line_number,
                                   ob);
    } else {
        DBG_assert(expr->kind == INDEX_EXPRESSION,
                   ("expr->kind..%d", expr->kind));
        
        generate_expression(exe, block, expr->u.index_expression.array, ob);
        generate_expression(exe, block, expr->u.index_expression.index, ob);
        generate_code(ob, expr->line_number,
                      DVM_POP_ARRAY_INT
                      + get_opcode_type_offset(expr->type));

    }
}

static void
generate_assign_expression(DVM_Executable *exe, Block *block,
                           Expression *expr, OpcodeBuf *ob,
                           DVM_Boolean is_toplevel)
{
    if (expr->u.assign_expression.operator != NORMAL_ASSIGN) {
        generate_expression(exe, block, 
                            expr->u.assign_expression.left, ob);
    }
    generate_expression(exe, block, expr->u.assign_expression.operand, ob);

    switch (expr->u.assign_expression.operator) {
    case NORMAL_ASSIGN : /* FALLTHRU */
        break;
    case ADD_ASSIGN:
        generate_code(ob, expr->line_number,
                      DVM_ADD_INT
                      + get_opcode_type_offset(expr->type));
        break;
    case SUB_ASSIGN:
        generate_code(ob, expr->line_number,
                      DVM_SUB_INT
                      + get_opcode_type_offset(expr->type));
        break;
    case MUL_ASSIGN:
        generate_code(ob, expr->line_number,
                      DVM_MUL_INT
                      + get_opcode_type_offset(expr->type));
        break;
    case DIV_ASSIGN:
        generate_code(ob, expr->line_number,
                      DVM_DIV_INT
                      + get_opcode_type_offset(expr->type));
        break;
    case MOD_ASSIGN:
        generate_code(ob, expr->line_number,
                      DVM_MOD_INT
                      + get_opcode_type_offset(expr->type));
        break;
    default:
        DBG_assert(0, ("operator..%d\n", expr->u.assign_expression.operator));
    }

    if (!is_toplevel) {
        generate_code(ob, expr->line_number, DVM_DUPLICATE);
    }
    generate_pop_to_lvalue(exe, block,
                           expr->u.assign_expression.left, ob);
}

static void
generate_binary_expression(DVM_Executable *exe, Block *block,
                           Expression *expr, DVM_Opcode code,
                           OpcodeBuf *ob)
{
    int offset;
    Expression *left = expr->u.binary_expression.left;
    Expression *right = expr->u.binary_expression.right;

    generate_expression(exe, block, left, ob);
    generate_expression(exe, block, right, ob);

    if ((left->kind == NULL_EXPRESSION && right->kind != NULL_EXPRESSION)
        || (left->kind != NULL_EXPRESSION && right->kind == NULL_EXPRESSION)) {
        offset = 2; /* object type */

    } else if ((code == DVM_EQ_INT || code == DVM_NE_INT)
               && dkc_is_string(left->type)) {
        offset = 3; /* string type */

    } else {
        offset = get_opcode_type_offset(expr->u.binary_expression.left->type);
    }
    generate_code(ob, expr->line_number,
                  code + offset);
}

static int
get_label(OpcodeBuf *ob)
{
    int ret;

    if (ob->label_table_alloc_size < ob->label_table_size + 1) {
        ob->label_table = MEM_realloc(ob->label_table,
                                      (ob->label_table_alloc_size
                                       + LABEL_TABLE_ALLOC_SIZE)
                                      * sizeof(LabelTable));
        ob->label_table_alloc_size += LABEL_TABLE_ALLOC_SIZE;
    }
    ret = ob->label_table_size;
    ob->label_table_size++;

    return ret;
}

static void
set_label(OpcodeBuf *ob, int label)
{
    ob->label_table[label].label_address = ob->size;
}

static void
generate_logical_and_expression(DVM_Executable *exe, Block *block,
                                Expression *expr,
                                OpcodeBuf *ob)
{
    int false_label;

    false_label = get_label(ob);
    generate_expression(exe, block, expr->u.binary_expression.left, ob);
    generate_code(ob, expr->line_number, DVM_DUPLICATE);
    generate_code(ob, expr->line_number, DVM_JUMP_IF_FALSE, false_label);
    generate_expression(exe, block, expr->u.binary_expression.right, ob);
    generate_code(ob, expr->line_number, DVM_LOGICAL_AND);
    set_label(ob, false_label);
}

static void
generate_logical_or_expression(DVM_Executable *exe, Block *block,
                               Expression *expr,
                               OpcodeBuf *ob)
{
    int true_label;

    true_label = get_label(ob);
    generate_expression(exe, block, expr->u.binary_expression.left, ob);
    generate_code(ob, expr->line_number, DVM_DUPLICATE);
    generate_code(ob, expr->line_number, DVM_JUMP_IF_TRUE, true_label);
    generate_expression(exe, block, expr->u.binary_expression.right, ob);
    generate_code(ob, expr->line_number, DVM_LOGICAL_OR);
    set_label(ob, true_label);
}

static void
generate_cast_expression(DVM_Executable *exe, Block *block,
                         Expression *expr, OpcodeBuf *ob)
{
    generate_expression(exe, block, expr->u.cast.operand, ob);
    switch (expr->u.cast.type) {
    case INT_TO_DOUBLE_CAST:
        generate_code(ob, expr->line_number, DVM_CAST_INT_TO_DOUBLE);
        break;
    case DOUBLE_TO_INT_CAST:
        generate_code(ob, expr->line_number, DVM_CAST_DOUBLE_TO_INT);
        break;
    case BOOLEAN_TO_STRING_CAST:
        generate_code(ob, expr->line_number, DVM_CAST_BOOLEAN_TO_STRING);
        break;
    case INT_TO_STRING_CAST:
        generate_code(ob, expr->line_number, DVM_CAST_INT_TO_STRING);
        break;
    case DOUBLE_TO_STRING_CAST:
        generate_code(ob, expr->line_number, DVM_CAST_DOUBLE_TO_STRING);
        break;
    default:
        DBG_assert(0, ("expr->u.cast.type..%d", expr->u.cast.type));
    }
}

static void
generate_array_literal_expression(DVM_Executable *exe, Block *block,
                                  Expression *expr, OpcodeBuf *ob)
{
    ExpressionList *pos;
    int count;

    DBG_assert(expr->type->derive
               && expr->type->derive->tag == ARRAY_DERIVE,
               ("array literal is not array."));

    count = 0;
    for (pos = expr->u.array_literal; pos; pos = pos->next) {
        generate_expression(exe, block, pos->expression, ob);
        count++;
    }
    DBG_assert(count > 0, ("empty array literal"));
    
    generate_code(ob, expr->line_number,
                  DVM_NEW_ARRAY_LITERAL_INT
                  + get_opcode_type_offset(expr->u.array_literal
                                           ->expression->type),
                  count);
}

static void
generate_index_expression(DVM_Executable *exe, Block *block,
                          Expression *expr, OpcodeBuf *ob)
{
    generate_expression(exe, block, expr->u.index_expression.array, ob);
    generate_expression(exe, block, expr->u.index_expression.index, ob);

    generate_code(ob, expr->line_number, DVM_PUSH_ARRAY_INT
                  + get_opcode_type_offset(expr->type));
}

static void
generate_inc_dec_expression(DVM_Executable *exe, Block *block,
                            Expression *expr, ExpressionKind kind,
                            OpcodeBuf *ob, DVM_Boolean is_toplevel)
{
    generate_expression(exe, block, expr->u.inc_dec.operand, ob);

    if (kind == INCREMENT_EXPRESSION) {
        generate_code(ob, expr->line_number, DVM_INCREMENT);
    } else {
        DBG_assert(kind == DECREMENT_EXPRESSION, ("kind..%d\n", kind));
        generate_code(ob, expr->line_number, DVM_DECREMENT);
    }
    if (!is_toplevel) {
        generate_code(ob, expr->line_number, DVM_DUPLICATE);
    }
    generate_pop_to_lvalue(exe, block,
                           expr->u.inc_dec.operand, ob);
}

static void
generate_function_call_expression(DVM_Executable *exe, Block *block,
                                  Expression *expr, OpcodeBuf *ob)
{
    FunctionCallExpression *fce = &expr->u.function_call_expression;
    ArgumentList *arg_pos;

    for (arg_pos = fce->argument; arg_pos; arg_pos = arg_pos->next) {
        generate_expression(exe, block, arg_pos->expression, ob);
    }
    generate_expression(exe, block, fce->function, ob);
    generate_code(ob, expr->line_number, DVM_INVOKE);
}

static void
generate_null_expression(DVM_Executable *exe, Expression *expr,
                         OpcodeBuf *ob)
{
    generate_code(ob, expr->line_number, DVM_PUSH_NULL);
}

static void
generate_array_creation_expression(DVM_Executable *exe, Block *block,
                                   Expression *expr, OpcodeBuf *ob)
{
    int index;
    TypeSpecifier type;
    ArrayDimension *dim_pos;
    int dim_count;

    index = add_type_specifier(expr->type, exe);

    DBG_assert(expr->type->derive->tag == ARRAY_DERIVE,
               ("expr->type->derive->tag..%d", expr->type->derive->tag));

    type.basic_type = expr->type->basic_type;
    type.derive = expr->type->derive;

    dim_count = 0;
    for (dim_pos = expr->u.array_creation.dimension;
         dim_pos; dim_pos = dim_pos->next) {
        if (dim_pos->expression == NULL)
            break;

        generate_expression(exe, block, dim_pos->expression, ob);
        dim_count++;
    }

    generate_code(ob, expr->line_number, DVM_NEW_ARRAY, dim_count, index);
}

static void
generate_expression(DVM_Executable *exe, Block *current_block,
                    Expression *expr, OpcodeBuf *ob)
{
    switch (expr->kind) {
    case BOOLEAN_EXPRESSION:
        generate_boolean_expression(exe, expr, ob);
        break;
    case INT_EXPRESSION:
        generate_int_expression(exe, expr, ob);
        break;
    case DOUBLE_EXPRESSION:
        generate_double_expression(exe, expr, ob);
        break;
    case STRING_EXPRESSION:
        generate_string_expression(exe, expr, ob);
        break;
    case IDENTIFIER_EXPRESSION:
        generate_identifier_expression(exe, current_block,
                                       expr, ob);
        break;
    case COMMA_EXPRESSION:
        generate_expression(exe, current_block, expr->u.comma.left, ob);
        generate_expression(exe, current_block, expr->u.comma.right, ob);
        break;
    case ASSIGN_EXPRESSION:
        generate_assign_expression(exe, current_block, expr, ob, DVM_FALSE);
        break;
    case ADD_EXPRESSION:
        generate_binary_expression(exe, current_block, expr,
                                   DVM_ADD_INT, ob);
        break;
    case SUB_EXPRESSION:
        generate_binary_expression(exe, current_block, expr,
                                   DVM_SUB_INT, ob);
        break;
    case MUL_EXPRESSION:
        generate_binary_expression(exe, current_block, expr,
                                   DVM_MUL_INT, ob);
        break;
    case DIV_EXPRESSION:
        generate_binary_expression(exe, current_block, expr,
                                   DVM_DIV_INT, ob);
        break;
    case MOD_EXPRESSION:
        generate_binary_expression(exe, current_block, expr,
                                   DVM_MOD_INT, ob);
        break;
    case EQ_EXPRESSION:
        generate_binary_expression(exe, current_block, expr,
                                   DVM_EQ_INT, ob);
        break;
    case NE_EXPRESSION:
        generate_binary_expression(exe, current_block, expr,
                                   DVM_NE_INT, ob);
        break;
    case GT_EXPRESSION:
        generate_binary_expression(exe, current_block, expr,
                                   DVM_GT_INT, ob);
        break;
    case GE_EXPRESSION:
        generate_binary_expression(exe, current_block, expr,
                                   DVM_GE_INT, ob);
        break;
    case LT_EXPRESSION:
        generate_binary_expression(exe, current_block, expr,
                                   DVM_LT_INT, ob);
        break;
    case LE_EXPRESSION:
        generate_binary_expression(exe, current_block, expr,
                                   DVM_LE_INT, ob);
        break;
    case LOGICAL_AND_EXPRESSION:
        generate_logical_and_expression(exe, current_block, expr, ob);
        break;
    case LOGICAL_OR_EXPRESSION:
        generate_logical_or_expression(exe, current_block, expr, ob);
        break;
    case MINUS_EXPRESSION:
        generate_expression(exe, current_block, expr->u.minus_expression, ob);
        generate_code(ob, expr->line_number,
                      DVM_MINUS_INT
                      + get_opcode_type_offset(expr->type));
        break;
    case LOGICAL_NOT_EXPRESSION:
        generate_expression(exe, current_block, expr->u.logical_not, ob);
        generate_code(ob, expr->line_number, DVM_LOGICAL_NOT);
        break;
    case FUNCTION_CALL_EXPRESSION:
        generate_function_call_expression(exe, current_block,
                                          expr, ob);
        break;
    case MEMBER_EXPRESSION:
        break;
    case NULL_EXPRESSION:
        generate_null_expression(exe, expr, ob);
        break;
    case ARRAY_LITERAL_EXPRESSION:
        generate_array_literal_expression(exe, current_block, expr, ob);
        break;
    case INDEX_EXPRESSION:
        generate_index_expression(exe, current_block, expr, ob);
        break;
    case INCREMENT_EXPRESSION:  /* FALLTHRU */
    case DECREMENT_EXPRESSION:
        generate_inc_dec_expression(exe, current_block, expr, expr->kind,
                                    ob, DVM_FALSE);
        break;
    case CAST_EXPRESSION:
        generate_cast_expression(exe, current_block, expr, ob);
        break;
    case ARRAY_CREATION_EXPRESSION:
        generate_array_creation_expression(exe, current_block, expr, ob);
        break;
    case EXPRESSION_KIND_COUNT_PLUS_1:  /* FALLTHRU */
    default:
        DBG_assert(0, ("expr->kind..%d", expr->kind));
    }
}

static void
generate_expression_statement(DVM_Executable *exe, Block *block,
                              Expression *expr, OpcodeBuf *ob)
{
    if (expr->kind == ASSIGN_EXPRESSION) {
        generate_assign_expression(exe, block, expr, ob, DVM_TRUE);
    } else if (expr->kind == INCREMENT_EXPRESSION
               || expr->kind == DECREMENT_EXPRESSION) {
        generate_inc_dec_expression(exe, block, expr, expr->kind, ob,
                                    DVM_TRUE);
    } else {
        generate_expression(exe, block, expr, ob);
        generate_code(ob, expr->line_number, DVM_POP);
    }
}

static void generate_statement_list(DVM_Executable *exe, Block *current_block,
                                    StatementList *statement_list,
                                    OpcodeBuf *ob);

static void
generate_if_statement(DVM_Executable *exe, Block *block,
                      Statement *statement, OpcodeBuf *ob)
{
    int if_false_label;
    int end_label;
    IfStatement *if_s = &statement->u.if_s;
    Elsif *elsif;

    generate_expression(exe, block, if_s->condition, ob);
    if_false_label = get_label(ob);
    generate_code(ob, statement->line_number,
                  DVM_JUMP_IF_FALSE, if_false_label);
    generate_statement_list(exe, if_s->then_block,
                            if_s->then_block->statement_list, ob);
    end_label = get_label(ob);
    generate_code(ob, statement->line_number, DVM_JUMP, end_label);
    set_label(ob, if_false_label);

    for (elsif = if_s->elsif_list; elsif; elsif = elsif->next) {
        generate_expression(exe, block, elsif->condition, ob);
        if_false_label = get_label(ob);
        generate_code(ob, statement->line_number,
                      DVM_JUMP_IF_FALSE, if_false_label);
        generate_statement_list(exe, elsif->block,
                                elsif->block->statement_list, ob);
        generate_code(ob, statement->line_number, DVM_JUMP, end_label);
        set_label(ob, if_false_label);
    }
    if (if_s->else_block) {
        generate_statement_list(exe, if_s->else_block,
                                if_s->else_block->statement_list,
                                ob);
    }
    set_label(ob, end_label);
}

static void
generate_while_statement(DVM_Executable *exe, Block *block,
                         Statement *statement, OpcodeBuf *ob)
{
    int loop_label;
    WhileStatement *while_s = &statement->u.while_s;

    loop_label = get_label(ob);
    set_label(ob, loop_label);

    generate_expression(exe, block, while_s->condition, ob);

    while_s->block->parent.statement.break_label = get_label(ob);
    while_s->block->parent.statement.continue_label = get_label(ob);

    generate_code(ob, statement->line_number,
                  DVM_JUMP_IF_FALSE,
                  while_s->block->parent.statement.break_label);
    generate_statement_list(exe, while_s->block,
                            while_s->block->statement_list, ob);

    set_label(ob, while_s->block->parent.statement.continue_label);
    generate_code(ob, statement->line_number, DVM_JUMP, loop_label);
    set_label(ob, while_s->block->parent.statement.break_label);
}

static void
generate_for_statement(DVM_Executable *exe, Block *block,
                       Statement *statement, OpcodeBuf *ob)
{
    int loop_label;
    ForStatement *for_s = &statement->u.for_s;

    if (for_s->init) {
        generate_expression_statement(exe, block, for_s->init, ob);
    }
    loop_label = get_label(ob);
    set_label(ob, loop_label);

    if (for_s->condition) {
        generate_expression(exe, block, for_s->condition, ob);
    }

    for_s->block->parent.statement.break_label = get_label(ob);
    for_s->block->parent.statement.continue_label = get_label(ob);

    if (for_s->condition) {
        generate_code(ob, statement->line_number,
                      DVM_JUMP_IF_FALSE,
                      for_s->block->parent.statement.break_label);
    }

    generate_statement_list(exe, for_s->block,
                            for_s->block->statement_list, ob);
    set_label(ob, for_s->block->parent.statement.continue_label);

    if (for_s->post) {
        generate_expression_statement(exe, block, for_s->post, ob);
    }

    generate_code(ob, statement->line_number,
                  DVM_JUMP, loop_label);
    set_label(ob, for_s->block->parent.statement.break_label);
}

static void
generate_return_statement(DVM_Executable *exe, Block *block,
                          Statement *statement, OpcodeBuf *ob)
{
    DBG_assert(statement->u.return_s.return_value != NULL,
               ("return value is null."));

    generate_expression(exe, block, statement->u.return_s.return_value, ob);
    generate_code(ob, statement->line_number, DVM_RETURN);
}

static void
generate_break_statement(DVM_Executable *exe, Block *block,
                         Statement *statement, OpcodeBuf *ob)
{
    BreakStatement *break_s = &statement->u.break_s;
    Block       *block_p;

    for (block_p = block; block_p; block_p = block_p->outer_block) {
        if (block_p->type != WHILE_STATEMENT_BLOCK
            && block_p->type != FOR_STATEMENT_BLOCK)
            continue;

        if (break_s->label == NULL) {
            break;
        }

        if (block_p->type == WHILE_STATEMENT_BLOCK) {
            if (block_p->parent.statement.statement->u.while_s.label == NULL)
                continue;

            if (!strcmp(break_s->label,
                        block_p->parent.statement.statement
                        ->u.while_s.label)) {
                break;
            }
        } else if (block_p->type == FOR_STATEMENT_BLOCK) {
            if (block_p->parent.statement.statement->u.for_s.label == NULL)
                continue;

            if (!strcmp(break_s->label,
                        block_p->parent.statement.statement
                        ->u.for_s.label)) {
                break;
            }
        }
    }
    if (block_p == NULL) {
        dkc_compile_error(statement->line_number,
                          LABEL_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "label", break_s->label,
                          MESSAGE_ARGUMENT_END);
    }

    generate_code(ob, statement->line_number,
                  DVM_JUMP,
                  block_p->parent.statement.break_label);

}

static void
generate_continue_statement(DVM_Executable *exe, Block *block,
                            Statement *statement, OpcodeBuf *ob)
{
    ContinueStatement *continue_s = &statement->u.continue_s;
    Block       *block_p;
    

    for (block_p = block; block_p; block_p = block_p->outer_block) {
        if (block_p->type != WHILE_STATEMENT_BLOCK
            && block_p->type != FOR_STATEMENT_BLOCK)
            continue;

        if (continue_s->label == NULL) {
            break;
        }

        if (block_p->type == WHILE_STATEMENT_BLOCK) {
            if (block_p->parent.statement.statement->u.while_s.label == NULL)
                continue;

            if (!strcmp(continue_s->label,
                        block_p->parent.statement.statement
                        ->u.while_s.label)) {
                break;
            }
        } else if (block_p->type == FOR_STATEMENT_BLOCK) {
            if (block_p->parent.statement.statement->u.for_s.label == NULL)
                continue;

            if (!strcmp(continue_s->label,
                        block_p->parent.statement.statement
                        ->u.for_s.label)) {
                break;
            }
        }
    }
    if (block_p == NULL) {
        dkc_compile_error(statement->line_number,
                          LABEL_NOT_FOUND_ERR,
                          STRING_MESSAGE_ARGUMENT, "label", continue_s->label,
                          MESSAGE_ARGUMENT_END);
    }
    generate_code(ob, statement->line_number,
                  DVM_JUMP,
                  block_p->parent.statement.continue_label);
}

static void
generate_initializer(DVM_Executable *exe, Block *block,
                     Statement *statement, OpcodeBuf *ob)
{
    Declaration *decl = statement->u.declaration_s;
    if (decl->initializer == NULL)
        return;

    generate_expression(exe, block, decl->initializer, ob);
    generate_pop_to_identifier(decl, statement->line_number,
                               ob);
}

static void
generate_statement_list(DVM_Executable *exe, Block *current_block,
                        StatementList *statement_list,
                        OpcodeBuf *ob)
{
    StatementList *pos;

    for (pos = statement_list; pos; pos = pos->next) {
        switch (pos->statement->type) {
        case EXPRESSION_STATEMENT:
            generate_expression_statement(exe, current_block,
                                          pos->statement->u.expression_s, ob);
            break;
        case IF_STATEMENT:
            generate_if_statement(exe, current_block, pos->statement, ob);
            break;
        case WHILE_STATEMENT:
            generate_while_statement(exe, current_block, pos->statement, ob);
            break;
        case FOR_STATEMENT:
            generate_for_statement(exe, current_block, pos->statement, ob);
            break;
        case FOREACH_STATEMENT:
            break;
        case RETURN_STATEMENT:
            generate_return_statement(exe, current_block, pos->statement, ob);
            break;
        case BREAK_STATEMENT:
            generate_break_statement(exe, current_block, pos->statement, ob);
            break;
        case CONTINUE_STATEMENT:
            generate_continue_statement(exe, current_block,
                                        pos->statement, ob);
            break;
        case TRY_STATEMENT:
            break;
        case THROW_STATEMENT:
            break;
        case DECLARATION_STATEMENT:
            generate_initializer(exe, current_block,
                                 pos->statement, ob);
            break;
        case STATEMENT_TYPE_COUNT_PLUS_1: /* FALLTHRU */
        default:
            DBG_assert(0, ("pos->statement->type..", pos->statement->type));
        }
    }
}

static void
init_opcode_buf(OpcodeBuf *ob)
{
    ob->size = 0;
    ob->alloc_size = 0;
    ob->code = NULL;
    ob->label_table_size = 0;
    ob->label_table_alloc_size = 0;
    ob->label_table = NULL;
    ob->line_number_size = 0;
    ob->line_number = NULL;
}

static void
fix_labels(OpcodeBuf *ob)
{
    int i;
    int j;
    OpcodeInfo *info;
    int label;
    int address;

    for (i = 0; i < ob->size; i++) {
        if (ob->code[i] == DVM_JUMP
            || ob->code[i] == DVM_JUMP_IF_TRUE
            || ob->code[i] == DVM_JUMP_IF_FALSE) {
            label = (ob->code[i+1] << 8) + (ob->code[i+2]);
            address = ob->label_table[label].label_address;
            ob->code[i+1] = (DVM_Byte)(address >> 8);
            ob->code[i+2] = (DVM_Byte)(address &0xff);
        }
        info = &dvm_opcode_info[ob->code[i]];
        for (j = 0; info->parameter[j] != '\0'; j++) {
            switch (info->parameter[j]) {
            case 'b':
                i++;
                break;
            case 's': /* FALLTHRU */
            case 'p':
                i += 2;
                break;
            default:
                DBG_assert(0, ("param..%s, j..%d", info->parameter, j));
            }
        }
    }
}

static DVM_Byte *
fix_opcode_buf(OpcodeBuf *ob)
{
    DVM_Byte *ret;

    fix_labels(ob);
    ret = MEM_realloc(ob->code, ob->size);
    MEM_free(ob->label_table);

    return ret;
}

static void
copy_function(FunctionDefinition *src, DVM_Function *dest)
{
    dest->type = copy_type_specifier(src->type);
    dest->name = MEM_strdup(src->name);
    dest->parameter = copy_parameter_list(src->parameter,
                                          &dest->parameter_count);
    if (src->block) {
        dest->local_variable
            = copy_local_variables(src, dest->parameter_count);
        dest->local_variable_count
            = src->local_variable_count - dest->parameter_count;
    } else {
        dest->local_variable = NULL;
        dest->local_variable_count = 0;
    }
}

static int
calc_need_stack_size(DVM_Byte *code, int code_size)
{
    int i, j;
    int stack_size = 0;
    OpcodeInfo  *info;

    for (i = 0; i < code_size; i++) {
        info = &dvm_opcode_info[code[i]];
        if (info->stack_increment > 0) {
            stack_size += info->stack_increment;
        }
        for (j = 0; info->parameter[j] != '\0'; j++) {
            switch (info->parameter[j]) {
            case 'b':
                i++;
                break;
            case 's': /* FALLTHRU */
            case 'p':
                i += 2;
                break;
            default:
                DBG_assert(0, ("param..%s, j..%d", info->parameter, j));
            }
        }
    }

    return stack_size;
}

static void
add_functions(DKC_Compiler *compiler, DVM_Executable *exe)
{
    FunctionDefinition  *fd;
    int         i;
    int         func_count = 0;
    OpcodeBuf           ob;

    for (fd = compiler->function_list; fd; fd = fd->next) {
        func_count++;
    }
    exe->function_count = func_count;
    exe->function = MEM_malloc(sizeof(DVM_Function) * func_count);

    for (fd = compiler->function_list, i = 0; fd; fd = fd->next, i++) {
        copy_function(fd, &exe->function[i]);
        if (fd->block) {
            init_opcode_buf(&ob);
            generate_statement_list(exe, fd->block, fd->block->statement_list,
                                    &ob);

            exe->function[i].is_implemented = DVM_TRUE;
            exe->function[i].code_size = ob.size;
            exe->function[i].code = fix_opcode_buf(&ob);
            exe->function[i].line_number_size = ob.line_number_size;
            exe->function[i].line_number = ob.line_number;
            exe->function[i].need_stack_size
                = calc_need_stack_size(exe->function[i].code,
                                       exe->function[i].code_size);
        } else {
            exe->function[i].is_implemented = DVM_FALSE;
        }
    }
}

static void
add_top_level(DKC_Compiler *compiler, DVM_Executable *exe)
{
    OpcodeBuf           ob;

    init_opcode_buf(&ob);
    generate_statement_list(exe, NULL, compiler->statement_list,
                            &ob);
    
    exe->code_size = ob.size;
    exe->code = fix_opcode_buf(&ob);
    exe->line_number_size = ob.line_number_size;
    exe->line_number = ob.line_number;


    exe->need_stack_size = calc_need_stack_size(exe->code, exe->code_size);
}


DVM_Executable *
dkc_generate(DKC_Compiler *compiler)
{
    DVM_Executable      *exe;

    exe = alloc_executable();

    add_global_variable(compiler, exe);
    add_functions(compiler, exe);
    add_top_level(compiler, exe);

    return exe;
}
