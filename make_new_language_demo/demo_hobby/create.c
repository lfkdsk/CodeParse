#include "hobby.h"
#include "MEM.h"
#include "DBG.h"

/*
  hbb_function_define 函数定义
  char *identifier 函数名
  ParameterList *parameter_list 参数列表
  Block *block 代码块
  return;
*/
void hbb_function_define(char *identifier,
  ParameterList *parameter_list
  , Block *block){
    FunctionDefinition *func;
    HBB_InterPreter *inter;

    if (hbb_search_function(identifier)) {
        hbb_compile_error(FUNCTION_MULTIPLE_DEFINE_ERROR,
                          STRING_MESSAGE_ARGUMENT,"name",identifier
                          MESSAGE_ARGUMENT_END);
        return;
    }

    inter = hbb_get_current_interpreter();
    func = hbb_malloc(sizeof(FunctionDefinition));
    func->name = identifier;
    func->type = HOBBY_FUNCTION_DEFINITION;
    func->u.hobby_f.parameter = parameter_list;
    func->u.hobby_f.block = block;
    /* function list */
    func->next = inter.function_list;
    inter->function_list = func;
}

/*
  hbb_create_parameter 创建参数
  char *identifier 参数名
  return ParameterList* 参数列表
*/
ParameterList *hbb_create_parameter(char *identifier){
    ParameterList *parameterList;
    parameterList = hbb_malloc(sizeof(ParameterList));
    parameterList->name = identifier;
    parameterList->next = NULL;
    return parameterList;
}

/*
  hbb_chain_parameter 创建一个参数列表的指针
  ParameterList *list 参数列表
  char *identifier 参数名
  return *list    参数列表
*/
ParameterList *hbb_chain_parameter(ParameterList *list, char *identifier){
    ParameterList *pos;
    /* 尾指针 */
    for(pos = list; pos->next; pos = pos->next);
    pos->next = hbb_create_parameter(identifier);

    return list;
}

/*
  hbb_create_argument_list 创建表达式列表
  Expression* expression 表达式
  return ＊arg 表达式列表
*/
ArgumentList *hbb_create_argument_list(Expression *expression){
    ArgumentList *arg;

    arg = hbb_malloc(sizeof(ArgumentList));
    arg->expression = expression;
    arg->next = NULL;
    return arg;
}

/*
  hbb_chain_argument_list 创建表达式列表
  ArgumentList *list 之前的列表
  Expression *exp 表达式
  return *list 返回的表达式列表
 */
ArgumentList *hbb_chain_argument_list(ArgumentList *list, Expression *exp){
    ArgumentList *pos;
    for(pos = list; pos->next; pos = pos->next);
    pos->next = hbb_create_argument_list(exp);
    return list;
}

/*
  这东西类似声明块
  hbb_create_statement_list 创建声明列表
  Statement *statement 声明
  return statement_list 声明列表
*/
StatementList *hbb_create_statement_list(Statement *statement){
    StatementList *statement_list;

    statement_list = hbb_malloc(sizeof(Statement));
    statement_list->statement = statement;
    statement_list->next = NULL;
    return statement_list;
}

/*
  hbb_chain_statement_list 创建声明列表
  StatementList *list 前驱声明列表
  Statement *st 声明
  return list 返回声明列表
*/
StatementList *hbb_chain_statement_list(StatementList *list, Statement *st){
    StatementList *pos;

    if (list == NULL) {
        return hbb_create_statement_list(statement);
    }

    for(pos = list; pos->next; pos = pos->next);
    pos->next = hbb_create_statement_list(statement);
    return list;
}

/*
  hbb_alloc_expression 表达式开空间
  ExpressionType type 表达式类型
  return exp 表达式
*/
Expression *hbb_alloc_expression(ExpressionType type){
    Expression *exp;

    exp = hbb_malloc(sizeof(Expression));
    exp->type = type;
    exp->line_number = hbb_get_current_interpreter()->current_line_number;

    return exp;
}

/*
  hbb_create_assign_expression 赋值语句开空间
  char *variable 符号
  Expression *operand 表达式
  return exp 返回赋值语句
*/
Expression *hbb_create_assign_expression(char *variable,Expression *operand){
    Expression *exp;

    exp = hbb_alloc_expression(ASSIGN_EXPRESSION);
    exp->u.assign_expression.variable = variable;
    exp->u.assign_expression.operand = operand;

    return exp;
}

/*
  静态方法 convert_value_to_expression 值转换为表达式
  HBB_Value *value 传入的值
  return expr; 返回的表达式
*/
static Expression convert_value_to_expression(HBB_Value *value){
    Expression expr;
    if (value->type == HBB_INT_VALUE) {
        expr.type = INT_EXPRESSION;
        expr.u.int_value = value->u.int_value;
    }else if(value->type == HBB_DOUBLE_VALUE){
        expr.type = DOUBLE_EXPRESSION;
        expr.u.double_value = value->u.double_value;
    }else {
        /* boolean */
        DBG_assert(value->type == HBB_BOOLEAN_VALUE,
                  ("value->type.. %d\n", value->type));
        expr->type = BOOLEAN_EXPRESSION;
        expr->u.boolean_value = value->u.boolean_value;
    }
    return expr;
}

/*
  hbb_create_binary_expression 常量折叠
  ExpressionType type 表达式类型
  Expression *left 左表达式
  Expression *right 右表达式
  return exp; 返回表达式
*/
Expression *hbb_create_binary_expression(ExpressionType type,
                                      Expression *left,Expression *right){
    /* 如果左／右全是数字 开始常量折叠 */
    if ((left->type == INT_EXPRESSION || left->type == DOUBLE_EXPRESSION)
        && (right->type == INT_EXPRESSION || right->type == DOUBLE_EXPRESSION)) {
        HBB_Value value;
        value = hbb_eval_binary_expression(hbb_get_current_interpreter(),
                                          NULL, type, left, right);
        *left = convert_value_to_expression(&value);

        return left;
    } else {
        Expression *exp;
        exp = hbb_alloc_expression(type);
        exp->u.binary_expression.left = left;
        exp->u.binary_expression.right = right;
        return exp;
    }
}

/*
  hbb_create_minus_expression 处理负号
  Expression *operand 有负号的表达式
  return *exp;
*/
Expression *hbb_create_minus_expression(Expression *operand){
    if (operand->type == INT_EXPRESSION || operand->type == DOUBLE_EXPRESSION) {
        HBB_Value value;
        /* 转换 */
        value = hbb_eval_minus_expression(hbb_get_current_interpreter(),
                                          NULL, operand);
        /* 常量折叠 */
        *operand = convert_value_to_expression(&value);
        return operand;
    } else {
        Expression *exp;
        exp = hbb_alloc_expression(MINUS_EXPRESSION);
        exp->u.minus_expression = operand;
        return exp;
    }
}

/*
  hbb_create_identifier_expression 创建id
  char *identifier id
*/
Expression *hbb_create_identifier_expression(char *identifier){
    Expression *exp;
    exp = hbb_alloc_expression(IDENTIFIER_EXPRESSION);
    exp->u.identifier = identifier;

    return exp;
}

/*
  hbb_create_function_call_expresssion 创建function调用
  char *func_name 函数名
  ArgumentList *argument 参数列表
*/
Expression *hbb_create_function_call_expresssion(char *func_name,
                                              ArgumentList *argument){
    Expression *exp;

    exp = hbb_alloc_expression(FUNCTION_CALL_EXPRESSION);
    exp->u.function_call_expression.identifier = func_name;
    exp->u.function_call_expression.argument = argument;

    return exp;
}

/*
  hbb_create_boolean_expression 创建布尔表达式
  HBB_Boolean value boolean类型数据
*/
Expression *hbb_create_boolean_expression(HBB_Boolean value){
    Expression *exp;

    exp = hbb_alloc_expression(BOOLEAN_EXPRESSION);
    exp->u.boolean_value = value;

    return exp;
}

/*
  hbb_create_null_expression 创建null表达式
  void
*/
Expression *hbb_create_null_expression(void){
    Expression *exp;

    exp = hbb_alloc_expression(NULL_EXPRESSION);

    return exp;
}

/*
  alloc_statement 声明分配空间
  StatementType type 声明类型
*/
static Statement *alloc_statement(StatementType type){
    Statement *st;

    st = hbb_malloc(sizeof(Statement));

    st->type = type;
    st->line_number = hbb_get_current_interpreter()->current_line_number;

    return st;
}

/*
  hbb_create_global_statement 全局变量声明
  IdentifierList *identifierList id列表
*/
Statement *hbb_create_global_statement(IdentifierList *identifierList){
    Statement *st;

    st = alloc_statement(GLOBAL_STATEMENT);
    st->u.global_s.identifier_list = identifier_list;

    return st;
}

/*
  hbb_create_global_identifier 创建全局id列表
  char *identifier id
*/
IdentifierList *hbb_create_global_identifier(char *identifier){
    IdentifierList *id_list;

    id_list = hbb_malloc(sizeof(IdentifierList));
    id_list->name = identifier;
    id_list->next = NULL;

    return id_list;
}

/*
  hbb_chain_identifier 根据前驱列表创建id
  IdentifierList *list 前驱列表
  char *identifier id
*/
IdentifierList *hbb_chain_identifier(IdentifierList *list, char *identifier){
    IdentifierList *pos;

    for(pos = list; pos->next; pos = pos->next);

    pos->next = hbb_create_global_identifier(identifier);

    return list;
}

Statement *hbb_create_if_statement(Expression *condition, Block *then_block,
                                  ElseIf *elseif_list, Block *else_block){
    Statement *st;

    st = alloc_statement(IF_STATEMENT);
    st->u.if_s.condition = condition;
    st->u.if_s.then_block = then_block;
    st->u.if_s.elseif_list = elseif_list;
    st->u.if_s.else_block = 
}
