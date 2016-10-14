#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static CRB_FunctionDefinition *
create_function_definition(char *identifier, CRB_ParameterList *parameter_list,
                           CRB_Boolean is_closure, CRB_Block *block)
{
    CRB_FunctionDefinition *f;

    f = crb_malloc(sizeof(CRB_FunctionDefinition));
    f->name = identifier;
    f->type = CRB_CROWBAR_FUNCTION_DEFINITION;
    f->is_closure = is_closure;
    f->u.crowbar_f.parameter = parameter_list;
    f->u.crowbar_f.block = block;

    return f;
}

void
crb_function_define(char *identifier, CRB_ParameterList *parameter_list,
                    CRB_Block *block)
{
    CRB_FunctionDefinition *f;
    CRB_Interpreter *inter;

    if (crb_search_function_in_compile(identifier)) {
        crb_compile_error(FUNCTION_MULTIPLE_DEFINE_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "name", identifier,
                          CRB_MESSAGE_ARGUMENT_END);
        return;
    }
    f = create_function_definition(identifier, parameter_list, CRB_FALSE,
                                   block);
    inter = crb_get_current_interpreter();
    f->next = inter->function_list;
    inter->function_list = f;
}

CRB_ParameterList *
crb_create_parameter(char *identifier)
{
    CRB_ParameterList   *p;

    p = crb_malloc(sizeof(CRB_ParameterList));
    p->name = identifier;
    p->next = NULL;

    return p;
}

CRB_ParameterList *
crb_chain_parameter(CRB_ParameterList *list, char *identifier)
{
    CRB_ParameterList *pos;

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

ExpressionList *
crb_create_expression_list(Expression *expression)
{
    ExpressionList *el;

    el = crb_malloc(sizeof(ExpressionList));
    el->expression = expression;
    el->next = NULL;

    return el;
}

ExpressionList *
crb_chain_expression_list(ExpressionList *list, Expression *expr)
{
    ExpressionList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = crb_create_expression_list(expr);

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
crb_create_comma_expression(Expression *left, Expression *right)
{
    Expression *exp;

    exp = crb_alloc_expression(COMMA_EXPRESSION);
    exp->u.comma.left = left;
    exp->u.comma.right = right;

    return exp;
}

Expression *
crb_create_assign_expression(CRB_Boolean is_final,
                             Expression *left, AssignmentOperator operator,
                             Expression *operand)
{
    Expression *exp;

    exp = crb_alloc_expression(ASSIGN_EXPRESSION);
    if (is_final) {
        if (left->type == INDEX_EXPRESSION) {
            crb_compile_error(ARRAY_ELEMENT_CAN_NOT_BE_FINAL_ERR,
                              CRB_MESSAGE_ARGUMENT_END);
        } else if (operator != NORMAL_ASSIGN) {
            crb_compile_error(COMPLEX_ASSIGNMENT_OPERATOR_TO_FINAL_ERR,
                              CRB_MESSAGE_ARGUMENT_END);
        }
    }
    exp->u.assign_expression.is_final = is_final;
    exp->u.assign_expression.left = left;
    exp->u.assign_expression.operator = operator;
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
    } else if (v->type == CRB_BOOLEAN_VALUE) {
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
crb_create_logical_not_expression(Expression *operand)
{
    Expression  *exp;

    exp = crb_alloc_expression(LOGICAL_NOT_EXPRESSION);
    exp->u.logical_not = operand;

    return exp;
}

Expression *
crb_create_index_expression(Expression *array, Expression *index)
{
    Expression *exp;

    exp = crb_alloc_expression(INDEX_EXPRESSION);
    exp->u.index_expression.array = array;
    exp->u.index_expression.index = index;

    return exp;
}

Expression *
crb_create_incdec_expression(Expression *operand, ExpressionType inc_or_dec)
{
    Expression *exp;

    exp = crb_alloc_expression(inc_or_dec);
    exp->u.inc_dec.operand = operand;

    return exp;
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
crb_create_function_call_expression(Expression *function,
                                    ArgumentList *argument)
{
    Expression  *exp;

    exp = crb_alloc_expression(FUNCTION_CALL_EXPRESSION);
    exp->u.function_call_expression.function = function;
    exp->u.function_call_expression.argument = argument;

    return exp;
}

Expression *
crb_create_member_expression(Expression *expression, char *member_name)
{
    Expression  *exp;

    exp = crb_alloc_expression(MEMBER_EXPRESSION);
    exp->u.member_expression.expression = expression;
    exp->u.member_expression.member_name = member_name;

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

Expression *
crb_create_array_expression(ExpressionList *list)
{
    Expression  *exp;

    exp = crb_alloc_expression(ARRAY_EXPRESSION);
    exp->u.array_literal = list;

    return exp;
}

Expression *
crb_create_closure_definition(char *identifier,
                              CRB_ParameterList *parameter_list,
                              CRB_Block *block)
{
    Expression  *exp;

    exp = crb_alloc_expression(CLOSURE_EXPRESSION);
    exp->u.closure.function_definition
        = create_function_definition(identifier, parameter_list, CRB_TRUE,
                                     block);
    
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
                        CRB_Block *then_block, Elsif *elsif_list,
                        CRB_Block *else_block)
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
crb_create_elsif(Expression *expr, CRB_Block *block)
{
    Elsif *ei;

    ei = crb_malloc(sizeof(Elsif));
    ei->condition = expr;
    ei->block = block;
    ei->next = NULL;

    return ei;
}

Statement *
crb_create_while_statement(char *label,
                           Expression *condition, CRB_Block *block)
{
    Statement *st;

    st = alloc_statement(WHILE_STATEMENT);
    st->u.while_s.label = label;
    st->u.while_s.condition = condition;
    st->u.while_s.block = block;

    return st;
}

Statement *
crb_create_for_statement(char *label, Expression *init, Expression *cond,
                         Expression *post, CRB_Block *block)
{
    Statement *st;

    st = alloc_statement(FOR_STATEMENT);
    st->u.for_s.label = label;
    st->u.for_s.init = init;
    st->u.for_s.condition = cond;
    st->u.for_s.post = post;
    st->u.for_s.block = block;

    return st;
}

Statement *
crb_create_foreach_statement(char *label, char *variable,
                             Expression *collection, CRB_Block *block)
{
    Statement *st;

    st = alloc_statement(FOREACH_STATEMENT);
    st->u.foreach_s.label = label;
    st->u.foreach_s.variable = variable;
    st->u.foreach_s.collection = collection;
    st->u.for_s.block = block;

    return st;
}

CRB_Block *
crb_create_block(StatementList *statement_list)
{
    CRB_Block *block;

    block = crb_malloc(sizeof(CRB_Block));
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

Statement *
crb_create_break_statement(char *label)
{
    Statement *st;

    st = alloc_statement(BREAK_STATEMENT);
    st->u.break_s.label = label;

    return st;
}

Statement *
crb_create_continue_statement(char *label)
{
    Statement *st;

    st = alloc_statement(CONTINUE_STATEMENT);
    st->u.continue_s.label = label;

    return st;
}

Statement *
crb_create_try_statement(CRB_Block *try_block, char *exception,
                         CRB_Block *catch_block, CRB_Block *finally_block)
{
    Statement *st;

    st = alloc_statement(TRY_STATEMENT);
    st->u.try_s.try_block = try_block;
    st->u.try_s.catch_block = catch_block;
    st->u.try_s.exception = exception;
    st->u.try_s.finally_block = finally_block;

    return st;
}

Statement *
crb_create_throw_statement(Expression *expression)
{
    Statement *st;

    st = alloc_statement(THROW_STATEMENT);
    st->u.throw_s.exception = expression;

    return st;
}
