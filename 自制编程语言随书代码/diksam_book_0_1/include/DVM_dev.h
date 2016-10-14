#ifndef DVM_DEV_H_INCLUDED
#define DVM_DEV_H_INCLUDED
#include "DVM.h"

typedef DVM_Value DVM_NativeFunctionProc(DVM_VirtualMachine *dvm,
                                         int arg_count, DVM_Value *args);
void DVM_add_native_function(DVM_VirtualMachine *dvm, char *func_name,
                             DVM_NativeFunctionProc *proc, int arg_count);

#endif /* DVM_DEV_H_INCLUDED */
