#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "diksamc.h"
#include "DVM_dev.h"

DeclarationList *
dkc_chain_declaration(DeclarationList *list, Declaration *decl)
{
    DeclarationList *new_item;
    DeclarationList *pos;

    new_item = dkc_malloc(sizeof(DeclarationList));
    new_item->declaration = decl;
    new_item->next = NULL;

    if (list == NULL) {
        return new_item;
    }

    for (pos = list; pos->next != NULL; pos = pos->next)
        ;
    pos->next = new_item;

    return list;
}

Declaration *
dkc_alloc_declaration(TypeSpecifier *type, char *identifier)
{
    Declaration *decl;

    decl = dkc_malloc(sizeof(Declaration));
    decl->name = identifier;
    decl->type = type;
    decl->variable_index = -1;

    return decl;
}

PackageName *
dkc_create_package_name(char *identifier)
{
    PackageName *pn;

    pn = dkc_malloc(sizeof(PackageName));
    pn->name = identifier;
    pn->next = NULL;

    return pn;
}

PackageName *
dkc_chain_package_name(PackageName *list, char *identifier)
{
    PackageName *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = dkc_create_package_name(identifier);

    return list;
}

RequireList *
dkc_create_require_list(PackageName *package_name)
{
    RequireList *rl;
    DKC_Compiler *compiler;
    char *current_package_name;
    char *req_package_name;

    compiler = dkc_get_current_compiler();

    current_package_name = dkc_package_name_to_string(compiler->package_name);
    req_package_name = dkc_package_name_to_string(package_name);
    if (dvm_compare_string(req_package_name, current_package_name)
        && compiler->source_suffix == DKH_SOURCE) {
        dkc_compile_error(compiler->current_line_number,
                          REQUIRE_ITSELF_ERR, MESSAGE_ARGUMENT_END);
    }
    MEM_free(current_package_name);
    MEM_free(req_package_name);

    rl = dkc_malloc(sizeof(RequireList));
    rl->package_name = package_name;
    rl->line_number = dkc_get_current_compiler()->current_line_number;
    rl->next = NULL;

    return rl;
}

RequireList *
dkc_chain_require_list(RequireList *list, RequireList *add)
{
    RequireList *pos;
    char buf[LINE_BUF_SIZE];

    for (pos = list; pos->next; pos = pos->next) {
        if (dkc_compare_package_name(pos->package_name, add->package_name)) {
            char *package_name;
            package_name = dkc_package_name_to_string(add->package_name);
            dvm_strncpy(buf, package_name, LINE_BUF_SIZE);
            MEM_free(package_name);
            dkc_compile_error(dkc_get_current_compiler()->current_line_number,
                              REQUIRE_DUPLICATE_ERR,
                              STRING_MESSAGE_ARGUMENT, "package_name", buf,
                              MESSAGE_ARGUMENT_END);
        }
    }
    pos->next = add;

    return list;
}

RenameList *
dkc_create_rename_list(PackageName *package_name, char *identifier)
{
    RenameList *rl;
    PackageName *pre_tail;
    PackageName *tail;

    pre_tail = NULL;
    for (tail = package_name; tail->next; tail = tail->next) {
        pre_tail = tail;
    }
    if (pre_tail == NULL) {
        dkc_compile_error(dkc_get_current_compiler()->current_line_number,
                          RENAME_HAS_NO_PACKAGED_NAME_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    pre_tail->next = NULL;

    rl = dkc_malloc(sizeof(RenameList));
    rl->package_name = package_name;
    rl->original_name = tail->name;
    rl->renamed_name = identifier;
    rl->line_number = dkc_get_current_compiler()->current_line_number;
    rl->next = NULL;

    return rl;
}

RenameList *
dkc_chain_rename_list(RenameList *list, RenameList *add)
{
    RenameList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = add;

    return list;
}

static RequireList *
add_default_package(RequireList *require_list)
{
    RequireList *req_pos;
    DVM_Boolean default_package_required = DVM_FALSE;

    for (req_pos = require_list; req_pos; req_pos = req_pos->next) {
        char *temp_name
            = dkc_package_name_to_string(req_pos->package_name);
        if (!strcmp(temp_name, DVM_DIKSAM_DEFAULT_PACKAGE)) {
            default_package_required = DVM_TRUE;
        }
        MEM_free(temp_name);
    }

    if (!default_package_required) {
        PackageName *pn;
        RequireList *req_temp;

        pn = dkc_create_package_name(DVM_DIKSAM_DEFAULT_PACKAGE_P1);
        pn = dkc_chain_package_name(pn, DVM_DIKSAM_DEFAULT_PACKAGE_P2);
        req_temp = require_list;
        require_list = dkc_create_require_list(pn);
        require_list->next = req_temp;
    }
    return require_list;
}

void
dkc_set_require_and_rename_list(RequireList *require_list,
                                RenameList *rename_list)
{
    DKC_Compiler *compiler;
    char *current_package_name;

    compiler = dkc_get_current_compiler();

    current_package_name
        = dkc_package_name_to_string(compiler->package_name);

    if (!dvm_compare_string(current_package_name,
                            DVM_DIKSAM_DEFAULT_PACKAGE)) {
        require_list = add_default_package(require_list);
    }
    MEM_free(current_package_name);
    compiler->require_list = require_list;
    compiler->rename_list = rename_list;
}

static void
add_function_to_compiler(FunctionDefinition *fd)
{
    DKC_Compiler *compiler;
    FunctionDefinition *pos;

    compiler = dkc_get_current_compiler();
    if (compiler->function_list) {
        for (pos = compiler->function_list; pos->next; pos = pos->next)
            ;
        pos->next = fd;
    } else {
        compiler->function_list = fd;
    }
}

FunctionDefinition *
dkc_create_function_definition(TypeSpecifier *type, char *identifier,
                               ParameterList *parameter_list, Block *block)
{
    FunctionDefinition *fd;
    DKC_Compiler *compiler;

    compiler = dkc_get_current_compiler();

    fd = dkc_malloc(sizeof(FunctionDefinition));
    fd->type = type;
    fd->package_name = compiler->package_name;
    fd->name = identifier;
    fd->parameter = parameter_list;
    fd->block = block;
    fd->local_variable_count = 0;
    fd->local_variable = NULL;
    fd->class_definition = NULL;
    fd->end_line_number = compiler->current_line_number;
    fd->next = NULL;
    if (block) {
        block->type = FUNCTION_BLOCK;
        block->parent.function.function = fd;
    }
    add_function_to_compiler(fd);

    return fd;
}

void
dkc_function_define(TypeSpecifier *type, char *identifier,
                    ParameterList *parameter_list, Block *block)
{
    FunctionDefinition *fd;

    if (dkc_search_function(identifier)
        || dkc_search_declaration(identifier, NULL)) {
        dkc_compile_error(dkc_get_current_compiler()->current_line_number,
                          FUNCTION_MULTIPLE_DEFINE_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", identifier,
                          MESSAGE_ARGUMENT_END);
        return;
    }
    fd = dkc_create_function_definition(type, identifier, parameter_list,
                                        block);
}

ParameterList *
dkc_create_parameter(TypeSpecifier *type, char *identifier)
{
    ParameterList       *p;

    p = dkc_malloc(sizeof(ParameterList));
    p->name = identifier;
    p->type = type;
    p->line_number = dkc_get_current_compiler()->current_line_number;
    p->next = NULL;

    return p;
}

ParameterList *
dkc_chain_parameter(ParameterList *list, TypeSpecifier *type,
                    char *identifier)
{
    ParameterList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = dkc_create_parameter(type, identifier);

    return list;
}

ArgumentList *
dkc_create_argument_list(Expression *expression)
{
    ArgumentList *al;

    al = dkc_malloc(sizeof(ArgumentList));
    al->expression = expression;
    al->next = NULL;

    return al;
}

ArgumentList *
dkc_chain_argument_list(ArgumentList *list, Expression *expr)
{
    ArgumentList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = dkc_create_argument_list(expr);

    return list;
}

ExpressionList *
dkc_create_expression_list(Expression *expression)
{
    ExpressionList *el;

    el = dkc_malloc(sizeof(ExpressionList));
    el->expression = expression;
    el->next = NULL;

    return el;
}

ExpressionList *
dkc_chain_expression_list(ExpressionList *list, Expression *expr)
{
    ExpressionList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = dkc_create_expression_list(expr);

    return list;
}

StatementList *
dkc_create_statement_list(Statement *statement)
{
    StatementList *sl;

    sl = dkc_malloc(sizeof(StatementList));
    sl->statement = statement;
    sl->next = NULL;

    return sl;
}

StatementList *
dkc_chain_statement_list(StatementList *list, Statement *statement)
{
    StatementList *pos;

    if (list == NULL)
        return dkc_create_statement_list(statement);

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = dkc_create_statement_list(statement);

    return list;
}


TypeSpecifier *
dkc_create_type_specifier(DVM_BasicType basic_type)
{
    TypeSpecifier *type;

    type = dkc_alloc_type_specifier(basic_type);
    type->line_number = dkc_get_current_compiler()->current_line_number;

    return type;
}

TypeSpecifier *
dkc_create_class_type_specifier(char *identifier)
{
    TypeSpecifier *type;

    type = dkc_alloc_type_specifier(DVM_CLASS_TYPE);
    type->class_ref.identifier = identifier;
    type->class_ref.class_definition = NULL;
    type->line_number = dkc_get_current_compiler()->current_line_number;

    return type;
}

TypeSpecifier *
dkc_create_array_type_specifier(TypeSpecifier *base)
{
    TypeDerive *new_derive;
    
    new_derive = dkc_alloc_type_derive(ARRAY_DERIVE);

    if (base->derive == NULL) {
        base->derive = new_derive;
    } else {
        TypeDerive *derive_p;
        for (derive_p = base->derive; derive_p->next != NULL;
             derive_p = derive_p->next)
            ;
        derive_p->next = new_derive;
    }

    return base;
}

Expression *
dkc_alloc_expression(ExpressionKind kind)
{
    Expression  *exp;

    exp = dkc_malloc(sizeof(Expression));
    exp->type = NULL;
    exp->kind = kind;
    exp->line_number = dkc_get_current_compiler()->current_line_number;

    return exp;
}

Expression *
dkc_create_comma_expression(Expression *left, Expression *right)
{
    Expression *exp;

    exp = dkc_alloc_expression(COMMA_EXPRESSION);
    exp->u.comma.left = left;
    exp->u.comma.right = right;

    return exp;
}

Expression *
dkc_create_assign_expression(Expression *left, AssignmentOperator operator,
                             Expression *operand)
{
    Expression *exp;

    exp = dkc_alloc_expression(ASSIGN_EXPRESSION);
    exp->u.assign_expression.left = left;
    exp->u.assign_expression.operator = operator;
    exp->u.assign_expression.operand = operand;

    return exp;
}

Expression *
dkc_create_binary_expression(ExpressionKind operator,
                             Expression *left, Expression *right)
{
    Expression *exp;
    exp = dkc_alloc_expression(operator);
    exp->u.binary_expression.left = left;
    exp->u.binary_expression.right = right;
    return exp;
}

Expression *
dkc_create_minus_expression(Expression *operand)
{
    Expression  *exp;
    exp = dkc_alloc_expression(MINUS_EXPRESSION);
    exp->u.minus_expression = operand;
    return exp;
}

Expression *
dkc_create_logical_not_expression(Expression *operand)
{
    Expression  *exp;

    exp = dkc_alloc_expression(LOGICAL_NOT_EXPRESSION);
    exp->u.logical_not = operand;

    return exp;
}

Expression *
dkc_create_index_expression(Expression *array, Expression *index)
{
    Expression *exp;

    exp = dkc_alloc_expression(INDEX_EXPRESSION);
    exp->u.index_expression.array = array;
    exp->u.index_expression.index = index;

    return exp;
}

Expression *
dkc_create_incdec_expression(Expression *operand, ExpressionKind inc_or_dec)
{
    Expression *exp;

    exp = dkc_alloc_expression(inc_or_dec);
    exp->u.inc_dec.operand = operand;

    return exp;
}

Expression *
dkc_create_instanceof_expression(Expression *operand, TypeSpecifier *type)
{
    Expression *exp;

    exp = dkc_alloc_expression(INSTANCEOF_EXPRESSION);
    exp->u.instanceof.operand = operand;
    exp->u.instanceof.type = type;

    return exp;
}

Expression *
dkc_create_identifier_expression(char *identifier)
{
    Expression  *exp;

    exp = dkc_alloc_expression(IDENTIFIER_EXPRESSION);
    exp->u.identifier.name = identifier;

    return exp;
}

Expression *
dkc_create_function_call_expression(Expression *function,
                                    ArgumentList *argument)
{
    Expression  *exp;

    exp = dkc_alloc_expression(FUNCTION_CALL_EXPRESSION);
    exp->u.function_call_expression.function = function;
    exp->u.function_call_expression.argument = argument;

    return exp;
}

Expression *
dkc_create_down_cast_expression(Expression *operand, TypeSpecifier *type)
{
    Expression  *exp;

    exp = dkc_alloc_expression(DOWN_CAST_EXPRESSION);
    exp->u.down_cast.operand = operand;
    exp->u.down_cast.type = type;

    return exp;
}

Expression *
dkc_create_member_expression(Expression *expression, char *member_name)
{
    Expression  *exp;

    exp = dkc_alloc_expression(MEMBER_EXPRESSION);
    exp->u.member_expression.expression = expression;
    exp->u.member_expression.member_name = member_name;

    return exp;
}


Expression *
dkc_create_boolean_expression(DVM_Boolean value)
{
    Expression *exp;

    exp = dkc_alloc_expression(BOOLEAN_EXPRESSION);
    exp->u.boolean_value = value;

    return exp;
}

Expression *
dkc_create_null_expression(void)
{
    Expression  *exp;

    exp = dkc_alloc_expression(NULL_EXPRESSION);

    return exp;
}

Expression *
dkc_create_this_expression(void)
{
    Expression  *exp;

    exp = dkc_alloc_expression(THIS_EXPRESSION);

    return exp;
}

Expression *
dkc_create_super_expression(void)
{
    Expression  *exp;

    exp = dkc_alloc_expression(SUPER_EXPRESSION);

    return exp;
}

Expression *
dkc_create_new_expression(char *class_name, char *method_name,
                          ArgumentList *argument)
{
    Expression *exp;

    exp = dkc_alloc_expression(NEW_EXPRESSION);
    exp->u.new_e.class_name = class_name;
    exp->u.new_e.class_definition = NULL;
    exp->u.new_e.method_name = method_name;
    exp->u.new_e.method_declaration = NULL;
    exp->u.new_e.argument = argument;

    return exp;
}

Expression *
dkc_create_array_literal_expression(ExpressionList *list)
{
    Expression  *exp;

    exp = dkc_alloc_expression(ARRAY_LITERAL_EXPRESSION);
    exp->u.array_literal = list;

    return exp;
}

Expression *
dkc_create_basic_array_creation(DVM_BasicType basic_type,
                                ArrayDimension *dim_expr_list,
                                ArrayDimension *dim_list)
{
    Expression  *exp;
    TypeSpecifier *type;

    type = dkc_create_type_specifier(basic_type);
    exp = dkc_create_class_array_creation(type, dim_expr_list, dim_list);

    return exp;
}

Expression *
dkc_create_class_array_creation(TypeSpecifier *type,
                                ArrayDimension *dim_expr_list,
                                ArrayDimension *dim_list)
{
    Expression  *exp;

    exp = dkc_alloc_expression(ARRAY_CREATION_EXPRESSION);
    exp->u.array_creation.type = type;
    exp->u.array_creation.dimension
        = dkc_chain_array_dimension(dim_expr_list, dim_list);

    return exp;
}

ArrayDimension *
dkc_create_array_dimension(Expression *expr)
{
    ArrayDimension *dim;

    dim = dkc_malloc(sizeof(ArrayDimension));
    dim->expression = expr;
    dim->next = NULL;

    return dim;
}

ArrayDimension *
dkc_chain_array_dimension(ArrayDimension *list, ArrayDimension *dim)
{
    ArrayDimension *pos;

    for (pos = list; pos->next != NULL; pos = pos->next)
        ;
    pos->next = dim;

    return list;
}

Statement *
dkc_alloc_statement(StatementType type)
{
    Statement *st;

    st = dkc_malloc(sizeof(Statement));
    st->type = type;
    st->line_number = dkc_get_current_compiler()->current_line_number;

    return st;
}

Statement *
dkc_create_if_statement(Expression *condition,
                        Block *then_block, Elsif *elsif_list,
                        Block *else_block)
{
    Statement *st;

    st = dkc_alloc_statement(IF_STATEMENT);
    st->u.if_s.condition = condition;
    st->u.if_s.then_block = then_block;
    st->u.if_s.elsif_list = elsif_list;
    st->u.if_s.else_block = else_block;

    return st;
}

Elsif *
dkc_chain_elsif_list(Elsif *list, Elsif *add)
{
    Elsif *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = add;

    return list;
}

Elsif *
dkc_create_elsif(Expression *expr, Block *block)
{
    Elsif *ei;

    ei = dkc_malloc(sizeof(Elsif));
    ei->condition = expr;
    ei->block = block;
    ei->next = NULL;

    return ei;
}

Statement *
dkc_create_while_statement(char *label,
                           Expression *condition, Block *block)
{
    Statement *st;

    st = dkc_alloc_statement(WHILE_STATEMENT);
    st->u.while_s.label = label;
    st->u.while_s.condition = condition;
    st->u.while_s.block = block;
    block->type = WHILE_STATEMENT_BLOCK;
    block->parent.statement.statement = st;

    return st;
}

Statement *
dkc_create_for_statement(char *label, Expression *init, Expression *cond,
                         Expression *post, Block *block)
{
    Statement *st;

    st = dkc_alloc_statement(FOR_STATEMENT);
    st->u.for_s.label = label;
    st->u.for_s.init = init;
    st->u.for_s.condition = cond;
    st->u.for_s.post = post;
    st->u.for_s.block = block;
    block->type = FOR_STATEMENT_BLOCK;
    block->parent.statement.statement = st;

    return st;
}

Statement *
dkc_create_do_while_statement(char *label, Block *block,
                              Expression *condition)
{
    Statement *st;

    st = dkc_alloc_statement(DO_WHILE_STATEMENT);
    st->u.do_while_s.label = label;
    st->u.do_while_s.block = block;
    st->u.do_while_s.condition = condition;
    block->type = DO_WHILE_STATEMENT_BLOCK;
    block->parent.statement.statement = st;

    return st;
}

Statement *
dkc_create_foreach_statement(char *label, char *variable,
                             Expression *collection, Block *block)
{
    Statement *st;

    st = dkc_alloc_statement(FOREACH_STATEMENT);
    st->u.foreach_s.label = label;
    st->u.foreach_s.variable = variable;
    st->u.foreach_s.collection = collection;
    st->u.for_s.block = block;

    return st;
}

Block *
dkc_alloc_block(void)
{
    Block *new_block;

    new_block = dkc_malloc(sizeof(Block));
    new_block->type = UNDEFINED_BLOCK;
    new_block->outer_block = NULL;
    new_block->statement_list = NULL;
    new_block->declaration_list = NULL;

    return new_block;
}

Block *
dkc_open_block(void)
{
    Block *new_block;

    DKC_Compiler *compiler = dkc_get_current_compiler();
    new_block = dkc_alloc_block();
    new_block->outer_block = compiler->current_block;
    compiler->current_block = new_block;

    return new_block;
}

Block *
dkc_close_block(Block *block, StatementList *statement_list)
{
    DKC_Compiler *compiler = dkc_get_current_compiler();

    DBG_assert(block == compiler->current_block,
               ("block mismatch.\n"));
    block->statement_list = statement_list;
    compiler->current_block = block->outer_block;

    return block;
}

Statement *
dkc_create_expression_statement(Expression *expression)
{
    Statement *st;

    st = dkc_alloc_statement(EXPRESSION_STATEMENT);
    st->u.expression_s = expression;

    return st;
}

Statement *
dkc_create_return_statement(Expression *expression)
{
    Statement *st;

    st = dkc_alloc_statement(RETURN_STATEMENT);
    st->u.return_s.return_value = expression;

    return st;
}

Statement *
dkc_create_break_statement(char *label)
{
    Statement *st;

    st = dkc_alloc_statement(BREAK_STATEMENT);
    st->u.break_s.label = label;

    return st;
}

Statement *
dkc_create_continue_statement(char *label)
{
    Statement *st;

    st = dkc_alloc_statement(CONTINUE_STATEMENT);
    st->u.continue_s.label = label;

    return st;
}

Statement *
dkc_create_declaration_statement(TypeSpecifier *type,
                                 char *identifier,
                                 Expression *initializer)
{
    Statement *st;
    Declaration *decl;

    st = dkc_alloc_statement(DECLARATION_STATEMENT);

    decl = dkc_alloc_declaration(type, identifier);

    decl->initializer = initializer;

    st->u.declaration_s = decl;

    return st;
}

DVM_AccessModifier
conv_access_modifier(ClassOrMemberModifierKind src)
{
    if (src == PUBLIC_MODIFIER) {
        return DVM_PUBLIC_ACCESS;
    } else if (src == PRIVATE_MODIFIER) {
        return DVM_PRIVATE_ACCESS;
    } else {
        DBG_assert(src == NOT_SPECIFIED_MODIFIER, ("src..%d\n", src));
        return DVM_FILE_ACCESS;
    }
}

void
dkc_start_class_definition(ClassOrMemberModifierList *modifier,
                           DVM_ClassOrInterface class_or_interface,
                           char *identifier,
                           ExtendsList *extends)
{
    ClassDefinition *cd;
    DKC_Compiler *compiler = dkc_get_current_compiler();

    cd = dkc_malloc(sizeof(ClassDefinition));

    cd->is_abstract = (class_or_interface == DVM_INTERFACE_DEFINITION);
    cd->access_modifier = DVM_FILE_ACCESS;
    if (modifier) {
        if (modifier->is_abstract == ABSTRACT_MODIFIER) {
            cd->is_abstract = DVM_TRUE;
        }
        cd->access_modifier = conv_access_modifier(modifier->access_modifier);
    }
    cd->class_or_interface = class_or_interface;
    cd->package_name = compiler->package_name;
    cd->name = identifier;
    cd->extends = extends;
    cd->super_class = NULL;
    cd->interface_list = NULL;
    cd->member = NULL;
    cd->next = NULL;
    cd->line_number = compiler->current_line_number;

    DBG_assert(compiler->current_class_definition == NULL,
               ("current_class_definition is not NULL."));
    compiler->current_class_definition = cd;
}

void dkc_class_define(MemberDeclaration *member_list)
{
    DKC_Compiler *compiler;
    ClassDefinition *cd;
    ClassDefinition *pos;

    compiler = dkc_get_current_compiler();
    cd = compiler->current_class_definition;
    DBG_assert(cd != NULL, ("current_class_definition is NULL."));

    if (compiler->class_definition_list == NULL) {
        compiler->class_definition_list = cd;
    } else {
        for (pos = compiler->class_definition_list; pos->next;
             pos = pos->next)
            ;
        pos->next = cd;
    }
    cd->member = member_list;
    compiler->current_class_definition = NULL;
}

ExtendsList *
dkc_create_extends_list(char *identifier)
{
    ExtendsList *list;

    list = dkc_malloc(sizeof(ExtendsList));
    list->identifier = identifier;
    list->class_definition = NULL;
    list->next = NULL;

    return list;
}

ExtendsList *
dkc_chain_extends_list(ExtendsList *list, char *add)
{
    ExtendsList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = dkc_create_extends_list(add);

    return list;
}

ClassOrMemberModifierList
dkc_create_class_or_member_modifier(ClassOrMemberModifierKind modifier)
{
    ClassOrMemberModifierList ret;

    ret.is_abstract = NOT_SPECIFIED_MODIFIER;
    ret.access_modifier = NOT_SPECIFIED_MODIFIER;
    ret.is_override = NOT_SPECIFIED_MODIFIER;
    ret.is_virtual = NOT_SPECIFIED_MODIFIER;

    switch (modifier) {
    case ABSTRACT_MODIFIER:
        ret.is_abstract = ABSTRACT_MODIFIER;
        break;
    case PUBLIC_MODIFIER:
        ret.access_modifier = PUBLIC_MODIFIER;
        break;
    case PRIVATE_MODIFIER:
        ret.access_modifier = PRIVATE_MODIFIER;
        break;
    case OVERRIDE_MODIFIER:
        ret.is_override = OVERRIDE_MODIFIER;
        break;
    case VIRTUAL_MODIFIER:
        ret.is_virtual = VIRTUAL_MODIFIER;
        break;
    case NOT_SPECIFIED_MODIFIER: /* FALLTHRU */
    default:
        DBG_assert(0, ("modifier..%d", modifier));
    }

    return ret;
}

ClassOrMemberModifierList
dkc_chain_class_or_member_modifier(ClassOrMemberModifierList list,
                                   ClassOrMemberModifierList add)
{
    if (add.is_abstract != NOT_SPECIFIED_MODIFIER) {
        DBG_assert(add.is_abstract == ABSTRACT_MODIFIER,
                   ("add.is_abstract..%d", add.is_abstract));
        if (list.is_abstract != NOT_SPECIFIED_MODIFIER) {
            dkc_compile_error(dkc_get_current_compiler()->current_line_number,
                              ABSTRACT_MULTIPLE_SPECIFIED_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        list.is_abstract = ABSTRACT_MODIFIER;

    } else if (add.access_modifier != NOT_SPECIFIED_MODIFIER) {
        DBG_assert(add.access_modifier == PUBLIC_MODIFIER
                   || add.access_modifier == PUBLIC_MODIFIER,
                   ("add.access_modifier..%d", add.access_modifier));
        if (list.access_modifier != NOT_SPECIFIED_MODIFIER) {
            dkc_compile_error(dkc_get_current_compiler()->current_line_number,
                              ACCESS_MODIFIER_MULTIPLE_SPECIFIED_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        list.access_modifier = add.access_modifier;

    } else if (add.is_override != NOT_SPECIFIED_MODIFIER) {
        DBG_assert(add.is_override == OVERRIDE_MODIFIER,
                   ("add.is_override..%d", add.is_override));
        if (list.is_override != NOT_SPECIFIED_MODIFIER) {
            dkc_compile_error(dkc_get_current_compiler()->current_line_number,
                              OVERRIDE_MODIFIER_MULTIPLE_SPECIFIED_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        list.is_override = add.is_override;
    } else if (add.is_virtual != NOT_SPECIFIED_MODIFIER) {
        DBG_assert(add.is_virtual == VIRTUAL_MODIFIER,
                   ("add.is_virtual..%d", add.is_virtual));
        if (list.is_virtual != NOT_SPECIFIED_MODIFIER) {
            dkc_compile_error(dkc_get_current_compiler()->current_line_number,
                              VIRTUAL_MODIFIER_MULTIPLE_SPECIFIED_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        list.is_virtual = add.is_virtual;
    }
    return list;
}

MemberDeclaration *
dkc_chain_member_declaration(MemberDeclaration *list, MemberDeclaration *add)
{
    MemberDeclaration *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = add;

    return list;
}

static MemberDeclaration *
alloc_member_declaration(MemberKind kind,
                         ClassOrMemberModifierList *modifier)
{
    MemberDeclaration *ret;

    ret = dkc_malloc(sizeof(MemberDeclaration));
    ret->kind = kind;
    if (modifier) {
        ret->access_modifier = conv_access_modifier(modifier->access_modifier);
    } else {
        ret->access_modifier = DVM_FILE_ACCESS;
    }
    ret->line_number = dkc_get_current_compiler()->current_line_number;
    ret->next = NULL;

    return ret;
}

MemberDeclaration *
dkc_create_method_member(ClassOrMemberModifierList *modifier,
                         FunctionDefinition *function_definition,
                         DVM_Boolean is_constructor)
{
    MemberDeclaration *ret;
    DKC_Compiler *compiler;

    ret = alloc_member_declaration(METHOD_MEMBER, modifier);
    ret->u.method.is_constructor = is_constructor;
    ret->u.method.is_abstract = DVM_FALSE;
    ret->u.method.is_virtual = DVM_FALSE;
    ret->u.method.is_override = DVM_FALSE;
    if (modifier) {
        if (modifier->is_abstract == ABSTRACT_MODIFIER) {
            ret->u.method.is_abstract = DVM_TRUE;
        }
        if (modifier->is_virtual == VIRTUAL_MODIFIER) {
            ret->u.method.is_virtual = DVM_TRUE;
        }
        if (modifier->is_override == OVERRIDE_MODIFIER) {
            ret->u.method.is_override = DVM_TRUE;
        }
    }
    compiler = dkc_get_current_compiler();
    if (compiler->current_class_definition->class_or_interface
        == DVM_INTERFACE_DEFINITION) {
        /* BUGBUG error check */
        ret->u.method.is_abstract = DVM_TRUE;
        ret->u.method.is_virtual = DVM_TRUE;
    }

    ret->u.method.function_definition = function_definition;

    if (ret->u.method.is_abstract) {
        if (function_definition->block) {
            dkc_compile_error(compiler->current_line_number,
                              ABSTRACT_METHOD_HAS_BODY_ERR,
                              MESSAGE_ARGUMENT_END);
        }
    } else {
        if (function_definition->block == NULL) {
            dkc_compile_error(compiler->current_line_number,
                              CONCRETE_METHOD_HAS_NO_BODY_ERR,
                              MESSAGE_ARGUMENT_END);
        }
    }
    function_definition->class_definition
        = compiler->current_class_definition;

    return ret;
}

FunctionDefinition *
dkc_method_function_define(TypeSpecifier *type, char *identifier,
                           ParameterList *parameter_list,
                           Block *block)
{
    FunctionDefinition *fd;

    fd = dkc_create_function_definition(type, identifier, parameter_list,
                                        block);

    return fd;
}

FunctionDefinition *
dkc_constructor_function_define(char *identifier,
                                ParameterList *parameter_list,
                                Block *block)
{
    FunctionDefinition *fd;
    TypeSpecifier *type;

    type = dkc_create_type_specifier(DVM_VOID_TYPE);
    fd = dkc_method_function_define(type, identifier, parameter_list,
                                    block);

    return fd;
}

MemberDeclaration *
dkc_create_field_member(ClassOrMemberModifierList *modifier,
                        TypeSpecifier *type, char *name)
{
    MemberDeclaration *ret;

    ret = alloc_member_declaration(FIELD_MEMBER, modifier);
    ret->u.field.name = name;
    ret->u.field.type = type;

    return ret;
}
