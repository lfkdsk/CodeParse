#include <string.h>
#include "dvm_pri.h"

DVM_ErrorDefinition dvm_error_message_format[] = {
    {"dummy"},
    {"不正确的多字节字符。"},
    {"找不到函数$(name)。"},
    {"重复定义了函数$(package)#$(name)。"},
    {"数组下标越界。数组大小为$(size)，访问的下标为[$(index)]。"},
    {"整数值不能被0除。"},
    {"引用了null。"},
    {"没有找到要加载的文件$(file)"},
    {"加载文件时发生错误($(status))。"},
    {"重复定义了类$(package)#$(name)。"},
    {"没有找到类$(name)。"},
    {"对象的类型为$(org)。"
     "不能向下转型为$(target)。"},
    {"重复定义了枚举类型$(package)#$(name)。"},
    {"重复定义了字面量$(package)#$(name)。"},
    {"由于函数$(name)没有指定包，不能动态加载。"},
    {"dummy"}
};

DVM_ErrorDefinition dvm_native_error_message_format[] = {
    {"数组下标越界。尝试为大小为$(size)的数组insert元素到$(pos)"},
    {"数组下标越界。尝试为大小为$(size)的数组remove元素$(pos)"},
    {"指定的位置超出字符串长度。"
     "为长度为$(len)的字符串指定了$(pos)。"},
    {"字符串substr()的第二个参数(截取字符串的长度)超出范围($(len))。"},
    {"传递给fopen的第1个参数为null。"},
    {"传递给fopen的第2个参数为null。"},
    {"传递给fgets文件指针为null。"},
    {"fgets的参数类型错误。"},
    {"传递给fgets文件指针无效。"
     "文件可能已经关闭。"},
    {"fgets读取的多字节字符串不能转换为内部表达式。"},
    {"fputs第2个参数文件指针为null。"},
    {"fputs的参数类型错误。"},
    {"传递给fputs的文件指针无效"
     "文件可能已经关闭。"},
    {"fclose的参数为null。"},
    {"fclose的参数类型错误。"},
    {"传递给fclose的文件指针无效。"
     "可能已经关闭。"},
    {"parse_int()的参数为null。"},
    {"parse_int()格式化错误。字符串:「$(str)」"},
    {"parse_double()的参数为null。"},
    {"parse_double()格式化错误。字符串:「$(str)」"},
};
