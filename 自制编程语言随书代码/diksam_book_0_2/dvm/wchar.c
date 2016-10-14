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
            dvm_error(dvm->current_executable, dvm->current_function,
                      dvm->pc,
                      BAD_MULTIBYTE_CHARACTER_ERR,
                      MESSAGE_ARGUMENT_END);
        } else {
            dvm_error(NULL, NULL, NO_LINE_NUMBER_PC,
                      BAD_MULTIBYTE_CHARACTER_ERR,
                      MESSAGE_ARGUMENT_END);
        }
        return NULL;
    }
    ret = MEM_malloc(sizeof(wchar_t) * (len+1));
    dvm_mbstowcs(src, ret);

    return ret;
}

