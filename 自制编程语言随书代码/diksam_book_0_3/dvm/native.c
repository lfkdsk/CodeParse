#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

extern DVM_ErrorDefinition dvm_native_error_message_format[];

static DVM_NativeLibInfo st_lib_info = {
    dvm_native_error_message_format,
};

typedef enum {
    INSERT_INDEX_OUT_OF_BOUNDS_ERR,
    REMOVE_INDEX_OUT_OF_BOUNDS_ERR,
    STRING_POS_OUT_OF_BOUNDS_ERR,
    STRING_SUBSTR_LEN_ERR,
    NATIVE_RUNTIME_ERROR_COUNT_PLUS_1
} NativeRuntimeError;

static DVM_Value
nv_print_proc(DVM_VirtualMachine *dvm, int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    DVM_Char *str;

    ret.int_value = 0;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));

    if (args[0].object.data == NULL) {
        str = NULL_STRING;
    } else {
        str = args[0].object.data->u.string.string;
    }
    dvm_print_wcs(stdout, str);
    fflush(stdout);

    return ret;
}

static DVM_Value
nv_array_size_proc(DVM_VirtualMachine *dvm, int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    DVM_Object *array;

    DBG_assert(arg_count == 0, ("arg_count..%d", arg_count));

    array = args[0].object.data;
    DBG_assert(array->type == ARRAY_OBJECT, ("array->type..%d", array->type));

    ret.int_value = DVM_array_size(dvm, array);

    return ret;
}

static DVM_Value
nv_array_resize_proc(DVM_VirtualMachine *dvm, int arg_count, DVM_Value *args)
{
    DVM_Value ret; /* dummy */
    DVM_Object *array;
    int new_size;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));

    new_size = args[0].int_value;
    array = args[1].object.data;
    DBG_assert(array->type == ARRAY_OBJECT, ("array->type..%d", array->type));

    DVM_array_resize(dvm, array, new_size);

    return ret;
}

static DVM_Value
nv_array_insert_proc(DVM_VirtualMachine *dvm, int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    DVM_Object *array;
    DVM_Value value;
    int pos;
    int array_size;

    pos = args[0].int_value;
    value = args[1];
    array = args[2].object.data;

    array_size = DVM_array_size(dvm, array);

    if (pos < 0 || pos > array_size) {
        dvm_error_n(dvm, st_lib_info.message_format,
                    INSERT_INDEX_OUT_OF_BOUNDS_ERR,
                    DVM_INT_MESSAGE_ARGUMENT, "pos", pos,
                    DVM_INT_MESSAGE_ARGUMENT, "size", array_size,
                    DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    DVM_array_insert(dvm, array, pos, value);

    return ret;
}

DVM_Value
nv_array_remove_proc(DVM_VirtualMachine *dvm,  int arg_count, DVM_Value *args)
{
    DVM_Value ret; /* dummy */
    DVM_Object *array;
    int pos;
    int array_size;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));

    pos = args[0].int_value;
    array = args[1].object.data;
    DBG_assert(array->type == ARRAY_OBJECT, ("array->type..%d", array->type));

    array_size = DVM_array_size(dvm, array);
    if (pos < 0 || pos >= array_size) {
        dvm_error_n(dvm, st_lib_info.message_format,
                    REMOVE_INDEX_OUT_OF_BOUNDS_ERR,
                    DVM_INT_MESSAGE_ARGUMENT, "pos", pos,
                    DVM_INT_MESSAGE_ARGUMENT, "size", array_size,
                    DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    DVM_array_remove(dvm, array, pos);

    return ret;
}

static DVM_Value
nv_array_add_proc(DVM_VirtualMachine *dvm,  int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    DVM_Object *array;
    DVM_Value value;
    int array_size;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));
    value = args[0];
    array = args[1].object.data;

    array_size = DVM_array_size(dvm, array);

    DVM_array_insert(dvm, array, array_size, value);

    return ret;
}

static DVM_Value
nv_string_length_proc(DVM_VirtualMachine *dvm, int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    DVM_Object *str;

    DBG_assert(arg_count == 0, ("arg_count..%d", arg_count));

    str = args[0].object.data;
    DBG_assert(str->type == STRING_OBJECT,
               ("str->type..%d", str->type));

    ret.int_value = DVM_string_length(dvm, str);

    return ret;
}

static DVM_Value
nv_string_substr_proc(DVM_VirtualMachine *dvm, int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    DVM_Object *str;
    int pos;
    int len;
    int org_len;

    DBG_assert(arg_count == 2, ("arg_count..%d", arg_count));

    pos = args[0].int_value;
    len = args[1].int_value;
    str = args[2].object.data;
    DBG_assert(str->type == STRING_OBJECT,
               ("str->type..%d", str->type));

    org_len = DVM_string_length(dvm, str);
    
    if (pos < 0 || pos >= org_len) {
        dvm_error_n(dvm, st_lib_info.message_format,
                    STRING_POS_OUT_OF_BOUNDS_ERR,
                    DVM_INT_MESSAGE_ARGUMENT, "len", org_len,
                    DVM_INT_MESSAGE_ARGUMENT, "pos", pos,
                    DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    if (len < 0 || pos + len > org_len) {
        dvm_error_n(dvm, st_lib_info.message_format,
                    STRING_SUBSTR_LEN_ERR,
                    DVM_INT_MESSAGE_ARGUMENT, "len", len,
                    DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }

    ret = DVM_string_substr(dvm, str, pos, len);

    return ret;
}

void
dvm_add_native_functions(DVM_VirtualMachine *dvm)
{
    DVM_add_native_function(dvm, "diksam.lang", "print", nv_print_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, BUILT_IN_METHOD_PACKAGE_NAME,
                            ARRAY_PREFIX ARRAY_METHOD_SIZE,
                            nv_array_size_proc, 0, DVM_TRUE, DVM_FALSE);
    DVM_add_native_function(dvm, BUILT_IN_METHOD_PACKAGE_NAME,
                            ARRAY_PREFIX ARRAY_METHOD_RESIZE,
                            nv_array_resize_proc, 1, DVM_TRUE, DVM_FALSE);
    DVM_add_native_function(dvm, BUILT_IN_METHOD_PACKAGE_NAME,
                            ARRAY_PREFIX ARRAY_METHOD_INSERT,
                            nv_array_insert_proc, 2, DVM_TRUE, DVM_FALSE);
    DVM_add_native_function(dvm, BUILT_IN_METHOD_PACKAGE_NAME,
                            ARRAY_PREFIX ARRAY_METHOD_REMOVE,
                            nv_array_remove_proc, 1, DVM_TRUE, DVM_FALSE);
    DVM_add_native_function(dvm, BUILT_IN_METHOD_PACKAGE_NAME,
                            ARRAY_PREFIX ARRAY_METHOD_ADD,
                            nv_array_add_proc, 1, DVM_TRUE, DVM_FALSE);

    DVM_add_native_function(dvm, BUILT_IN_METHOD_PACKAGE_NAME,
                            STRING_PREFIX STRING_METHOD_LENGTH,
                            nv_string_length_proc, 0, DVM_TRUE, DVM_FALSE);
    DVM_add_native_function(dvm, BUILT_IN_METHOD_PACKAGE_NAME,
                            STRING_PREFIX STRING_METHOD_SUBSTR,
                            nv_string_substr_proc, 2, DVM_TRUE, DVM_FALSE);
}
