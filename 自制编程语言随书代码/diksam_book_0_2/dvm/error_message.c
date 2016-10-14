#include <string.h>
#include "dvm_pri.h"

ErrorDefinition dvm_error_message_format[] = {
    {"dummy"},
    {"不正确的多字节字符。"},
    {"找不到函数$(name)。"},
    {"重复定义函数$(name)。"},
    {"数组下标越界。数组大小为$(size)，访问的下标为[$(index)]。"},
    {"整数值不能被0除。"},
    {"引用了null。"},
    {"dummy"}
};
