#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "DBG.h"
#include "MEM.h"
#include "dvm_pri.h"

wchar_t *
dvm_mbstowcs_alloc(DVM_Executable *exe, Function *func, int pc,
                   const char *src)
{
    int len;
    wchar_t *ret;

    len = dvm_mbstowcs_len(src);
    if (len < 0) {
        return NULL;
        dvm_error(exe, func, pc,
                  BAD_MULTIBYTE_CHARACTER_ERR,
                  MESSAGE_ARGUMENT_END);
    }
    ret = MEM_malloc(sizeof(wchar_t) * (len+1));
    dvm_mbstowcs(src, ret);

    return ret;
}

