#include "hobby.h"
#include <string.h>

/**
 * 硬编码的错误信息
 */

MessageFormat hbb_compile_error_message_format[] = {
        {"dummy"},
        {"在 ($(token)) 附近发生语法错误"},
        {"不正确的字符 ($(bad_char)) "},
        {"函数名重复 ($(name))"},
        {"dummy"}
};

MessageFormat hbb_runtime_error_message_format[] = {
        {"dummy"},
        {"找不到变量 ($(name)) ."},
        {"找不到函数 ($(name)) ."},
        {"传入的参数多余函数定义 "},
        {"传入的参数少于函数定义 "},
        {"条件表达式的值必须为boolean类型 "},
        {"减法运算的操作数必须为数值类型"},
        {"双目运算符 $(operator) 的操作数类型不同 "},
        {"$(operator) 操作符不能用于boolean类型"},
        {"请为 fopen() 函数传入的文件路径和打开方式 (两者都是字符串类型的) "},
        {"请为 fclose() 函数传入文件指针 "},
        {"请为 fgets() 函数传入文件指针 "},
        {"请为 fputs() 函数传入文件指针和字符串 "},
        {"null 只能用于运算符 == 和 != (不能进行 $(operator) 操作) "},
        {"不能被0除"},
        {"全局变量$(name)不存在"},
        {"不能在函数外使用global语句 "},
        {"运算符$(operator) 不能用于字符串类型 "},
        {"dummy"}
};
