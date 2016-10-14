#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

typedef struct {
    DVM_MessageArgumentType type;
    char        *name;
    union {
        int     int_val;
        double  double_val;
        char    *string_val;
        void    *pointer_val;
        int     character_val;
    } u;
} MessageArgument;

static void
create_message_argument(MessageArgument *arg, va_list ap)
{
    int index = 0;
    DVM_MessageArgumentType type;
    
    while ((type = va_arg(ap, DVM_MessageArgumentType))
           != DVM_MESSAGE_ARGUMENT_END) {
        arg[index].type = type;
        arg[index].name = va_arg(ap, char*);
        switch (type) {
        case DVM_INT_MESSAGE_ARGUMENT:
            arg[index].u.int_val = va_arg(ap, int);
            break;
        case DVM_DOUBLE_MESSAGE_ARGUMENT:
            arg[index].u.double_val = va_arg(ap, double);
            break;
        case DVM_STRING_MESSAGE_ARGUMENT:
            arg[index].u.string_val = va_arg(ap, char*);
            break;
        case DVM_POINTER_MESSAGE_ARGUMENT:
            arg[index].u.pointer_val = va_arg(ap, void*);
            break;
        case DVM_CHARACTER_MESSAGE_ARGUMENT:
            arg[index].u.character_val = va_arg(ap, int);
            break;
        case DVM_MESSAGE_ARGUMENT_END:
            assert(0);
            break;
        default:
            assert(0);
        }
        index++;
        assert(index < MESSAGE_ARGUMENT_MAX);
    }
}

static void
search_argument(MessageArgument *arg_list,
                char *arg_name, MessageArgument *arg)
{
    int i;

    for (i = 0; arg_list[i].type != DVM_MESSAGE_ARGUMENT_END; i++) {
        if (!strcmp(arg_list[i].name, arg_name)) {
            *arg = arg_list[i];
            return;
        }
    }
    assert(0);
}

static void
format_message(DVM_ErrorDefinition *format, VString *v,
               va_list ap)
{
    int         i;
    char        buf[LINE_BUF_SIZE];
    DVM_Char    wc_buf[LINE_BUF_SIZE];
    int         arg_name_index;
    char        arg_name[LINE_BUF_SIZE];
    MessageArgument     arg[MESSAGE_ARGUMENT_MAX];
    MessageArgument     cur_arg;
    DVM_Char    *wc_format;

    create_message_argument(arg, ap);

    wc_format = dvm_mbstowcs_alloc(NULL, format->format);
    DBG_assert(wc_format != NULL, ("wc_format is null.\n"));
    
    for (i = 0; wc_format[i] != L'\0'; i++) {
        if (wc_format[i] != L'$') {
            dvm_vstr_append_character(v, wc_format[i]);
            continue;
        }
        assert(wc_format[i+1] == L'(');
        i += 2;
        for (arg_name_index = 0; wc_format[i] != L')';
             arg_name_index++, i++) {
            arg_name[arg_name_index] = dvm_wctochar(wc_format[i]);
        }
        arg_name[arg_name_index] = '\0';
        assert(wc_format[i] == L')');

        search_argument(arg, arg_name, &cur_arg);
        switch (cur_arg.type) {
        case DVM_INT_MESSAGE_ARGUMENT:
            sprintf(buf, "%d", cur_arg.u.int_val);
            dvm_mbstowcs(buf, wc_buf);
            dvm_vstr_append_string(v, wc_buf);
            break;
        case DVM_DOUBLE_MESSAGE_ARGUMENT:
            sprintf(buf, "%f", cur_arg.u.double_val);
            dvm_mbstowcs(buf, wc_buf);
            dvm_vstr_append_string(v, wc_buf);
            break;
        case DVM_STRING_MESSAGE_ARGUMENT:
            dvm_mbstowcs(cur_arg.u.string_val, wc_buf);
            dvm_vstr_append_string(v, wc_buf);
            break;
        case DVM_POINTER_MESSAGE_ARGUMENT:
            sprintf(buf, "%p", cur_arg.u.pointer_val);
            dvm_mbstowcs(buf, wc_buf);
            dvm_vstr_append_string(v, wc_buf);
            break;
        case DVM_CHARACTER_MESSAGE_ARGUMENT:
            sprintf(buf, "%c", cur_arg.u.character_val);
            dvm_mbstowcs(buf, wc_buf);
            dvm_vstr_append_string(v, wc_buf);
            break;
        case DVM_MESSAGE_ARGUMENT_END:
            assert(0);
            break;
        default:
            assert(0);
        }
    }
    MEM_free(wc_format);
}

static void
self_check()
{
    if (strcmp(dvm_error_message_format[0].format, "dummy") != 0) {
        DBG_panic(("runtime error message format error.\n"));
    }
    if (strcmp(dvm_error_message_format
               [RUNTIME_ERROR_COUNT_PLUS_1].format,
               "dummy") != 0) {
        DBG_panic(("runtime error message format error. "
                   "RUNTIME_ERROR_COUNT_PLUS_1..%d\n",
                   RUNTIME_ERROR_COUNT_PLUS_1));
    }
}

int
dvm_conv_pc_to_line_number(DVM_Executable *exe, Function *func, int pc)
{
    DVM_LineNumber *line_number;
    int line_number_size;
    int i;
    int ret;

    if (func) {
        line_number
            = exe->function[func->u.diksam_f.index].line_number;
        line_number_size
            = exe->function[func->u.diksam_f.index].line_number_size;
    } else {
        line_number = exe->line_number;
        line_number_size = exe->line_number_size;
    }

    for (i = 0; i < line_number_size; i++) {
        if (pc >= line_number[i].start_pc
            && pc < line_number[i].start_pc + line_number[i].pc_count) {
            ret = line_number[i].line_number;
        }
    }

    return ret;
}

static void
error_v(DVM_Executable *exe, Function *func, int pc,
        DVM_ErrorDefinition *error_def, RuntimeError id, va_list ap)
{
    VString     message;
    int         line_number;

    dvm_vstr_clear(&message);
    format_message(&error_def[id], &message, ap);

    if (pc != NO_LINE_NUMBER_PC) {
        line_number = dvm_conv_pc_to_line_number(exe, func, pc);
        fprintf(stderr, "%s:%d:", exe->path, line_number);
    }
    dvm_print_wcs_ln(stderr, message.string);
}

void
dvm_error_i(DVM_Executable *exe, Function *func, int pc,
            RuntimeError id, ...)
{
    va_list     ap;

    self_check();
    va_start(ap, id);
    error_v(exe, func, pc, dvm_error_message_format, id, ap);
    va_end(ap);

    exit(1);
}

void
dvm_error_n(DVM_VirtualMachine *dvm, DVM_ErrorDefinition *error_def,
            RuntimeError id, ...)
{
    va_list     ap;

    self_check();
    va_start(ap, id);
    error_v(dvm->current_executable->executable,
            dvm->current_function, dvm->pc, error_def, id, ap);
    va_end(ap);

    exit(1);
}

void
dvm_format_message(DVM_ErrorDefinition *error_definition,
                   int id, VString *message, va_list ap)
{
    dvm_vstr_clear(message);
    format_message(&error_definition[id],
                   message, ap);
}
