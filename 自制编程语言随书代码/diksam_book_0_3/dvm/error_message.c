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
    {"由于函数$(name)没有指定包，不能动态加载。"},
    {"dummy"}
};

DVM_ErrorDefinition dvm_native_error_message_format[] = {
    {"数?下?越界。???大小?$(size)的数?insert元素到$(pos)"
     "insertしようとしています。"},
    {"数?下?越界。???大小?$(size)的数?insert元素到$(pos)"},
    {"指定的位置超出字符串?度。"
     "??度?$(len)的字符串指定了$(pos)。"},
    {"字符串substr()函数的第二个参数(截取字符串的?度)超出范?($(len))。"},
};
