#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

static DVM_Value
nv_print_proc(DVM_VirtualMachine *dvm,
              int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.int_value = 0;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));
    dvm_print_wcs(stdout, args[0].object->u.string.string);
    fflush(stdout);

    return ret;
}

void
dvm_add_native_functions(DVM_VirtualMachine *dvm)
{
    DVM_add_native_function(dvm, "print", nv_print_proc, 1);
}
