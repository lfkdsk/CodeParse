#ifndef DVM_DEV_H_INCLUDED
#define DVM_DEV_H_INCLUDED
#include "DVM.h"

typedef struct DVM_Array_tag DVM_Array;

typedef DVM_Value DVM_NativeFunctionProc(DVM_VirtualMachine *dvm,
                                         int arg_count, DVM_Value *args);
/* execute.c */
void DVM_add_native_function(DVM_VirtualMachine *dvm, char *func_name,
                             DVM_NativeFunctionProc *proc, int arg_count);
/* nativeif.c */
int DVM_array_get_int(DVM_VirtualMachine *dvm, DVM_Object *array, int index);
double DVM_array_get_double(DVM_VirtualMachine *dvm, DVM_Object *array,
                            int index);
DVM_Object *
DVM_array_get_object(DVM_VirtualMachine *dvm, DVM_Object *array, int index);
void
DVM_array_set_int(DVM_VirtualMachine *dvm, DVM_Object *array, int index,
                  int value);
void
DVM_array_set_double(DVM_VirtualMachine *dvm, DVM_Object *array, int index,
                     double value);
void
DVM_array_set_object(DVM_VirtualMachine *dvm, DVM_Object *array, int index,
                     DVM_Object *value);


/* heap.c */
DVM_Object *DVM_create_array_int(DVM_VirtualMachine *dvm, int size);
DVM_Object *DVM_create_array_double(DVM_VirtualMachine *dvm, int size);
DVM_Object *DVM_create_array_object(DVM_VirtualMachine *dvm, int size);

#endif /* DVM_DEV_H_INCLUDED */
