#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

extern char *yytext;
extern MessageFormat crb_compile_error_message_format[];
extern MessageFormat crb_runtime_error_message_format[];

typedef struct {
    char        *string;
} VString;

static void
clear_v_string(VString *v)
{
    v->string = NULL;
}

int
my_strlen(char *str)
{
    if (str == NULL) {
        return 0;
    }
    return strlen(str);
}

static void
add_string(VString *v, char *str)
{
    int new_size;
    int old_len;

    old_len = my_strlen(v->string);
    new_size = old_len + strlen(str) + 1;
    v->string = MEM_realloc(v->string, new_size);
    strcpy(&v->string[old_len], str);
}

static void
add_character(VString *v, int ch)
{
    int current_len;
    
    current_len = my_strlen(v->string);
    v->string = MEM_realloc(v->string, current_len + 2);
    v->string[current_len] = ch;
    v->string[current_len+1] = '\0';
}

typedef struct {
    MessageArgumentType type;
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
    MessageArgumentType type;
    
    while ((type = va_arg(ap, MessageArgumentType)) != MESSAGE_ARGUMENT_END) {
        arg[index].type = type;
        arg[index].name = va_arg(ap, char*);
        switch (type) {
        case INT_MESSAGE_ARGUMENT:
            arg[index].u.int_val = va_arg(ap, int);
            break;
        case DOUBLE_MESSAGE_ARGUMENT:
            arg[index].u.double_val = va_arg(ap, double);
            break;
        case STRING_MESSAGE_ARGUMENT:
            arg[index].u.string_val = va_arg(ap, char*);
            break;
        case POINTER_MESSAGE_ARGUMENT:
            arg[index].u.pointer_val = va_arg(ap, void*);
            break;
        case CHARACTER_MESSAGE_ARGUMENT:
            arg[index].u.character_val = va_arg(ap, int);
            break;
        case MESSAGE_ARGUMENT_END:
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

    for (i = 0; arg_list[i].type != MESSAGE_ARGUMENT_END; i++) {
        if (!strcmp(arg_list[i].name, arg_name)) {
            *arg = arg_list[i];
            return;
        }
    }
    assert(0);
}

static void
format_message(MessageFormat *format, VString *v, va_list ap)
{
    int         i;
    char        buf[LINE_BUF_SIZE];
    int         arg_name_index;
    char        arg_name[LINE_BUF_SIZE];
    MessageArgument     arg[MESSAGE_ARGUMENT_MAX];
    MessageArgument     cur_arg;

    create_message_argument(arg, ap);

    for (i = 0; format->format[i] != '\0'; i++) {
        if (format->format[i] != '$') {
            add_character(v, format->format[i]);
            continue;
        }
        assert(format->format[i+1] == '(');
        i += 2;
        for (arg_name_index = 0; format->format[i] != ')';
             arg_name_index++, i++) {
            arg_name[arg_name_index] = format->format[i];
        }
        arg_name[arg_name_index] = '\0';
        assert(format->format[i] == ')');

        search_argument(arg, arg_name, &cur_arg);
        switch (cur_arg.type) {
        case INT_MESSAGE_ARGUMENT:
            sprintf(buf, "%d", cur_arg.u.int_val);
            add_string(v, buf);
            break;
        case DOUBLE_MESSAGE_ARGUMENT:
            sprintf(buf, "%f", cur_arg.u.double_val);
            add_string(v, buf);
            break;
        case STRING_MESSAGE_ARGUMENT:
            strcpy(buf, cur_arg.u.string_val);
            add_string(v, cur_arg.u.string_val);
            break;
        case POINTER_MESSAGE_ARGUMENT:
            sprintf(buf, "%p", cur_arg.u.pointer_val);
            add_string(v, buf);
            break;
        case CHARACTER_MESSAGE_ARGUMENT:
            sprintf(buf, "%c", cur_arg.u.character_val);
            add_string(v, buf);
            break;
        case MESSAGE_ARGUMENT_END:
            assert(0);
            break;
        default:
            assert(0);
        }
    }
}

void
self_check()
{
    if (strcmp(crb_compile_error_message_format[0].format, "dummy") != 0) {
        DBG_panic(("compile error message format error.\n"));
    }
    if (strcmp(crb_compile_error_message_format
               [COMPILE_ERROR_COUNT_PLUS_1].format,
               "dummy") != 0) {
        DBG_panic(("compile error message format error. "
                   "COMPILE_ERROR_COUNT_PLUS_1..%d\n",
                   COMPILE_ERROR_COUNT_PLUS_1));
    }
    if (strcmp(crb_runtime_error_message_format[0].format, "dummy") != 0) {
        DBG_panic(("runtime error message format error.\n"));
    }
    if (strcmp(crb_runtime_error_message_format
               [RUNTIME_ERROR_COUNT_PLUS_1].format,
               "dummy") != 0) {
        DBG_panic(("runtime error message format error. "
                   "RUNTIME_ERROR_COUNT_PLUS_1..%d\n",
                   RUNTIME_ERROR_COUNT_PLUS_1));
    }
}

void
crb_compile_error(CompileError id, ...)
{
    va_list     ap;
    VString     message;
    int         line_number;

    self_check();
    va_start(ap, id);
    line_number = crb_get_current_interpreter()->current_line_number;
    clear_v_string(&message);
    format_message(&crb_compile_error_message_format[id],
                   &message, ap);
    fprintf(stderr, "%3d:%s\n", line_number, message.string);
    va_end(ap);

    exit(1);
}

void
crb_runtime_error(int line_number, RuntimeError id, ...)
{
    va_list     ap;
    VString     message;

    self_check();
    va_start(ap, id);
    clear_v_string(&message);
    format_message(&crb_runtime_error_message_format[id],
                   &message, ap);
    fprintf(stderr, "%3d:%s\n", line_number, message.string);
    va_end(ap);

    exit(1);
}


int
yyerror(char const *str)
{
    char *near_token;

    if (yytext[0] == '\0') {
        near_token = "EOF";
    } else {
        near_token = yytext;
    }
    crb_compile_error(PARSE_ERR,
                      STRING_MESSAGE_ARGUMENT, "token", near_token,
                      MESSAGE_ARGUMENT_END);

    return 0;
}
