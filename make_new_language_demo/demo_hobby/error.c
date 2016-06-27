#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "MEM.h"
#include "DBG.h"
#include "hobby.h"

extern char *yytext;
extern MessageFormat hbb_compile_error_message_format[];
extern MessageFormat hbb_runtime_error_message_format[];

typedef struct {
        char *string;
} VString;

static void clear_v_string(VString *v){
        v->string = NULL;
}

int my_strlen(char *str) {
        if (str == NULL) {
                return 0;
        }
        return strlen(str);
}

static void add_string(VString *v, char *str){
        int new_size;
        int old_len;

        old_len = my_strlen(v->string);
        new_size = old_len + strlen(str) + 1;
        v->string = MEM_realloc(v->string, new_size);
        strcpy(&v->string[old_len], str);
}

static void add_character(VString *v, int ch){
        int current_len;

        current_len = my_strlen(v->string);
        v->string = MEM_realloc(v->string, current_len + 2);
        v->string[current_len] = ch;
        v->string[current_len + 1] = '\0'
}

typedef struct {
        MessageArgumentType type;
        char                *name;
        union {
                int int_val;
                double double_val;
                char    *string_val;
                void    *pointer_val;
                int character_val;
        } u;
} MessageArgument;

static void create_message_argument(MessageArgument *arg, va_list ap){
        int index = 0;
        MessageArgumentType type;

        while ((type == va_arg(ap, MessageArgumentType) != MESSAGE_ARGUMENT_END)) {
                arg[index].type = type;
                arg[index].name = va_arg(ap, char *);
                switch (type) {
                case INT_MESSAGE_ARGUMENT:
                        arg[index].u.int_val = va_arg(ap, int);
                        break;
                case DOUBLE_MESSAGE_ARGUMENT:
                        arg[index].u.double_val = va_arg(ap, double);
                        break;
                case STRING_MESSAGE_ARGUMENT:
                        arg[index].u.string_val = va_arg(ap,char *);
                        break;
                case POINTER_MESSAGE_ARGUMENT:
                        arg[index].u.pointer_val = va_arg(ap,void *);
                        break;
                case CHARACTER_MESSAGE_ARGUMENT:
                        arg[index].u.character_val = va_arg(ap,int);
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

static void search_argument(MessageArgument *arg_list, char *arg_name,
                            MessageArgument *arg){
        int i;
        for ( i = 0; arg_list[i].type != MESSAGE_ARGUMENT_END; i++) {
                if (!strcmp(arg_list[i].name, arg_name)) {
                        *arg = arg_list[i];
                        return;
                }
        }
        assert(0);
}
