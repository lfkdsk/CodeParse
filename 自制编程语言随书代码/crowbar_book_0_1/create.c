#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

void
crb_function_define(char *identifier, ParameterList *parameter_list,
                    Block *block)
{
    FunctionDefinition *f;
    CRB_Interpreter *inter;

    if (crb_search_function(identifier)) {
        crb_compile_error(FUNCTION_MULTIPLE_DEFINE_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", identifier,
                          MESSAGE_ARGUMENT_END);
        return;
    }
    inter = crb_get_current_interpreter();

    f = crb_malloc(sizeof(FunctionDefinition));
    f->name = identifier;
    f->type = CROWBAR_FUNCTION_DEFINITION;
    f->u.crowbar_f.parameter = parameter_list;
    f->u.crowbar_f.block = block;
    f->next = inter->function_list;
    inter->function_list = f;
}

ParameterList *
crb_create_parameter(char *identifier)
{
    ParameterList       *p;

    p = crb_malloc(sizeof(ParameterList));
    p->name = identifier;
    p->next = NULL;

    return p;
}

ParameterList *
crb_chain_parameter(ParameterList *list, char *identifier)
{
    ParameterList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = crb_create_parameter(identifier);

    return list;
}

ArgumentList *
crb_create_argument_list(Expression *expression)
{
    ArgumentList *al;

    al = crb_malloc(sizeof(ArgumentList));
    al->expression = expression;
    al->next = NULL;

    return al;
}

ArgumentList *
crb_chain_argument_list(ArgumentList *list, Expression *expr)
{
    ArgumentList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = crb_create_argument_list(expr);

    return list;
}

StatementList *
crb_create_statement_list(Statement *statement)
{
    StatementList *sl;

    sl = crb_malloc(sizeof(StatementList));
    sl->statement = statement;
    sl->next = NULL;

    return sl;
}

StatementList *
crb_chain_statement_list(StatementList *list, Statement *statement)
{
    StatementList *pos;

    if (list == NULL)
        return crb_create_statement_list(statement);

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = crb_create_statement_list(statement);

    return list;
}

Expression *
crb_alloc_expression(ExpressionType type)
{
    Expression  *exp;

    exp = crb_malloc(sizeof(Expression));
    exp->type = type;
    exp->line_number = crb_get_current_interpreter()->current_line_number;

    return exp;
}

Expression *
crb_create_assign_expression(char *variable, Expression *operand)
{
    Expression *exp;

    exp = crb_alloc_expression(ASSIGN_EXPRESSION);
    exp->u.assign_expression.variable = variable;
    exp->u.assign_expression.operand = operand;

    return exp;
}

static Expression
convert_value_to_expression(CRB_Value *v)
{
    Expression  expr;

    if (v->type == CRB_INT_VALUE) {
        expr.type = INT_EXPRESSION;
        expr.u.int_value = v->u.int_value;
    } else if (v->type == CRB_DOUBLE_VALUE) {
        expr.type = DOUBLE_EXPRESSION;
        expr.u.double_value = v->u.double_value;
    } else {
        DBG_assert(v->type == CRB_BOOLEAN_VALUE,
                   ("v->type..%d\n", v->type));
        expr.type = BOOLEAN_EXPRESSION;
        expr.u.boolean_value = v->u.boolean_value;
    }
    return expr;
}

Expression *
crb_create_binary_expression(ExpressionType operator,
                             Expression *left, Expression *right)
{
    if ((left->type == INT_EXPRESSION
         || left->type == DOUBLE_EXPRESSION)
        && (right->type == INT_EXPRESSION
            || right->type == DOUBLE_EXPRESSION)) {
        CRB_Value v;
        v = crb_eval_binary_expression(crb_get_current_interpreter(),
                                       NULL, operator, left, right);
        /* Overwriting left hand expression. */
        *left = convert_value_to_expression(&v);

        return left;
    } else {
        Expression *exp;
        exp = crb_alloc_expression(operator);
        exp->u.binary_expression.left = left;
        exp->u.binary_expression.right = right;
        return exp;
    }
}

Expression *
crb_create_minus_expression(Expression *operand)
{
    if (operand->type == INT_EXPRESSION
        || operand->type == DOUBLE_EXPRESSION) {
        CRB_Value       v;
        v = crb_eval_minus_expression(crb_get_current_interpreter(),
                                      NULL, operand);
        /* Notice! Overwriting operand expression. */
        *operand = convert_value_to_expression(&v);
        return operand;
    } else {
        Expression      *exp;
        exp = crb_alloc_expression(MINUS_EXPRESSION);
        exp->u.minus_expression = operand;
        return exp;
    }
}

Expression *
crb_create_identifier_expression(char *identifier)
{
    Expression  *exp;

    exp = crb_alloc_expression(IDENTIFIER_EXPRESSION);
    exp->u.identifier = identifier;

    return exp;
}

Expression *
crb_create_function_call_expression(char *func_name, ArgumentList *argument)
{
    Expression  *exp;

    exp = crb_alloc_expression(FUNCTION_CALL_EXPRESSION);
    exp->u.function_call_expression.identifier = func_name;
    exp->u.function_call_expression.argument = argument;

    return exp;
}

Expression *
crb_create_boolean_expression(CRB_Boolean value)
{
    Expression *exp;

    exp = crb_alloc_expression(BOOLEAN_EXPRESSION);
    exp->u.boolean_value = value;

    return exp;
}

Expression *
crb_create_null_expression(void)
{
    Expression  *exp;

    exp = crb_alloc_expression(NULL_EXPRESSION);

    return exp;
}

static Statement *
alloc_statement(StatementType type)
{
    Statement *st;

    st = crb_malloc(sizeof(Statement));
    st->type = type;
    st->line_number = crb_get_current_interpreter()->current_line_number;

    return st;
}

Statement *
crb_create_global_statement(IdentifierList *identifier_list)
{
    Statement *st;

    st = alloc_statement(GLOBAL_STATEMENT);
    st->u.global_s.identifier_list = identifier_list;

    return st;
}

IdentifierList *
crb_create_global_identifier(char *identifier)
{
    IdentifierList      *i_list;

    i_list = crb_malloc(sizeof(IdentifierList));
    i_list->name = identifier;
    i_list->next = NULL;

    return i_list;
}

IdentifierList *
crb_chain_identifier(IdentifierList *list, char *identifier)
{
    IdentifierList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = crb_create_global_identifier(identifier);

    return list;
}

Statement *
crb_create_if_statement(Expression *condition,
                        Block *then_block, Elsif *elsif_list,
                        Block *else_block)
{
    Statement *st;

    st = alloc_statement(IF_STATEMENT);
    st->u.if_s.condition = condition;
    st->u.if_s.then_block = then_block;
    st->u.if_s.elsif_list = elsif_list;
    st->u.if_s.else_block = else_block;

    return st;
}

Elsif *
crb_chain_elsif_list(Elsif *list, Elsif *add)
{
    Elsif *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = add;

    return list;
}

Elsif *
crb_create_elsif(Expression *expr, Block *block)
{
    Elsif *ei;

    ei = crb_malloc(sizeof(Elsif));
    ei->condition = expr;
    ei->block = block;
    ei->next = NULL;

    return ei;
}

Statement *
crb_create_while_statement(Expression *condition, Block *block)
{
    Statement *st;

    st = alloc_statement(WHILE_STATEMENT);
    st->u.while_s.condition = condition;
    st->u.while_s.block = block;

    return st;
}

Statement *
crb_create_for_statement(Expression *init, Expression *cond,
                         Expression *post, Block *block)
{
    Statement *st;

    st = alloc_statement(FOR_STATEMENT);
    st->u.for_s.init = init;
    st->u.for_s.condition = cond;
    st->u.for_s.post = post;
    st->u.for_s.block = block;

    return st;
}

Block *
crb_create_block(StatementList *statement_list)
{
    Block *block;

    block = crb_malloc(sizeof(Block));
    block->statement_list = statement_list;

    return block;
}

Statement *
crb_create_expression_statement(Expression *expression)
{
    Statement *st;

    st = alloc_statement(EXPRESSION_STATEMENT);
    st->u.expression_s = expression;

    return st;
}

Statement *
crb_create_return_statement(Expression *expression)
{
    Statement *st;

    st = alloc_statement(RETURN_STATEMENT);
    st->u.return_s.return_value = expression;

    return st;
}

Statement *crb_create_break_statement(void)
{
    return alloc_statement(BREAK_STATEMENT);
}

Statement *crb_create_continue_statement(void)
{
    return alloc_statement(CONTINUE_STATEMENT);
}

