#include "hobby.h"
#include "MEM.h"
#include "DBG.h"

/*
  hbb_function_define 函数定义
  identifier 函数名
  parameter_list 参数列表
  block 代码块
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

ParameterList *hbb_create_parameter(char *identifier){
    ParameterList *parameterList;
    parameterList = hbb_malloc(sizeof(ParameterList));
    parameterList->name = identifier;
    parameterList->next = NULL;
    return parameterList;
}
