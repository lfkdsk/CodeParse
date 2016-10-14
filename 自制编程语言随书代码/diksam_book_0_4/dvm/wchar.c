#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "DBG.h"
#include "MEM.h"
#include "dvm_pri.h"

wchar_t *
dvm_mbstowcs_alloc(DVM_VirtualMachine *dvm, const char *src)
{
    int len;
    wchar_t *ret;

    len = dvm_mbstowcs_len(src);
    if (len < 0) {
        if (dvm) {
            dvm_error_i(dvm->current_executable->executable,
                        dvm->current_function,
                        dvm->pc,
                        BAD_MULTIBYTE_CHARACTER_ERR,
                        DVM_MESSAGE_ARGUMENT_END);
        } else {
            dvm_error_i(NULL, NULL, NO_LINE_NUMBER_PC,
                        BAD_MULTIBYTE_CHARACTER_ERR,
                        DVM_MESSAGE_ARGUMENT_END);
        }
        return NULL;
    }
    ret = MEM_malloc(sizeof(wchar_t) * (len+1));
    dvm_mbstowcs(src, ret);

    return ret;
}

char *
DVM_wcstombs(const wchar_t *src)
{
    char *ret;

    ret = dvm_wcstombs_alloc(src);

    return ret;
}

DVM_Char *
DVM_mbstowcs(const char *src)
{
    int len;
    DVM_Char *ret;

    len = dvm_mbstowcs_len(src);
    if (len < 0) {
        return NULL;
    }
    ret = MEM_malloc(sizeof(wchar_t) * (len+1));
    dvm_mbstowcs(src, ret);

    return ret;
}
