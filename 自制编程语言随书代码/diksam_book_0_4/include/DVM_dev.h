#ifndef DVM_DEV_H_INCLUDED
#define DVM_DEV_H_INCLUDED
#include "DVM.h"
#include "DVM_code.h"

typedef struct DVM_Array_tag DVM_Array;
typedef struct DVM_Context_tag DVM_Context;

typedef enum {
    DVM_SUCCESS = 1,
    DVM_ERROR
} DVM_ErrorStatus;

#define DVM_is_null(value) ((value).object.data == NULL)
#define DVM_is_null_object(obj) ((obj).data == NULL)

typedef DVM_Value DVM_NativeFunctionProc(DVM_VirtualMachine *dvm,
                                         DVM_Context *context,
                                         int arg_count, DVM_Value *args);

typedef void DVM_NativePointerFinalizeProc(DVM_VirtualMachine *dvm,
                                           DVM_Object *obj);

typedef struct {
    char                                *name;
    DVM_NativePointerFinalizeProc       *finalizer;
} DVM_NativePointerInfo;

typedef enum {
    DVM_INT_MESSAGE_ARGUMENT = 1,
    DVM_DOUBLE_MESSAGE_ARGUMENT,
    DVM_STRING_MESSAGE_ARGUMENT,
    DVM_CHARACTER_MESSAGE_ARGUMENT,
    DVM_POINTER_MESSAGE_ARGUMENT,
    DVM_MESSAGE_ARGUMENT_END
} DVM_MessageArgumentType;

typedef struct {
    char *format;
} DVM_ErrorDefinition;

typedef struct {
    DVM_ErrorDefinition *message_format;
} DVM_NativeLibInfo;

#define DVM_DIKSAM_DEFAULT_PACKAGE_P1  "diksam"
#define DVM_DIKSAM_DEFAULT_PACKAGE_P2  "lang"
#define DVM_DIKSAM_DEFAULT_PACKAGE \
 (DVM_DIKSAM_DEFAULT_PACKAGE_P1 "." DVM_DIKSAM_DEFAULT_PACKAGE_P2)
#define DVM_NULL_POINTER_EXCEPTION_NAME ("NullPointerException")

/* load.c */
int DVM_search_class(DVM_VirtualMachine *dvm, char *package_name, char *name);

/* execute.c */
DVM_Context *DVM_push_context(DVM_VirtualMachine *dvm);
void DVM_pop_context(DVM_VirtualMachine *dvm, DVM_Context *context);
DVM_Context *DVM_create_context(DVM_VirtualMachine *dvm);
void DVM_dispose_context(DVM_VirtualMachine *dvm, DVM_Context *context);
void DVM_add_native_function(DVM_VirtualMachine *dvm,
                             char *package_name, char *func_name,
                             DVM_NativeFunctionProc *proc, int arg_count,
                             DVM_Boolean is_method,
                             DVM_Boolean return_pointer);
DVM_Value DVM_invoke_delegate(DVM_VirtualMachine *dvm, DVM_Value delegate,
                              DVM_Value *args);
/* nativeif.c */
DVM_ErrorStatus
DVM_array_get_int(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index,
                  int *value, DVM_ObjectRef *exception_p);
DVM_ErrorStatus
DVM_array_get_double(DVM_VirtualMachine *dvm, DVM_ObjectRef array,
                     int index, double *value,
                     DVM_ObjectRef *exception_p);
DVM_ErrorStatus
DVM_array_get_object(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index,
                     DVM_ObjectRef *value, DVM_ObjectRef *exception_p);
DVM_ErrorStatus
DVM_array_set_int(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index,
                  int value, DVM_ObjectRef *exception_p);
DVM_ErrorStatus
DVM_array_set_double(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index,
                     double value, DVM_ObjectRef *exception_p);
DVM_ErrorStatus
DVM_array_set_object(DVM_VirtualMachine *dvm, DVM_ObjectRef array, int index,
                     DVM_ObjectRef value, DVM_ObjectRef *exception_p);
int DVM_array_size(DVM_VirtualMachine *dvm, DVM_Object *array);
void
DVM_array_resize(DVM_VirtualMachine *dvm, DVM_Object *array, int new_size);
void DVM_array_insert(DVM_VirtualMachine *dvm, DVM_Object *array, int pos,
                      DVM_Value value);
void DVM_array_remove(DVM_VirtualMachine *dvm, DVM_Object *array, int pos);
int DVM_string_length(DVM_VirtualMachine *dvm, DVM_Object *string);
DVM_Char *DVM_string_get_string(DVM_VirtualMachine *dvm, DVM_Object *string);
DVM_ErrorStatus
DVM_string_get_character(DVM_VirtualMachine *dvm, DVM_ObjectRef string,
                         int index, DVM_Char *ch, DVM_ObjectRef *exception);
DVM_Value DVM_string_substr(DVM_VirtualMachine *dvm, DVM_Object *str,
                            int pos, int len);
int DVM_get_field_index(DVM_VirtualMachine *dvm, DVM_ObjectRef obj,
                        char *field_name);
DVM_Value DVM_get_field(DVM_ObjectRef obj, int index);
void DVM_set_field(DVM_ObjectRef obj, int index, DVM_Value value);

void DVM_set_exception(DVM_VirtualMachine *dvm, DVM_Context *context,
                       DVM_NativeLibInfo *lib_info,
                       char *package_name, char *class_name,
                       int error_id, ...);
void DVM_set_null(DVM_Value *value);
DVM_ObjectRef DVM_up_cast(DVM_ObjectRef obj, int target_index);

DVM_Value DVM_check_exception(DVM_VirtualMachine *dvm);

/* heap.c */
void DVM_check_gc(DVM_VirtualMachine *dvm);
void DVM_add_reference_to_context(DVM_Context *context, DVM_Value value);
DVM_ObjectRef
DVM_create_array_int(DVM_VirtualMachine *dvm, DVM_Context *context,
                     int size);
DVM_ObjectRef
DVM_create_array_double(DVM_VirtualMachine *dvm, DVM_Context *context,
                        int size);
DVM_ObjectRef
DVM_create_array_object(DVM_VirtualMachine *dvm,  DVM_Context *context,
                        int size);
void *DVM_object_get_native_pointer(DVM_Object *obj);
DVM_Boolean DVM_check_native_pointer_type(DVM_Object *native_pointer,
                                          DVM_NativePointerInfo *info);
void DVM_object_set_native_pointer(DVM_Object *obj, void *p);
DVM_ObjectRef
DVM_create_dvm_string(DVM_VirtualMachine *dvm, DVM_Context *context,
                      DVM_Char *str);
DVM_ObjectRef
DVM_create_class_object(DVM_VirtualMachine *dvm, DVM_Context *context,
                        int class_index);
DVM_ObjectRef
DVM_create_native_pointer(DVM_VirtualMachine *dvm, DVM_Context *context,
                          void *pointer, DVM_NativePointerInfo *info);

/* wchar.c */
char *DVM_wcstombs(const wchar_t *src);
DVM_Char *DVM_mbstowcs(const char *src);

#endif /* DVM_DEV_H_INCLUDED */
