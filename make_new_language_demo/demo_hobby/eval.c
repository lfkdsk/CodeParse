#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "hobby.h"

static HBB_Value eval_boolean_expression(HBB_Boolean boolean_value){

        HBB_Value v;
        v.type = HBB_BOOLEAN_VALUE;
        v.u.boolean_value = boolean_value;

        return v;
}

static HBB_Value eval_int_expression(int int_value){

        HBB_Value v;
        v.type = HBB_INT_VALUE;
        v.u.int_value = int_value;

        return v;
}

static HBB_Value eval_double_expression(double double_value){

        HBB_Value v;

        v.type = HBB_DOUBLE_VALUE;
        v.u.double_value = double_value;

        return v;
}

static HBB_Value eval_string_expression(HBB_Interpreter *inter, char *string_value){

        HBB_Value v;
        v.type = HBB_STRING_VALUE;
        v.u.string_value = hbb_literal_to_hbb_string(inter, string_value);

        return v;
}

static HBB_Value eval_null_expression(void){
        HBB_Value v;

        v.type = HBB_NULL_VALUE;

        return v;
}

static void refer_if_string(HBB_Value *v){

        if (v->type == HBB_STRING_VALUE) {
                hbb_refer_string(v->u.string_value);
        }
}

static void release_if_string(HBB_Value *v){

        if (v->type == HBB_STRING_VALUE) {
                hbb_release_string(v->u.string_value);
        }
}

/**
 * 从字母表搜索全局变量
 * @param  inter [description]
 * @param  env   [description]
 * @param  name  [description]
 * @return       [description]
 */
static Variable *search_global_variable_from_env(HBB_Interpreter *inter, LocalEnvironment *env,
                                                 char *name){

        GlobalVariableRef *pos;

        if (env == NULL) {
                return hbb_search_global_variable(inter, name);
        }

        for(pos = env->global_variable; pos; pos = pos->next) {

                if (!strcmp(pos->variable->name, name)) {
                        return pos->variable;
                }
        }

        return NULL;
}

static HBB_Value eval_identifier_expression(HBB_Interpreter *inter, LocalEnvironment *env,
                                            Expression *expr){

        HBB_Value v;

        Variable *vp;

        vp = hbb_search_local_variable(env, expr->u.identifier);

        if (vp != NULL) {
                v = vp->value;
        } else {
                vp = search_global_variable_from_env(inter, env, expr->u.identifier);

                if (vp != NULL) {
                        v = vp->value;
                } else {
                        hbb_runtime_error(expr->line_number, VARIABLE_NOT_FOUND_ERROR,
                                          STRING_MESSAGE_ARGUMENT, "name", expr->u.identifier, MESSAGE_ARGUMENT_END);
                }
        }

        refer_if_string(&v);

        return v;
}

static HBB_Value eval_expression(HBB_Interpreter *inter, LocalEnvironment *env,
                                  char *identifier, Expression *expression);

static HBB_Value eval_assign_expression(HBB_Interpreter *inter, LocalEnvironment *env,
                                        char *identifier, Expression *expression){
        HBB_Value v;
        Variable *left;

        v = eval_expression(inter, env, expression);

        left = hbb_search_local_variable(env, identifier);

        if (left == NULL) {
                left = search_global_variable_from_env(inter, env, identifier);
        }

        if (left != NULL) {
                release_if_string(&left->value);
                left->value = v;
                refer_if_string(&v);
        } else {
                if (env != NULL) {
                        hbb_and_local_variable(env, identifier, &v);
                } else {
                        HBB_and_global_variable(inter, identifier, &v);
                }

                refer_if_string(&v);
        }
        return v;
}

/**
 * 运行布尔表达式
 * @param  inter       [description]
 * @param  operator    [description]
 * @param  left        [description]
 * @param  right       [description]
 * @param  line_number [description]
 * @return             [description]
 */
static HBB_Boolean eval_binary_boolean(HBB_Interpreter *inter, ExpressionType operator,
                                       HBB_Boolean left, HBB_Boolean right, int line_number){
        HBB_Boolean result;

        if (operator == EQ_EXPRESSION) {
                result = left == right;
        } else if (operator == NE_EXPRESSION) {
                result = left != right;
        } else {
                char *op_str = hbb_get_operator_string(operator);
                hbb_runtime_error(line_number, NOT_BOOLEAN_OPERATOR_ERROR,
                                  STRING_MESSAGE_ARGUMENT, "operator", op_str, MESSAGE_ARGUMENT_END);
        }

        return result;
}
