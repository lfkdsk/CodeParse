#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

static DVM_Value
nv_print_proc(DVM_VirtualMachine *dvm,
              int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    DVM_Char *str;

    ret.int_value = 0;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));

    if (args[0].object == NULL) {
        str = NULL_STRING;
    } else {
        str = args[0].object->u.string.string;
    }
    dvm_print_wcs(stdout, str);
    fflush(stdout);

    return ret;
}

void
dvm_add_native_functions(DVM_VirtualMachine *dvm)
{
    DVM_add_native_function(dvm, "print", nv_print_proc, 1);
}
