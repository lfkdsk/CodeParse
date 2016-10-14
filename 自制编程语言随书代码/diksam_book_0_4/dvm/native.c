#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

static void file_finalizer(DVM_VirtualMachine *dvm, DVM_Object* obj);

static DVM_NativePointerInfo st_file_type_info = {
    "diksam.lang.file_pointer",
    file_finalizer
};

extern DVM_ErrorDefinition dvm_native_error_message_format[];

static DVM_NativeLibInfo st_lib_info = {
    dvm_native_error_message_format,
};

typedef enum {
    INSERT_INDEX_OUT_OF_BOUNDS_ERR,
    REMOVE_INDEX_OUT_OF_BOUNDS_ERR,
    STRING_POS_OUT_OF_BOUNDS_ERR,
    STRING_SUBSTR_LEN_ERR,
    FOPEN_1ST_ARG_NULL_ERR,
    FOPEN_2ND_ARG_NULL_ERR,
    FGETS_ARG_NULL_ERR,
    FGETS_FP_BAD_TYPE_ERR,
    FGETS_FP_INVALID_ERR,
    FGETS_BAD_MULTIBYTE_CHARACTER_ERR,
    FPUTS_2ND_ARG_NULL_ERR,
    FPUTS_FP_BAD_TYPE_ERR,
    FPUTS_FP_INVALID_ERR,
    FCLOSE_ARG_NULL_ERR,
    FCLOSE_FP_BAD_TYPE_ERR,
    FCLOSE_FP_INVALID_ERR,
    PARSE_INT_ARG_NULL_ERR,
    PARSE_INT_FORMAT_ERR,
    PARSE_DOUBLE_ARG_NULL_ERR,
    PARSE_DOUBLE_FORMAT_ERR,
    NATIVE_RUNTIME_ERROR_COUNT_PLUS_1
} NativeRuntimeError;

static void
file_finalizer(DVM_VirtualMachine *dvm, DVM_Object *obj)
{
    FILE *fp;

    fp = (FILE*)DVM_object_get_native_pointer(obj);
    if (fp && fp != stdin && fp != stdout && fp != stderr) {
        fclose(fp);
    }
}

static DVM_Value
nv_print_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
              int arg_count, DVM_Value *args)
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
nv_fopen_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
              int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    char *file_name;
    char *mode;
    FILE *fp;

    DBG_assert(arg_count == 2, ("arg_count..%d", arg_count));

    if (is_object_null(args[0].object)) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          FOPEN_1ST_ARG_NULL_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    if (is_object_null(args[1].object)) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          FOPEN_2ND_ARG_NULL_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    file_name = DVM_wcstombs(args[0].object.data->u.string.string);
    mode = DVM_wcstombs(args[1].object.data->u.string.string);
    fp = fopen(file_name, mode);
    MEM_free(file_name);
    MEM_free(mode);

    ret.object = DVM_create_native_pointer(dvm, context, fp,
                                           &st_file_type_info);

    return ret;
}

static DVM_Value
nv_fgets_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
              int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    FILE *fp;
    char buf[LINE_BUF_SIZE];
    char *mb_buf = NULL;
    int ret_len = 0;
    DVM_Char *wc_str;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));

    if (is_object_null(args[0].object)) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          FOPEN_1ST_ARG_NULL_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    if (!DVM_check_native_pointer_type(args[0].object.data,
                                       &st_file_type_info)) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          FGETS_FP_BAD_TYPE_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    fp = DVM_object_get_native_pointer(args[0].object.data);
    if (fp == NULL) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          FGETS_FP_INVALID_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }

    while (fgets(buf, LINE_BUF_SIZE, fp)) {
        int new_len;
        new_len = ret_len + strlen(buf);
        mb_buf = MEM_realloc(mb_buf, new_len + 1);
        if (ret_len == 0) {
            strcpy(mb_buf, buf);
        } else {
            strcat(mb_buf, buf);
        }
        ret_len = new_len;
        if (mb_buf[ret_len-1] == '\n')
            break;
    }
    if (ret_len > 0) {
        wc_str = DVM_mbstowcs(mb_buf);
        if (wc_str == NULL) {
            MEM_free(mb_buf);
            DVM_set_exception(dvm, context, &st_lib_info,
                              DVM_DIKSAM_DEFAULT_PACKAGE,
                              MULTIBYTE_CONVERTION_EXCEPTION_NAME,
                              (int)FGETS_BAD_MULTIBYTE_CHARACTER_ERR,
                              DVM_MESSAGE_ARGUMENT_END);
        }
        ret.object = DVM_create_dvm_string(dvm, context, wc_str);
    } else {
        ret.object = dvm_null_object_ref;
    }
    MEM_free(mb_buf);

    return ret;
}

static DVM_Value
nv_fputs_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
              int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    DVM_Char *str;
    FILE *fp;

    ret.int_value = 0;

    DBG_assert(arg_count == 2, ("arg_count..%d", arg_count));

    if (args[0].object.data == NULL) {
        str = NULL_STRING;
    } else {
        str = args[0].object.data->u.string.string;
    }
    if (is_object_null(args[1].object)) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          FPUTS_2ND_ARG_NULL_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    if (!DVM_check_native_pointer_type(args[1].object.data,
                                       &st_file_type_info)) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          FPUTS_FP_BAD_TYPE_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    fp = DVM_object_get_native_pointer(args[1].object.data);
    if (fp == NULL) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          FPUTS_FP_INVALID_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    dvm_print_wcs(fp, str);

    return ret;
}

static DVM_Value
nv_fclose_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
               int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    FILE *fp;

    ret.int_value = 0;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));

    if (is_object_null(args[0].object)) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          FCLOSE_ARG_NULL_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    if (!DVM_check_native_pointer_type(args[0].object.data,
                                       &st_file_type_info)) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          FCLOSE_FP_BAD_TYPE_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    fp = DVM_object_get_native_pointer(args[0].object.data);
    if (fp == NULL) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          FCLOSE_FP_INVALID_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    fclose(fp);
    DVM_object_set_native_pointer(args[0].object.data, NULL);

    return ret;
}

static DVM_Value
nv_exit_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
              int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    exit(args[0].int_value);

    return ret;
}

static DVM_Value
nv_randomize_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
                  int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    srand(time(NULL));

    return ret;
}

static DVM_Value
nv_random_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
                  int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    if (args[0].int_value == 0) {
        ret.int_value = 0;
    } else {
        ret.int_value = rand() % args[0].int_value;
    }

    return ret;
}

static DVM_Value
nv_parse_int_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
                  int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    char *mb_str;

    if (is_object_null(args[0].object)) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          PARSE_INT_ARG_NULL_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    mb_str = DVM_wcstombs(args[0].object.data->u.string.string);

    if (sscanf(mb_str, "%d", &ret.int_value) != 1) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          NUMBER_FORMAT_EXCEPTION_NAME,
                          PARSE_INT_FORMAT_ERR,
                          DVM_STRING_MESSAGE_ARGUMENT, "str", mb_str,
                          DVM_MESSAGE_ARGUMENT_END);
    }
    MEM_free(mb_str);
    return ret;
}

static DVM_Value
nv_parse_double_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
                     int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    char *mb_str;

    if (is_object_null(args[0].object)) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          DVM_NULL_POINTER_EXCEPTION_NAME,
                          PARSE_DOUBLE_ARG_NULL_ERR,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    mb_str = DVM_wcstombs(args[0].object.data->u.string.string);

    if (sscanf(mb_str, "%lf", &ret.double_value) != 1) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          NUMBER_FORMAT_EXCEPTION_NAME,
                          PARSE_DOUBLE_FORMAT_ERR,
                          DVM_STRING_MESSAGE_ARGUMENT, "str", mb_str,
                          DVM_MESSAGE_ARGUMENT_END);
    }
    MEM_free(mb_str);
    return ret;
}

static DVM_Value
nv_fabs_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
              int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = fabs(args[0].double_value);

    return ret;
}

static DVM_Value
nv_pow_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
            int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = pow(args[0].double_value,
                           args[1].double_value);

    return ret;
}

static DVM_Value
nv_fmod_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
             int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = fmod(args[0].double_value,
                            args[1].double_value);

    return ret;
}

static DVM_Value
nv_ceil_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
             int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = ceil(args[0].double_value);

    return ret;
}

static DVM_Value
nv_floor_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
              int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = floor(args[0].double_value);

    return ret;
}

static DVM_Value
nv_sqrt_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
             int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = sqrt(args[0].double_value);

    return ret;
}

static DVM_Value
nv_exp_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
            int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = exp(args[0].double_value);

    return ret;
}

static DVM_Value
nv_log10_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
              int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = log10(args[0].double_value);

    return ret;
}

static DVM_Value
nv_log_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
            int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = log(args[0].double_value);

    return ret;
}

static DVM_Value
nv_sin_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
            int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = sin(args[0].double_value);

    return ret;
}

static DVM_Value
nv_cos_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
            int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = cos(args[0].double_value);

    return ret;
}

static DVM_Value
nv_tan_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
            int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = tan(args[0].double_value);

    return ret;
}

static DVM_Value
nv_asin_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
             int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = asin(args[0].double_value);

    return ret;
}

static DVM_Value
nv_acos_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
             int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = acos(args[0].double_value);

    return ret;
}

static DVM_Value
nv_atan_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
             int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = atan(args[0].double_value);

    return ret;
}

static DVM_Value
nv_atan2_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
              int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = atan2(args[0].double_value,
                             args[0].double_value);

    return ret;
}

static DVM_Value
nv_sinh_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
             int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = sinh(args[0].double_value);

    return ret;
}

static DVM_Value
nv_cosh_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
             int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = cosh(args[0].double_value);

    return ret;
}

static DVM_Value
nv_tanh_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
             int arg_count, DVM_Value *args)
{
    DVM_Value ret;

    ret.double_value = tanh(args[0].double_value);

    return ret;
}

static DVM_Value
nv_array_size_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
                   int arg_count, DVM_Value *args)
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
nv_array_resize_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
                     int arg_count, DVM_Value *args)
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
nv_array_insert_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
                     int arg_count, DVM_Value *args)
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
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          ARRAY_INDEX_EXCEPTION_NAME,
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
nv_array_remove_proc(DVM_VirtualMachine *dvm,  DVM_Context *context,
                     int arg_count, DVM_Value *args)
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
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          ARRAY_INDEX_EXCEPTION_NAME,
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
nv_array_add_proc(DVM_VirtualMachine *dvm,  DVM_Context *context,
                  int arg_count, DVM_Value *args)
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
nv_string_length_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
                      int arg_count, DVM_Value *args)
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
nv_string_substr_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
                      int arg_count, DVM_Value *args)
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
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          STRING_INDEX_EXCEPTION_NAME,
                          STRING_POS_OUT_OF_BOUNDS_ERR,
                          DVM_INT_MESSAGE_ARGUMENT, "len", org_len,
                          DVM_INT_MESSAGE_ARGUMENT, "pos", pos,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }
    if (len < 0 || pos + len > org_len) {
        DVM_set_exception(dvm, context, &st_lib_info,
                          DVM_DIKSAM_DEFAULT_PACKAGE,
                          STRING_INDEX_EXCEPTION_NAME,
                          STRING_SUBSTR_LEN_ERR,
                          DVM_INT_MESSAGE_ARGUMENT, "len", len,
                          DVM_MESSAGE_ARGUMENT_END);
        return ret;
    }

    ret = DVM_string_substr(dvm, str, pos, len);

    return ret;
}

DVM_Value
nv_test_native_proc(DVM_VirtualMachine *dvm, DVM_Context *context,
                    int arg_count, DVM_Value *args)
{
    DVM_Value ret;
    DVM_Value dik_arg;

    dik_arg.int_value = 13;
    DVM_invoke_delegate(dvm, args[0], &dik_arg);

    return ret;
}

void
dvm_add_native_functions(DVM_VirtualMachine *dvm)
{
    DVM_add_native_function(dvm, "diksam.lang", "print", nv_print_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.lang", "__fopen", nv_fopen_proc, 2,
                            DVM_FALSE, DVM_TRUE);
    DVM_add_native_function(dvm, "diksam.lang", "__fgets", nv_fgets_proc, 1,
                            DVM_FALSE, DVM_TRUE);
    DVM_add_native_function(dvm, "diksam.lang", "__fputs", nv_fputs_proc, 2,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.lang", "__fclose", nv_fclose_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.lang", "exit", nv_exit_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.lang", "randomize",
                            nv_randomize_proc, 0,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.lang", "random", nv_random_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.lang", "parse_int", nv_parse_int_proc,
                            1, DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.lang", "parse_double",
                            nv_parse_double_proc, 1, DVM_FALSE, DVM_FALSE);
    
    /* math.dkh */
    DVM_add_native_function(dvm, "diksam.math", "fabs", nv_fabs_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "pow", nv_pow_proc, 2,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "fmod", nv_fmod_proc, 2,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "ceil", nv_ceil_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "floor", nv_floor_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "sqrt", nv_sqrt_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "exp", nv_exp_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "log10", nv_log10_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "log", nv_log_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "sin", nv_sin_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "cos", nv_cos_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "tan", nv_tan_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "asin", nv_asin_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "acos", nv_acos_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "atan", nv_atan_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "atan2", nv_atan2_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "sinh", nv_sinh_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "cosh", nv_cosh_proc, 1,
                            DVM_FALSE, DVM_FALSE);
    DVM_add_native_function(dvm, "diksam.math", "tanh", nv_tanh_proc, 1,
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

    DVM_add_native_function(dvm, "diksam.lang", "test_native",
                            nv_test_native_proc, 1,
                            DVM_FALSE, DVM_FALSE);
}
