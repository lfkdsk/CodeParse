#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"
#include "CRB_dev.h"

#define LINE_BUF_SIZE   (1024)

typedef enum {
    FOPEN_ARGUMENT_TYPE_ERR = 0,
    FCLOSE_ARGUMENT_TYPE_ERR,
    FGETS_ARGUMENT_TYPE_ERR,
    FILE_ALREADY_CLOSED_ERR,
    FPUTS_ARGUMENT_TYPE_ERR,
    NEW_ARRAY_ARGUMENT_TYPE_ERR,
    NEW_ARRAY_ARGUMENT_TOO_FEW_ERR,
    EXIT_ARGUMENT_TYPE_ERR,
    NEW_EXCEPTION_ARGUMENT_ERR,
    FGETS_BAD_MULTIBYTE_CHARACTER_ERR
} NativeErrorCode;

extern CRB_ErrorDefinition crb_native_error_message_format[];

static CRB_NativeLibInfo st_lib_info = {
    crb_native_error_message_format,
};

static void file_finalizer(CRB_Interpreter *inter, CRB_Object *obj);

static CRB_NativePointerInfo st_file_type_info = {
    "crowbar.lang.file",
    file_finalizer
};

static void
file_finalizer(CRB_Interpreter *inter, CRB_Object *obj)
{
    FILE *fp;

    fp = (FILE*)CRB_object_get_native_pointer(obj);
    if (fp && fp != stdin && fp != stdout && fp != stderr) {
        fclose(fp);
    }
}

static CRB_Value
nv_print_proc(CRB_Interpreter *interpreter,
              CRB_LocalEnvironment *env,
              int arg_count, CRB_Value *args)
{
    CRB_Value value;
    CRB_Char *str;

    value.type = CRB_NULL_VALUE;

    CRB_check_argument_count(interpreter, env, arg_count, 1);

    str = CRB_value_to_string(interpreter, env, __LINE__, &args[0]);
    CRB_print_wcs(stdout, str);
    MEM_free(str);

    return value;
}

static void
check_file_pointer(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                   CRB_Object *obj)
{
    FILE *fp;

    fp = (FILE*)CRB_object_get_native_pointer(obj);
    if (fp == NULL) {
        CRB_error(inter, env, &st_lib_info, __LINE__,
                  (int)FILE_ALREADY_CLOSED_ERR,
                  CRB_MESSAGE_ARGUMENT_END);
    }
}

static CRB_Value
nv_fopen_proc(CRB_Interpreter *interpreter,
              CRB_LocalEnvironment *env,
              int arg_count, CRB_Value *args)
{
    CRB_Value value;
    char *filename;
    char *mode;
    FILE *fp;

    CRB_check_argument_count(interpreter, env, arg_count, 2);

    if (args[0].type != CRB_STRING_VALUE
        || args[1].type != CRB_STRING_VALUE) {
        CRB_error(interpreter, env, &st_lib_info, __LINE__,
                  (int)FOPEN_ARGUMENT_TYPE_ERR,
                  CRB_MESSAGE_ARGUMENT_END);
    }
    
    filename = CRB_wcstombs_alloc(CRB_object_get_string(args[0].u.object));
    mode = CRB_wcstombs_alloc(CRB_object_get_string(args[1].u.object));

    fp = fopen(filename, mode);
    if (fp == NULL) {
        value.type = CRB_NULL_VALUE;
    } else {
        value.type = CRB_NATIVE_POINTER_VALUE;
        value.u.object
            = CRB_create_native_pointer(interpreter, env, fp,
                                        &st_file_type_info);
    }
    MEM_free(filename);
    MEM_free(mode);

    return value;
}

static CRB_Value
nv_fclose_proc(CRB_Interpreter *interpreter,
               CRB_LocalEnvironment *env,
               int arg_count, CRB_Value *args)
{
    CRB_Value value;
    FILE *fp;

    CRB_check_argument_count(interpreter, env, arg_count, 1);

    value.type = CRB_NULL_VALUE;
    if (args[0].type != CRB_NATIVE_POINTER_VALUE
        || (!CRB_check_native_pointer_type(args[0].u.object,
                                           &st_file_type_info))) {
        CRB_error(interpreter, env, &st_lib_info, __LINE__,
                  (int)FCLOSE_ARGUMENT_TYPE_ERR,
                  CRB_MESSAGE_ARGUMENT_END);
    }
    check_file_pointer(interpreter,env, args[0].u.object);
    fp = CRB_object_get_native_pointer(args[0].u.object);
    fclose(fp);
    CRB_object_set_native_pointer(args[0].u.object, NULL);

    return value;
}

static CRB_Value
nv_fgets_proc(CRB_Interpreter *interpreter,
              CRB_LocalEnvironment *env,
              int arg_count, CRB_Value *args)
{
    CRB_Value value;
    FILE *fp;
    char buf[LINE_BUF_SIZE];
    char *mb_buf = NULL;
    int ret_len = 0;
    CRB_Char *wc_str;

    CRB_check_argument_count(interpreter, env, arg_count, 1);

    if (args[0].type != CRB_NATIVE_POINTER_VALUE
        || (!CRB_check_native_pointer_type(args[0].u.object,
                                           &st_file_type_info))) {
        CRB_error(interpreter, env, &st_lib_info, __LINE__,
                  (int)FGETS_ARGUMENT_TYPE_ERR,
                  CRB_MESSAGE_ARGUMENT_END);
    }
    check_file_pointer(interpreter,env, args[0].u.object);
    fp = CRB_object_get_native_pointer(args[0].u.object);

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
        wc_str = CRB_mbstowcs_alloc(interpreter, env, __LINE__, mb_buf);
        if (wc_str == NULL) {
            MEM_free(mb_buf);
            CRB_error(interpreter, env, &st_lib_info, __LINE__,
                      (int)FGETS_BAD_MULTIBYTE_CHARACTER_ERR,
                      CRB_MESSAGE_ARGUMENT_END);
        }
        value.type = CRB_STRING_VALUE;
        value.u.object = CRB_create_crowbar_string(interpreter, env, wc_str);
    } else {
        value.type = CRB_NULL_VALUE;
    }
    MEM_free(mb_buf);

    return value;
}

static CRB_Value
nv_fputs_proc(CRB_Interpreter *interpreter,
              CRB_LocalEnvironment *env,
              int arg_count, CRB_Value *args)
{
    CRB_Value value;
    FILE *fp;

    CRB_check_argument_count(interpreter, env, arg_count, 2);
    value.type = CRB_NULL_VALUE;
    if (args[0].type != CRB_STRING_VALUE
        || args[1].type != CRB_NATIVE_POINTER_VALUE
        || (!CRB_check_native_pointer_type(args[1].u.object,
                                           &st_file_type_info))) {
        CRB_error(interpreter, env, &st_lib_info, __LINE__,
                  (int)FPUTS_ARGUMENT_TYPE_ERR,
                  CRB_MESSAGE_ARGUMENT_END);
    }
    check_file_pointer(interpreter,env, args[1].u.object);
    fp = CRB_object_get_native_pointer(args[1].u.object);

    CRB_print_wcs(fp, CRB_object_get_string(args[0].u.object));

    return value;
}

CRB_Value
new_array_sub(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
              int arg_count, CRB_Value *args, int arg_idx)
{
    CRB_Value ret;
    CRB_Value value;
    int size;
    int i;

    if (args[arg_idx].type != CRB_INT_VALUE) {
        CRB_error(inter, env, &st_lib_info, __LINE__,
                  (int)NEW_ARRAY_ARGUMENT_TYPE_ERR,
                  CRB_MESSAGE_ARGUMENT_END);
    }

    size = args[arg_idx].u.int_value;

    ret.type = CRB_ARRAY_VALUE;
    ret.u.object = CRB_create_array(inter, env, size);

    if (arg_idx == arg_count-1) {
        value.type = CRB_NULL_VALUE;
        for (i = 0; i < size; i++) {
            CRB_array_set(inter, env, ret.u.object, i, &value);
        }
    } else {
        for (i = 0; i < size; i++) {
            value = new_array_sub(inter, env,
                                  arg_count, args, arg_idx+1);
            CRB_array_set(inter, env, ret.u.object, i, &value);
        }
    }

    return ret;
}

static CRB_Value
nv_new_array_proc(CRB_Interpreter *interpreter,
                  CRB_LocalEnvironment *env,
                  int arg_count, CRB_Value *args)
{
    CRB_Value value;

    if (arg_count < 1) {
        CRB_error(interpreter, env, &st_lib_info, __LINE__,
                  (int)NEW_ARRAY_ARGUMENT_TOO_FEW_ERR,
                  CRB_MESSAGE_ARGUMENT_END);
    }

    value = new_array_sub(interpreter, env, arg_count, args, 0);

    return value;
}

static CRB_Value
nv_new_object_proc(CRB_Interpreter *interpreter,
                   CRB_LocalEnvironment *env,
                   int arg_count, CRB_Value *args)
{
    CRB_Value value;

    CRB_check_argument_count(interpreter, env, arg_count, 0);
    value.type = CRB_ASSOC_VALUE;
    value.u.object = CRB_create_assoc(interpreter, env);

    return value;
}

static CRB_Value
nv_new_exception_proc(CRB_Interpreter *interpreter,
                      CRB_LocalEnvironment *env,
                      int arg_count, CRB_Value *args)
{
    CRB_Value value;

    CRB_check_argument_count(interpreter, env, arg_count, 1);
    if (args[0].type != CRB_STRING_VALUE) {
        CRB_error(interpreter, env, &st_lib_info, __LINE__,
                  (int)NEW_EXCEPTION_ARGUMENT_ERR,
                  CRB_STRING_MESSAGE_ARGUMENT,
                  "type", CRB_get_type_name(args[0].type),
                  CRB_MESSAGE_ARGUMENT_END);
    }
    value.type = CRB_ASSOC_VALUE;
    value.u.object = CRB_create_exception(interpreter, env->next,
                                          args[0].u.object,
                                          env->caller_line_number);

    return value;
}

static CRB_Value
nv_exit_proc(CRB_Interpreter *interpreter,
             CRB_LocalEnvironment *env,
             int arg_count, CRB_Value *args)
{
    CRB_Value value;

    CRB_check_argument_count(interpreter, env, arg_count, 1);
    if (args[0].type != CRB_INT_VALUE) {
        CRB_error(interpreter, env, &st_lib_info, __LINE__,
                  (int)EXIT_ARGUMENT_TYPE_ERR,
                  CRB_STRING_MESSAGE_ARGUMENT,
                  "type", CRB_get_type_name(args[0].type),
                  CRB_MESSAGE_ARGUMENT_END);
    }

    exit(args[0].u.int_value);

    return value;
}

void
crb_add_native_functions(CRB_Interpreter *inter)
{
    CRB_add_native_function(inter, "print", nv_print_proc);
    CRB_add_native_function(inter, "fopen", nv_fopen_proc);
    CRB_add_native_function(inter, "fclose", nv_fclose_proc);
    CRB_add_native_function(inter, "fgets", nv_fgets_proc);
    CRB_add_native_function(inter, "fputs", nv_fputs_proc);
    CRB_add_native_function(inter, "new_array", nv_new_array_proc);
    CRB_add_native_function(inter, "new_object", nv_new_object_proc);
    CRB_add_native_function(inter, "new_exception", nv_new_exception_proc);
    CRB_add_native_function(inter, "exit", nv_exit_proc);
}

void
crb_add_std_fp(CRB_Interpreter *inter)
{
    CRB_Value fp_value;

    fp_value.type = CRB_NATIVE_POINTER_VALUE;
    fp_value.u.object = crb_create_native_pointer_i(inter, stdin,
                                                    &st_file_type_info);
    CRB_add_global_variable(inter, "STDIN", &fp_value, CRB_TRUE);

    fp_value.u.object = crb_create_native_pointer_i(inter, stdout,
                                                    &st_file_type_info);
    CRB_add_global_variable(inter, "STDOUT", &fp_value, CRB_TRUE);

    fp_value.u.object = crb_create_native_pointer_i(inter, stderr,
                                                    &st_file_type_info);
    CRB_add_global_variable(inter, "STDERR", &fp_value, CRB_TRUE);
}

