#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "hobby.h"


static StatementResult execute_statement(HBB_Interpreter *inter,
                                         LocalEnvironment *env.
                                         Statement *statement);

static StatementResult execute_expression_statement(HBB_Interpreter *inter,
                                                    LocalEnvironment *env,
                                                    Statement *statement){
        StatementResult result;

        HBB_Value = v;

        result.type = NORMAL_STATEMENT_RESULT;

        v = hbb_eval_expresison(inter, env, statement->u.expression_s);
        if (v.type == HBB_STRING_VALUE) {
                hbb_release_string(v.u.string_value);
        }

        return result;
}

/**
 * 运行时处理全局变量
 * @param  inter     编译器
 * @param  env       所处环境(这是个很差的翻译)
 * @param  statement 语句段
 * @return           返回
 */
static StatementResult execute_global_statement(HBB_Interpreter *inter,
                                                LocalEnvironment *env,
                                                Statement *statement){
        IdentifierList *pos;
        StatementResult result;

        result.type = NORMAL_STATEMENT_RESULT;

        if (env == NULL) {
                hbb_runtime_error(statement->line_number, GLOBAL_STATEMENT_IN_TOP_LEVEL,
                                  MESSAGE_ARGUMENT_END);
        }

        for (pos = statement->u.global_s.identifier_list; pos; pos = pos->next) {
                GlobalVariableRef *ref_pos;
                GlobalVariableRef *new_ref;

                Variable *variable;

                for(ref_pos = env->global_variable; ref_pos; ref_pos = ref_pos->next) {
                        if (!strcmp(ref_pos->variable->name, pos->name)) {
                                goto NEXT_IDENTIFIER;
                        }
                }

                variable = hbb_search_global_variable(inter, pos->name);

                if (variable == NULL) {
                        hbb_runtime_error(statement->line_number,
                                          GLOABL_VARIABLE_NOT_FOUND_ERROR, STRING_MESSAGE_ARGUMENT,
                                          "name", pos->name, MESSAGE_ARGUMENT_END);
                }

                new_ref = MEM_malloc(sizeof(GlobalVariableRef));
                new_ref->variable = variable;
                new_ref->next = env->global_variable;
                env->global_variable = new_ref;
NEXT_IDENTIFIER:
                ;
        }

        return;
}

/**
 * 运行elseif
 * @param  inter       [description]
 * @param  env         [description]
 * @param  elseif_list [description]
 * @param  executed    [description]
 * @return             [description]
 */
static StatementResult execute_elseif(HBB_Interpreter *inter,
                                      LocalEnvironment *env,
                                      ElseIf *elseif_list, HBB_Boolean *executed){
        StatementResult result;
        HBB_Value cond;
        ElseIf *pos;

        *executed = HBB_FAlSE;
        result.type = NORMAL_STATEMENT_RESULT;
        for(pos = elseif_list; pos; pos->pos->next) {
                cond = hbb_eval_expresison(inter, env, pos->condition);
                if (cond.type != HBB_BOOLEAN_VALUE) {
                        hbb_runtime_error(pos->condition->line_number,
                                          NOT_BOOLAEAN_TYOE_ERROR, MESSAGE_ARGUMENT_END);
                }

                if(cond.u.boolean_type) {
                        result = hbb_execute_statement_list(inter, env, pos->block->statement_list);

                        *executed = HBB_TRUE;
                        if (result.type != NORMAL_STATEMENT_RESULT) {
                                goto FUNC_END;
                        }
                }
        }

FUNC_END:
        return result;
}

/**
 * 处理基础的if
 * @param  inter     [description]
 * @param  env       [description]
 * @param  statement [description]
 * @return           [description]
 */
static StatementResult execute_if_statement(HBB_Interpreter *inter,
                                            LocalEnvironment *env,
                                            Statement *statement){
        StatementResult result;
        HBB_Value cond;

        result.type = NORMAL_STATEMENT_RESULT;
        // 求布尔表达式的值
        cond = hbb_eval_expression(inter, env, statement->u.if_s.condition);
        if (cond.type != HBB_BOOLEAN_VALUE) {
                hbb_runtime_error(statement->u.if_s.condition->line_number,NOT_BOOLAEAN_TYOE_ERROR,
                                  MESSAGE_ARGUMENT_END);
        }

        DBG_assert(cond.type == HBB_BOOLEAN_VALUE, ("cond.type..%d", cond.type));
        // 判断走那个分支
        if (cond.u.boolean_type) {
                result = hbb_execute_statement_list(inter, env,
                                                    statement->u.if_s.then_block->statement_list
                                                    );
        }else{
                HBB_Value elseif_executed;
                result = execute_elseif(inter, env, statement->u.if_s.elseif_list,
                                        &elseif_executed);

                if (result.type != NORMAL_STATEMENT_RESULT) {
                        goto FUNC_END;
                }

                if (!elseif_executed && statement->u.if_s.u.else_block) {
                        result = hbb_execute_statement_list(inter, env,
                                                            statement->u.if_s.else_block>statement_list);
                }
        }
FUNC_END:
        return result;
}

/**
 * 运行while结构
 * @param  inter     [description]
 * @param  env       [description]
 * @param  statement [description]
 * @return           [description]
 */
static StatementResult execute_while_statement(HBB_Interpreter *inter,
                                               LocalEnvironment *env,
                                               Statement *statement){
        StatementResult result;
        HBB_Value cond;

        result.type = NORMAL_STATEMENT_RESULT;
        for(;; ) {
                cond = hbb_eval_expression(inter, env, statement->u.while_s.condition);
                if (cond.type != HBB_BOOLEAN_VALUE) {
                        hbb_runtime_error(statement->u.while_s.condition->line_number,
                                          NOT_BOOLAEAN_TYOE_ERROR,MESSAGE_ARGUMENT_END);
                }

                DBG_assert(cond.type == HBB_BOOLEAN_VALUE,
                           ("cond.type.. %d",cond.type));

                if (!cond.u.boolean_type) {
                        break;
                }

                result = hbb_execute_statement_list(inter, env,
                                                    statement->u.while_s.block->statement_list);
                // return 这个标记符 应该在一个地方会被修改吧。。。
                // 应该是execute_expression_statement的时候
                if (result.type == RETURN_STATEMENT_RESULT) {
                        break;
                } else if (result.type == BREAK_STATEMENT_RESULT) {
                        result.type = NORMAL_STATEMENT_RESULT;
                        break;
                }
        }

        return result;
}

static StatementResult execute_for_statement(HBB_Interpreter *inter, LocalEnvironment *env,
                                             Statement *statement){
        StatementResult result;
        HBB_Value cond;

        result.type = NORMAL_STATEMENT_RESULT;

        if (statement->u.for_s.init) {
                hbb_eval_expression(inter, env, statement->u.for_s.init);
        }

        for(;; ) {
                if (statement->u.for_s.condition) {
                        cond = hbb_eval_expression(inter, env, statement->u.for_s.condition);

                        if (cond.type != HBB_BOOLEAN_VALUE) {
                                hbb_runtime_error(statement->u.for_s.condition->line_number,
                                                  NOT_BOOLAEAN_TYOE_ERROR, MESSAGE_ARGUMENT_END);
                        }

                        DBG_assert(cond.type == HBB_BOOLEAN_VALUE,
                                   ("cond.type..%d",cond.type));

                        if (!cond.u.boolean_type) {
                                break;
                        }
                }

                result = hbb_execute_statement_list(inter, env,
                                                    statement->u.for_s.block->statement_list);

                if (result.type == RETURN_STATEMENT_RESULT) {
                        break;
                }else if (result.type == BREAK_STATEMENT_RESULT) {
                        result.type = NORMAL_STATEMENT_RESULT;
                        break;
                }

                // post 运行
                if (statement->u.for_s.post) {
                        hbb_eval_expression(inter, env,statement->u.for_s.post);
                }
        }

        return result;
}

static StatementResult execute_return_statement(HBB_Interpreter *inter, LocalEnvironment *env,
                                                Statement *statement){
        StatementResult result;

        result.type = RETURN_STATEMENT_RESULT;

        if (statement->u.return_s.return_value) {
                result.u.return_value = hbb_eval_expression(inter, env, statement->u.return_s.value);
                result.u.return_value = hbb_eval_expression(inter, env, statement->u.return_s.return_value);

        }else{
                result.u.return_value.type = HBB_NULL_VALUE;
        }

        return result;
}

/**
 * break/continue 的处理其实就是修改返回值类型
 * 返回上级函数处理跳转
 * @param  inter     [description]
 * @param  env       [description]
 * @param  statement [description]
 * @return           [description]
 */
static StatementResult execute_break_statement(HBB_Interpreter *inter, LocalEnvironment *env,
                                               Statement *statement){
        StatementResult result;

        result.type = BREAK_STATEMENT_RESULT;

        return result;
}

static StatementResult execute_continue_statement(HBB_Interpreter *inter, LocalEnvironment *env,
                                                  Statement *statement){
        StatementResult result;

        result.type = CONTINUE_STATEMENT_RESULT;

        return result;
}


/**
 * 只是个分发
 * @param  inter     [description]
 * @param  env       [description]
 * @param  statement [description]
 * @return           [description]
 */
static StatementResult execute_statement(HBB_Interpreter *inter, LocalEnvironment *env,
                                         Statement *statement){
        StatementResult result;

        result.type = NORMAL_STATEMENT_RESULT;

        switch (statement->type) {
        case EXPRESSION_STATEMENT:
                result = execute_expression_statement(inter, env, statement);
                break;
        case GALOBAL_STATEMENT:
                result = execute_global_statement(inter, env, statement);
                break;
        case IF_STATEMENT:
                result = execute_if_statement(inter, env, statement);
                break;

        case WHILE_STATEMENT:
                result = execute_while_statement(inter, env, statement);
                break;

        case FOR_STATEMENT:
                result = execute_for_statement(inter, env, statement);
                break;

        case RETURN_STATEMENT_RESULT:
                result = execute_return_statement(inter, env, statement);
                break;
        case CONTINUE_STATEMENT_RESULT:
                result = execute_continue_statement(inter, env, statement);
                break;
        case BREAK_STATEMENT_RESULT:
                result = execute_break_statement(inter, env, statement);
                break;
        case STATEMENT_TYPE_COUNT_PLUS_1:
        default:
                DBG_assert(("bad case...%d", statement->type));
        }

        return result;
}

/**
 * 顺着链表处理
 * @param  inter [description]
 * @param  env   [description]
 * @param  list  [description]
 * @return       [description]
 */
StatementResult hbb_execute_statement_list(HBB_Interpreter *inter, LocalEnvironment *env,
                                           StatementList *list){

        StatementList *pos;

        StatementResult result;

        result.type = NORMAL_STATEMENT_RESULT;

        for(pos = list; pos; pos->pos->next) {
                result = execute_statement(inter, env, pos->statement);
                if (result.type != NORMAL_STATEMENT_RESULT) {
                        goto FUNC_END;
                }
        }

FUNC_END:
        return result;
}
