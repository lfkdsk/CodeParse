#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "CRB_dev.h"
#include "crowbar.h"

#define NATIVE_LIB_NAME "crowbar.lang.file"

static CRB_NativePointerInfo st_native_lib_info = {
    NATIVE_LIB_NAME
};

static void
check_argument_count(int arg_count, int true_count)
{
    if (arg_count < true_count) {
        crb_runtime_error(0, ARGUMENT_TOO_FEW_ERR,
                          MESSAGE_ARGUMENT_END);
    } else if (arg_count > true_count) {
        crb_runtime_error(0, ARGUMENT_TOO_MANY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
}

CRB_Value
crb_nv_print_proc(CRB_Interpreter *interpreter,
                  CRB_LocalEnvironment *env,
                  int arg_count, CRB_Value *args)
{
    CRB_Value value;
    CRB_Char *str;

    value.type = CRB_NULL_VALUE;

    check_argument_count(arg_count, 1);
    str = CRB_value_to_string(&args[0]);
    CRB_print_wcs(stdout, str);
    MEM_free(str);

    return value;
}

CRB_Value
crb_nv_fopen_proc(CRB_Interpreter *interpreter,
                  CRB_LocalEnvironment *env,
                  int arg_count, CRB_Value *args)
{
    CRB_Value value;
    char *filename;
    char *mode;
    FILE *fp;

    check_argument_count(arg_count, 2);

    if (args[0].type != CRB_STRING_VALUE
        || args[1].type != CRB_STRING_VALUE) {
        crb_runtime_error(0, FOPEN_ARGUMENT_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    
    filename = CRB_wcstombs_alloc(args[0].u.object->u.string.string);
    mode = CRB_wcstombs_alloc(args[1].u.object->u.string.string);

    fp = fopen(filename, mode);
    if (fp == NULL) {
        value.type = CRB_NULL_VALUE;
    } else {
        value.type = CRB_NATIVE_POINTER_VALUE;
        value.u.native_pointer.info = &st_native_lib_info;
        value.u.native_pointer.pointer = fp;
    }
    MEM_free(filename);
    MEM_free(mode);

    return value;
}

static CRB_Boolean
check_native_pointer(CRB_Value *value)
{
    return value->u.native_pointer.info == &st_native_lib_info;
}

CRB_Value
crb_nv_fclose_proc(CRB_Interpreter *interpreter,
                   CRB_LocalEnvironment *env,
                   int arg_count, CRB_Value *args)
{
    CRB_Value value;
    FILE *fp;

    check_argument_count(arg_count, 1);

    value.type = CRB_NULL_VALUE;
    if (args[0].type != CRB_NATIVE_POINTER_VALUE
        || !check_native_pointer(&args[0])) {
        crb_runtime_error(0, FCLOSE_ARGUMENT_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    fp = args[0].u.native_pointer.pointer;
    fclose(fp);

    return value;
}

CRB_Value
crb_nv_fgets_proc(CRB_Interpreter *interpreter,
                  CRB_LocalEnvironment *env,
                  int arg_count, CRB_Value *args)
{
    CRB_Value value;
    FILE *fp;
    char buf[LINE_BUF_SIZE];
    char *mb_buf = NULL;
    int ret_len = 0;
    CRB_Char *wc_str;

    check_argument_count(arg_count, 1);

    if (args[0].type != CRB_NATIVE_POINTER_VALUE
        || !check_native_pointer(&args[0])) {
        crb_runtime_error(0, FGETS_ARGUMENT_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    fp = args[0].u.native_pointer.pointer;

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
            crb_runtime_error(0,
                              BAD_MULTIBYTE_CHARACTER_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        value.type = CRB_STRING_VALUE;
        value.u.object = CRB_create_crowbar_string(interpreter, env, wc_str);
    } else {
        value.type = CRB_NULL_VALUE;
    }
    MEM_free(mb_buf);

    return value;
}

CRB_Value
crb_nv_fputs_proc(CRB_Interpreter *interpreter,
                  CRB_LocalEnvironment *env,
                  int arg_count, CRB_Value *args)
{
    CRB_Value value;
    FILE *fp;

    check_argument_count(arg_count, 2);
    value.type = CRB_NULL_VALUE;
    if (args[0].type != CRB_STRING_VALUE
        || (args[1].type != CRB_NATIVE_POINTER_VALUE
            || !check_native_pointer(&args[1]))) {
        crb_runtime_error(0, FPUTS_ARGUMENT_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    fp = args[1].u.native_pointer.pointer;

    CRB_print_wcs(fp, args[0].u.object->u.string.string);

    return value;
}

void
crb_add_std_fp(CRB_Interpreter *inter)
{
    CRB_Value fp_value;

    fp_value.type = CRB_NATIVE_POINTER_VALUE;
    fp_value.u.native_pointer.info = &st_native_lib_info;

    fp_value.u.native_pointer.pointer = stdin;
    CRB_add_global_variable(inter, "STDIN", &fp_value);

    fp_value.u.native_pointer.pointer = stdout;
    CRB_add_global_variable(inter, "STDOUT", &fp_value);

    fp_value.u.native_pointer.pointer = stderr;
    CRB_add_global_variable(inter, "STDERR", &fp_value);
}


CRB_Value
new_array_sub(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
              int arg_count, CRB_Value *args, int arg_idx)
{
    CRB_Value ret;
    int size;
    int i;

    if (args[arg_idx].type != CRB_INT_VALUE) {
        crb_runtime_error(0, NEW_ARRAY_ARGUMENT_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    size = args[arg_idx].u.int_value;

    ret.type = CRB_ARRAY_VALUE;
    ret.u.object = CRB_create_array(inter, env, size);

    if (arg_idx == arg_count-1) {
        for (i = 0; i < size; i++) {
            ret.u.object->u.array.array[i].type = CRB_NULL_VALUE;
        }
    } else {
        for (i = 0; i < size; i++) {
            ret.u.object->u.array.array[i]
                = new_array_sub(inter, env, arg_count, args, arg_idx+1);
        }
    }

    return ret;
}

CRB_Value
crb_nv_new_array_proc(CRB_Interpreter *interpreter,
                      CRB_LocalEnvironment *env,
                      int arg_count, CRB_Value *args)
{
    CRB_Value value;

    if (arg_count < 1) {
        crb_runtime_error(0, ARGUMENT_TOO_FEW_ERR,
                          MESSAGE_ARGUMENT_END);
    }

    value = new_array_sub(interpreter, env, arg_count, args, 0);

    return value;

}
