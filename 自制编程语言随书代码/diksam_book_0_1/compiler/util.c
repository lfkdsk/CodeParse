#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "diksamc.h"

static DKC_Compiler *st_current_compiler;

DKC_Compiler *
dkc_get_current_compiler(void)
{
    return st_current_compiler;
}

void
dkc_set_current_compiler(DKC_Compiler *compiler)
{
    st_current_compiler = compiler;
}

void *
dkc_malloc(size_t size)
{
    void *p;
    DKC_Compiler *compiler;

    compiler = dkc_get_current_compiler();
    p = MEM_storage_malloc(compiler->compile_storage, size);

    return p;
}

TypeSpecifier *
dkc_alloc_type_specifier(DVM_BasicType type)
{
    TypeSpecifier *ts = dkc_malloc(sizeof(TypeSpecifier));

    ts->basic_type = type;
    ts->derive = NULL;

    return ts;
}

FunctionDefinition *
dkc_search_function(char *name)
{
    DKC_Compiler *compiler;
    FunctionDefinition *pos;

    compiler = dkc_get_current_compiler();

    for (pos = compiler->function_list; pos; pos = pos->next) {
        if (!strcmp(pos->name, name))
            break;
    }
    return pos;
}

Declaration *
dkc_search_declaration(char *identifier, Block *block)
{
    Block *b_pos;
    DeclarationList *d_pos;
    DKC_Compiler *compiler;

    for (b_pos = block; b_pos; b_pos = b_pos->outer_block) {
        for (d_pos = b_pos->declaration_list; d_pos; d_pos = d_pos->next) {
            if (!strcmp(identifier, d_pos->declaration->name)) {
                return d_pos->declaration;
            }
        }
    }

    compiler = dkc_get_current_compiler();

    for (d_pos = compiler->declaration_list; d_pos; d_pos = d_pos->next) {
        if (!strcmp(identifier, d_pos->declaration->name)) {
            return d_pos->declaration;
        }
    }

    return NULL;
}

void
dkc_vstr_clear(VString *v)
{
    v->string = NULL;
}

static int
my_strlen(DVM_Char *str)
{
    if (str == NULL) {
        return 0;
    }
    return dvm_wcslen(str);
}

void
dkc_vstr_append_string(VString *v, DVM_Char *str)
{
    int new_size;
    int old_len;

    old_len = my_strlen(v->string);
    new_size = sizeof(DVM_Char) * (old_len + dvm_wcslen(str)  + 1);
    v->string = MEM_realloc(v->string, new_size);
    dvm_wcscpy(&v->string[old_len], str);
}

void
dkc_vstr_append_character(VString *v, DVM_Char ch)
{
    int current_len;
    
    current_len = my_strlen(v->string);
    v->string = MEM_realloc(v->string,sizeof(DVM_Char) * (current_len + 2));
    v->string[current_len] = ch;
    v->string[current_len+1] = L'\0';
}

char *
dkc_get_basic_type_name(DVM_BasicType type)
{
    switch (type) {
    case DVM_BOOLEAN_TYPE:
        return "boolean";
        break;
    case DVM_INT_TYPE:
        return "int";
        break;
    case DVM_DOUBLE_TYPE:
        return "double";
        break;
    case DVM_STRING_TYPE:
        return "string";
        break;
    default:
        DBG_assert(0, ("bad case. type..%d\n", type));
    }
    return NULL;
}

DVM_Char *
dkc_expression_to_string(Expression *expr)
{
    char        buf[LINE_BUF_SIZE];
    DVM_Char    wc_buf[LINE_BUF_SIZE];
    int         len;
    DVM_Char    *new_str;

    if (expr->kind == BOOLEAN_EXPRESSION) {
        if (expr->u.boolean_value) {
            dvm_mbstowcs("true", wc_buf);
        } else {
            dvm_mbstowcs("false", wc_buf);
        }
    } else if (expr->kind == INT_EXPRESSION) {
        sprintf(buf, "%d", expr->u.int_value);
        dvm_mbstowcs(buf, wc_buf);
    } else if (expr->kind == DOUBLE_EXPRESSION) {
        sprintf(buf, "%f", expr->u.double_value);
        dvm_mbstowcs(buf, wc_buf);
    } else if (expr->kind == STRING_EXPRESSION) {
        return expr->u.string_value;
    } else {
        return NULL;
    }
    len = dvm_wcslen(wc_buf);
    new_str = MEM_malloc(sizeof(DVM_Char) * (len + 1));
    dvm_wcscpy(new_str, wc_buf);

    return new_str;
}
