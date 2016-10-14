#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static StatementResult
execute_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                  Statement *statement);

static StatementResult
execute_expression_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                             Statement *statement)
{
    StatementResult result;
    CRB_Value v;

    result.type = NORMAL_STATEMENT_RESULT;

    v = crb_eval_expression(inter, env, statement->u.expression_s);

    return result;
}

static StatementResult
execute_global_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                         Statement *statement)
{
    IdentifierList *pos;
    StatementResult result;

    result.type = NORMAL_STATEMENT_RESULT;

    if (env == NULL) {
        crb_runtime_error(inter, env, statement->line_number,
                          GLOBAL_STATEMENT_IN_TOPLEVEL_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    for (pos = statement->u.global_s.identifier_list; pos; pos = pos->next) {
        GlobalVariableRef *ref_pos;
        GlobalVariableRef *new_ref;
        Variable *variable;
        for (ref_pos = env->global_variable; ref_pos;
             ref_pos = ref_pos->next) {
            if (!strcmp(ref_pos->name, pos->name))
                goto NEXT_IDENTIFIER;
        }
        variable = crb_search_global_variable(inter, pos->name);
        if (variable == NULL) {
            crb_runtime_error(inter, env, statement->line_number,
                              GLOBAL_VARIABLE_NOT_FOUND_ERR,
                              CRB_STRING_MESSAGE_ARGUMENT, "name", pos->name,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        new_ref = MEM_malloc(sizeof(GlobalVariableRef));
        new_ref->name = pos->name;
        new_ref->variable = variable;
        new_ref->next = env->global_variable;
        env->global_variable = new_ref;
      NEXT_IDENTIFIER:
        ;
    }

    return result;
}


static StatementResult
execute_elsif(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
              Elsif *elsif_list, CRB_Boolean *executed)
{
    StatementResult result;
    CRB_Value   cond;
    Elsif *pos;

    *executed = CRB_FALSE;
    result.type = NORMAL_STATEMENT_RESULT;
    for (pos = elsif_list; pos; pos = pos->next) {
        cond = crb_eval_expression(inter, env, pos->condition);
        if (cond.type != CRB_BOOLEAN_VALUE) {
            crb_runtime_error(inter, env, pos->condition->line_number,
                              NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        if (cond.u.boolean_value) {
            result = crb_execute_statement_list(inter, env,
                                                pos->block->statement_list);
            *executed = CRB_TRUE;
            if (result.type != NORMAL_STATEMENT_RESULT)
                goto FUNC_END;
        }
    }

  FUNC_END:
    return result;
}

static StatementResult
execute_if_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                     Statement *statement)
{
    StatementResult result;
    CRB_Value   cond;

    result.type = NORMAL_STATEMENT_RESULT;
    cond = crb_eval_expression(inter, env, statement->u.if_s.condition);
    if (cond.type != CRB_BOOLEAN_VALUE) {
        crb_runtime_error(inter, env, statement->u.if_s.condition->line_number,
                          NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    DBG_assert(cond.type == CRB_BOOLEAN_VALUE, ("cond.type..%d", cond.type));

    if (cond.u.boolean_value) {
        result = crb_execute_statement_list(inter, env,
                                            statement->u.if_s.then_block
                                            ->statement_list);
    } else {
        CRB_Boolean elsif_executed;
        result = execute_elsif(inter, env, statement->u.if_s.elsif_list,
                               &elsif_executed);
        if (result.type != NORMAL_STATEMENT_RESULT)
            goto FUNC_END;
        if (!elsif_executed && statement->u.if_s.else_block) {
            result = crb_execute_statement_list(inter, env,
                                                statement->u.if_s.else_block
                                                ->statement_list);
        }
    }

  FUNC_END:
    return result;
}

static StatementResultType
compare_labels(char *result_label, char *loop_label, 
               StatementResultType current_result)
{
    if (result_label == NULL)
        return NORMAL_STATEMENT_RESULT;

    if (loop_label && !strcmp(result_label, loop_label)) {
        return NORMAL_STATEMENT_RESULT;
    } else {
        return current_result;
    }
}

static StatementResult
execute_while_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                        Statement *statement)
{
    StatementResult result;
    CRB_Value   cond;

    result.type = NORMAL_STATEMENT_RESULT;
    for (;;) {
        cond = crb_eval_expression(inter, env, statement->u.while_s.condition);
        if (cond.type != CRB_BOOLEAN_VALUE) {
            crb_runtime_error(inter, env,
                              statement->u.while_s.condition->line_number,
                              NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        DBG_assert(cond.type == CRB_BOOLEAN_VALUE,
                   ("cond.type..%d", cond.type));
        if (!cond.u.boolean_value)
            break;

        result = crb_execute_statement_list(inter, env,
                                            statement->u.while_s.block
                                            ->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label,
                                         statement->u.while_s.label,
                                         result.type);
            break;
        } else if (result.type == CONTINUE_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label,
                                         statement->u.while_s.label,
                                         result.type);
        }
    }

    return result;
}

static StatementResult
execute_for_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                      Statement *statement)
{
    StatementResult result;
    CRB_Value   cond;

    result.type = NORMAL_STATEMENT_RESULT;

    if (statement->u.for_s.init) {
        crb_eval_expression(inter, env, statement->u.for_s.init);
    }
    for (;;) {
        if (statement->u.for_s.condition) {
            cond = crb_eval_expression(inter, env,
                                       statement->u.for_s.condition);
            if (cond.type != CRB_BOOLEAN_VALUE) {
                crb_runtime_error(inter, env,
                                  statement->u.for_s.condition->line_number,
                                  NOT_BOOLEAN_TYPE_ERR,
                                  CRB_MESSAGE_ARGUMENT_END);
            }
            DBG_assert(cond.type == CRB_BOOLEAN_VALUE,
                       ("cond.type..%d", cond.type));
            if (!cond.u.boolean_value)
                break;
        }
        result = crb_execute_statement_list(inter, env,
                                            statement->u.for_s.block
                                            ->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label,
                                         statement->u.for_s.label,
                                         result.type);
            break;
        } else if (result.type == CONTINUE_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label,
                                         statement->u.for_s.label,
                                         result.type);
        }

        if (statement->u.for_s.post) {
            crb_eval_expression(inter, env, statement->u.for_s.post);
        }
    }

    return result;
}

static CRB_Value *
assign_to_variable(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                   int line_number, char *name, CRB_Value *value)
{
    CRB_Value *ret;

    ret = crb_get_identifier_lvalue(inter, env, line_number, name);
    if (ret == NULL) {
        if (env != NULL) {
            ret = CRB_add_local_variable(inter, env, name, value, CRB_FALSE);
        } else {
            if (CRB_search_function(inter, name)) {
                crb_runtime_error(inter, env, line_number,
                                  FUNCTION_EXISTS_ERR,
                                  CRB_STRING_MESSAGE_ARGUMENT, "name",
                                  name,
                                  CRB_MESSAGE_ARGUMENT_END);
            }
            ret = CRB_add_global_variable(inter,  name, value, CRB_FALSE);
        }
    } else {
        *ret = *value;
    }

    return ret;
}

static StatementResult
execute_foreach_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          Statement *statement)
{
    StatementResult result;
    CRB_Value   *collection;
    CRB_Value   iterator;
    CRB_Value   *var;
    CRB_Value   is_done;
    int stack_count = 0;
    CRB_Value   temp;

    result.type = NORMAL_STATEMENT_RESULT;

    collection
        = crb_eval_and_peek_expression(inter, env,
                                       statement->u.foreach_s.collection);
    stack_count++;
    collection = CRB_peek_stack(inter, 0);
    
    iterator = CRB_call_method(inter, env, statement->line_number,
                               collection->u.object, ITERATOR_METHOD_NAME,
                               0, NULL);
    CRB_push_value(inter, &iterator);
    stack_count++;

    temp.type = CRB_NULL_VALUE;
    var = assign_to_variable(inter, env, statement->line_number,
                             statement->u.foreach_s.variable,
                             &temp);
    for (;;) {
        is_done = CRB_call_method(inter, env, statement->line_number,
                                  iterator.u.object, IS_DONE_METHOD_NAME,
                                  0, NULL);
        if (is_done.type != CRB_BOOLEAN_VALUE) {
                crb_runtime_error(inter, env,
                                  statement->line_number,
                                  NOT_BOOLEAN_TYPE_ERR,
                                  CRB_MESSAGE_ARGUMENT_END);
        }
        if (is_done.u.boolean_value)
            break;

        *var = CRB_call_method(inter, env, statement->line_number,
                               iterator.u.object, CURRENT_ITEM_METHOD_NAME,
                               0, NULL);

        result = crb_execute_statement_list(inter, env,
                                            statement->u.for_s.block
                                            ->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label,
                                         statement->u.for_s.label,
                                         result.type);
            break;
        } else if (result.type == CONTINUE_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label,
                                         statement->u.for_s.label,
                                         result.type);
        }

        CRB_call_method(inter, env, statement->line_number,
                        iterator.u.object, NEXT_METHOD_NAME,
                        0, NULL);
    }
    CRB_shrink_stack(inter, stack_count);

    return result;
}

static StatementResult
execute_return_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                         Statement *statement)
{
    StatementResult result;

    result.type = RETURN_STATEMENT_RESULT;
    if (statement->u.return_s.return_value) {
        result.u.return_value
            = crb_eval_expression(inter, env,
                                  statement->u.return_s.return_value);
    } else {
        result.u.return_value.type = CRB_NULL_VALUE;
    }

    return result;
}

static StatementResult
execute_break_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                        Statement *statement)
{
    StatementResult result;

    result.type = BREAK_STATEMENT_RESULT;
    result.u.label = statement->u.break_s.label;

    return result;
}

static StatementResult
execute_continue_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                           Statement *statement)
{
    StatementResult result;

    result.type = CONTINUE_STATEMENT_RESULT;
    result.u.label = statement->u.continue_s.label;

    return result;
}

static StatementResult
execute_try_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                      Statement *statement)
{
    StatementResult result;
    int stack_pointer_backup;
    RecoveryEnvironment env_backup;

    stack_pointer_backup = crb_get_stack_pointer(inter);
    env_backup = inter->current_recovery_environment;
    if (setjmp(inter->current_recovery_environment.environment) == 0) {
        result = crb_execute_statement_list(inter, env,
                                            statement->u.try_s.try_block
                                            ->statement_list);
    } else {
        crb_set_stack_pointer(inter, stack_pointer_backup);
        inter->current_recovery_environment = env_backup;

        if (statement->u.try_s.catch_block) {
            CRB_Value ex_value;

            ex_value = inter->current_exception;
            CRB_push_value(inter, &ex_value);
            inter->current_exception.type = CRB_NULL_VALUE;

            assign_to_variable(inter, env, statement->line_number,
                               statement->u.try_s.exception, &ex_value);

            result = crb_execute_statement_list(inter, env,
                                                statement->u.try_s.catch_block
                                                ->statement_list);
            CRB_shrink_stack(inter, 1);
        }
    }
    inter->current_recovery_environment = env_backup;
    if (statement->u.try_s.finally_block) {
        crb_execute_statement_list(inter, env,
                                   statement->u.try_s.finally_block
                                   ->statement_list);
    }
    if (!statement->u.try_s.catch_block
        && inter->current_exception.type != CRB_NULL_VALUE) {
        longjmp(env_backup.environment, LONGJMP_ARG);
    }

    return result;
}

static StatementResult
execute_throw_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                        Statement *statement)
{
    CRB_Value *ex_val;

    ex_val = crb_eval_and_peek_expression(inter, env,
                                          statement->u.throw_s.exception);
    inter->current_exception = *ex_val;

    CRB_shrink_stack(inter, 1);

    longjmp(inter->current_recovery_environment.environment, LONGJMP_ARG);
}

static StatementResult
execute_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                  Statement *statement)
{
    StatementResult result;

    result.type = NORMAL_STATEMENT_RESULT;

    switch (statement->type) {
    case EXPRESSION_STATEMENT:
        result = execute_expression_statement(inter, env, statement);
        break;
    case GLOBAL_STATEMENT:
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
    case FOREACH_STATEMENT:
        result = execute_foreach_statement(inter, env, statement);
        break;
    case RETURN_STATEMENT:
        result = execute_return_statement(inter, env, statement);
        break;
    case BREAK_STATEMENT:
        result = execute_break_statement(inter, env, statement);
        break;
    case CONTINUE_STATEMENT:
        result = execute_continue_statement(inter, env, statement);
        break;
    case TRY_STATEMENT:
        result = execute_try_statement(inter, env, statement);
        break;
    case THROW_STATEMENT:
        result = execute_throw_statement(inter, env, statement);
        break;
    case STATEMENT_TYPE_COUNT_PLUS_1:   /* FALLTHRU */
    default:
        DBG_assert(0, ("bad case...%d", statement->type));
    }

    return result;
}

StatementResult
crb_execute_statement_list(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                           StatementList *list)
{
    StatementList *pos;
    StatementResult result;

    result.type = NORMAL_STATEMENT_RESULT;
    for (pos = list; pos; pos = pos->next) {
        result = execute_statement(inter, env, pos->statement);
        if (result.type != NORMAL_STATEMENT_RESULT)
            goto FUNC_END;
    }

  FUNC_END:
    return result;
}
