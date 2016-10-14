#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

static void
check_array(DVM_Object *array, int index,
            DVM_Executable *exe, Function *func, int pc)
{
    if (array == NULL) {
        dvm_error(exe, func, pc, NULL_POINTER_ERR,
                  MESSAGE_ARGUMENT_END);
    }
    if (index < 0 || index >= array->u.array.size) {
        dvm_error(exe, func, pc, INDEX_OUT_OF_BOUNDS_ERR,
                  INT_MESSAGE_ARGUMENT, "index", index,
                  INT_MESSAGE_ARGUMENT, "size", array->u.array.size,
                  MESSAGE_ARGUMENT_END);
    }
}

int
DVM_array_get_int(DVM_VirtualMachine *dvm, DVM_Object *array, int index)
{
    check_array(array, index,
                dvm->current_executable, dvm->current_function, dvm->pc);

    return array->u.array.u.int_array[index];
}

double
DVM_array_get_double(DVM_VirtualMachine *dvm, DVM_Object *array, int index)
{
    check_array(array, index,
                dvm->current_executable, dvm->current_function, dvm->pc);


    return array->u.array.u.double_array[index];
}

DVM_Object *
DVM_array_get_object(DVM_VirtualMachine *dvm, DVM_Object *array, int index)
{
    check_array(array, index,
                dvm->current_executable, dvm->current_function, dvm->pc);

    return array->u.array.u.object[index];
}

void
DVM_array_set_int(DVM_VirtualMachine *dvm, DVM_Object *array, int index,
                  int value)
{
    check_array(array, index,
                dvm->current_executable, dvm->current_function, dvm->pc);

    array->u.array.u.int_array[index] = value;
}

void
DVM_array_set_double(DVM_VirtualMachine *dvm, DVM_Object *array, int index,
                     double value)
{
    check_array(array, index,
                dvm->current_executable, dvm->current_function, dvm->pc);

    array->u.array.u.double_array[index] = value;
}

void
DVM_array_set_object(DVM_VirtualMachine *dvm, DVM_Object *array, int index,
                     DVM_Object *value)
{
    check_array(array, index,
                dvm->current_executable, dvm->current_function, dvm->pc);

    array->u.array.u.object[index] = value;
}
