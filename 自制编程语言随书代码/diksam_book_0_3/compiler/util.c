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

char *
dkc_strdup(char *src)
{
    char *p;
    DKC_Compiler *compiler;

    compiler = dkc_get_current_compiler();
    p = MEM_storage_malloc(compiler->compile_storage, strlen(src)+1);
    strcpy(p, src);

    return p;
}

TypeSpecifier *
dkc_alloc_type_specifier(DVM_BasicType type)
{
    TypeSpecifier *ts = dkc_malloc(sizeof(TypeSpecifier));

    ts->basic_type = type;
    ts->line_number = 0;
    ts->derive = NULL;
    if (type == DVM_CLASS_TYPE) {
        ts->class_ref.identifier = NULL;
        ts->class_ref.class_definition = NULL;
    }

    return ts;
}

TypeDerive *
dkc_alloc_type_derive(DeriveTag derive_tag)
{
    TypeDerive *td = dkc_malloc(sizeof(TypeDerive));
    td->tag = derive_tag;
    td->next = NULL;

    return td;
}

TypeSpecifier *
dkc_alloc_type_specifier2(TypeSpecifier *src)
{
    TypeSpecifier *ts = dkc_malloc(sizeof(TypeSpecifier));

    *ts = *src;

    return ts;
}

DVM_Boolean
dkc_compare_parameter(ParameterList *param1, ParameterList *param2)
{
    ParameterList *pos1;
    ParameterList *pos2;

    for (pos1 = param1, pos2 = param2; pos1 && pos2;
         pos1 = pos1->next, pos2 = pos2->next) {
        if (strcmp(pos1->name, pos2->name) != 0) {
            return DVM_FALSE;
        }
        if (!dkc_compare_type(pos1->type, pos2->type)) {
            return DVM_FALSE;
        }
    }
    if (pos1 || pos2) {
        return DVM_FALSE;
    }

    return DVM_TRUE;
}

DVM_Boolean
dkc_compare_type(TypeSpecifier *type1, TypeSpecifier *type2)
{
    TypeDerive *d1;
    TypeDerive *d2;

    if (type1->basic_type != type2->basic_type) {
        return DVM_FALSE;
    }

    /* BUGBUG? */
    if (type1->basic_type == DVM_CLASS_TYPE) {
        if (type1->class_ref.class_definition
            != type2->class_ref.class_definition) {
            return DVM_FALSE;
        }
    }

    for (d1 = type1->derive, d2 = type2->derive;
         d1 && d2; d1 = d1->next, d2 = d2->next) {
        if (d1->tag != d2->tag) {
            return DVM_FALSE;
        }
        if (d1->tag == FUNCTION_DERIVE) {
            if (!dkc_compare_parameter(d1->u.function_d.parameter_list,
                                       d2->u.function_d.parameter_list)) {
                return DVM_FALSE;
            }
        }
    }
    if (d1 || d2) {
        return DVM_FALSE;
    }

    return DVM_TRUE;
}

DVM_Boolean
dkc_compare_package_name(PackageName *p1, PackageName *p2)
{
    PackageName *pos1;
    PackageName *pos2;

    for (pos1 = p1, pos2 = p2; pos1 && pos2;
         pos1 = pos1->next, pos2 = pos2->next) {
        if (strcmp(pos1->name, pos2->name) != 0) {
            return DVM_FALSE;
        }
    }
    if (pos1 || pos2) {
        return DVM_FALSE;
    }

    return DVM_TRUE;
}

static FunctionDefinition *
search_renamed_function(DKC_Compiler *compiler, RenameList *rename)
{
    FunctionDefinition *func_pos;
    CompilerList *comp_pos;

    for (comp_pos = compiler->required_list; comp_pos;
         comp_pos = comp_pos->next) {
        if (!dkc_compare_package_name(rename->package_name,
                                      comp_pos->compiler->package_name)) {
            continue;
        }
        for (func_pos = comp_pos->compiler->function_list; func_pos;
             func_pos = func_pos->next) {
            if (!strcmp(func_pos->name, rename->original_name)
                && func_pos->class_definition == NULL) {
                return func_pos;
            }
        }
    }
    return NULL;
}

FunctionDefinition *
dkc_search_function(char *name)
{
    DKC_Compiler *compiler;
    RenameList *ren_pos;
    CompilerList *comp_pos;
    FunctionDefinition *func_pos;

    compiler = dkc_get_current_compiler();

    for (func_pos = compiler->function_list; func_pos;
         func_pos = func_pos->next) {
        if (!strcmp(func_pos->name, name)
            && func_pos->class_definition == NULL) {
            return func_pos;
        }
    }

    for (ren_pos = compiler->rename_list; ren_pos; ren_pos = ren_pos->next) {
        if (!strcmp(ren_pos->renamed_name, name)) {
            FunctionDefinition * fd
                = search_renamed_function(compiler, ren_pos);
            if (fd) {
                return fd;
            }
        }
    }

    for (comp_pos = compiler->required_list; comp_pos;
         comp_pos = comp_pos->next) {
        for (func_pos = comp_pos->compiler->function_list; func_pos;
             func_pos = func_pos->next) {
            if (!strcmp(func_pos->name, name)
                && func_pos->class_definition == NULL) {
                return func_pos;
            }
        }
    }

    return NULL;
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

static ClassDefinition *
search_renamed_class(DKC_Compiler *compiler, RenameList *rename)
{
    ClassDefinition *class_pos;
    CompilerList *comp_pos;

    for (comp_pos = compiler->required_list; comp_pos;
         comp_pos = comp_pos->next) {
        if (!dkc_compare_package_name(rename->package_name,
                                      comp_pos->compiler->package_name)) {
            continue;
        }
        for (class_pos = comp_pos->compiler->class_definition_list; class_pos;
             class_pos = class_pos->next) {
            if (!strcmp(class_pos->name, rename->original_name)) {
                return class_pos;
            }
        }
    }
    return NULL;
}

ClassDefinition *
dkc_search_class(char *identifier)
{
    DKC_Compiler *compiler;
    RenameList *ren_pos;
    CompilerList *comp_pos;
    ClassDefinition *class_def;

    compiler = dkc_get_current_compiler();
    for (class_def = compiler->class_definition_list;
         class_def; class_def = class_def->next) {
        if (!strcmp(class_def->name, identifier)) {
            return class_def;
        }
    }

    for (ren_pos = compiler->rename_list; ren_pos; ren_pos = ren_pos->next) {
        if (!strcmp(ren_pos->renamed_name, identifier)) {
            class_def = search_renamed_class(compiler, ren_pos);
            if (class_def) {
                return class_def;
            }
        }
    }

    for (comp_pos = compiler->required_list; comp_pos;
         comp_pos = comp_pos->next) {
        for (class_def = comp_pos->compiler->class_definition_list;
             class_def; class_def = class_def->next) {
            if (!strcmp(class_def->name, identifier)) {
                return class_def;
            }
        }
    }

    return NULL;
}

MemberDeclaration *
dkc_search_member(ClassDefinition *class_def,
                  char *member_name)
{
    MemberDeclaration *member;
    ExtendsList *extends_p;
    
    for (member = class_def->member; member;
         member = member->next) {
        if (member->kind == METHOD_MEMBER) {
            if (!strcmp(member->u.method.function_definition->name,
                        member_name)) {
                break;
            }
        } else {
            DBG_assert(member->kind == FIELD_MEMBER,
                       ("member..%d", member->kind));
            if (!strcmp(member->u.field.name, member_name)) {
                break;
            }
        }
    }
    if (member) {
        return member;
    }
    if (class_def->super_class) {
        member = dkc_search_member(class_def->super_class, member_name);
    }
    if (member) {
        return member;
    }
    for (extends_p = class_def->interface_list;
         extends_p;
         extends_p = extends_p->next) {
        member = dkc_search_member(extends_p->class_definition, member_name);
        if (member) {
            return member;
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
my_strlen(char *str)
{
    if (str == NULL) {
        return 0;
    }
    return strlen(str);
}

void
dkc_vstr_append_string(VString *v, char *str)
{
    int new_size;
    int old_len;

    old_len = my_strlen(v->string);
    new_size = old_len + strlen(str)  + 1;
    v->string = MEM_realloc(v->string, new_size);
    strcpy(&v->string[old_len], str);
}

void
dkc_vstr_append_character(VString *v, int ch)
{
    int current_len;
    
    current_len = my_strlen(v->string);
    v->string = MEM_realloc(v->string, current_len + 2);
    v->string[current_len] = ch;
    v->string[current_len+1] = '\0';
}

void
dkc_vwstr_clear(VWString *v)
{
    v->string = NULL;
}

static int
my_wcslen(DVM_Char *str)
{
    if (str == NULL) {
        return 0;
    }
    return dvm_wcslen(str);
}

void
dkc_vwstr_append_string(VWString *v, DVM_Char *str)
{
    int new_size;
    int old_len;

    old_len = my_wcslen(v->string);
    new_size = sizeof(DVM_Char) * (old_len + dvm_wcslen(str)  + 1);
    v->string = MEM_realloc(v->string, new_size);
    dvm_wcscpy(&v->string[old_len], str);
}

void
dkc_vwstr_append_character(VWString *v, int ch)
{
    int current_len;
    
    current_len = my_wcslen(v->string);
    v->string = MEM_realloc(v->string,sizeof(DVM_Char) * (current_len + 2));
    v->string[current_len] = ch;
    v->string[current_len+1] = L'\0';
}

char *
dkc_get_basic_type_name(DVM_BasicType type)
{
    switch (type) {
    case DVM_VOID_TYPE:
        return "void";
        break;
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
    case DVM_CLASS_TYPE:
        return "class";
        break;
    case DVM_NULL_TYPE:
        return "null";
        break;
    case DVM_BASE_TYPE: /* FALLTHRU */
    default:
        DBG_assert(0, ("bad case. type..%d\n", type));
    }
    return NULL;
}

static void
function_type_to_string(VString *vstr, TypeDerive *derive)
{
    ParameterList *param_pos;

    dkc_vstr_append_string(vstr, "(");
    for (param_pos = derive->u.function_d.parameter_list; param_pos;
         param_pos = param_pos->next) {
        char *type_name = dkc_get_type_name(param_pos->type);
        dkc_vstr_append_string(vstr, type_name);
        dkc_vstr_append_string(vstr, " ");
        dkc_vstr_append_string(vstr, param_pos->name);
        if (param_pos->next) {
            dkc_vstr_append_string(vstr, ", ");
        }
    }
    dkc_vstr_append_string(vstr, ")");
}

char *
dkc_get_type_name(TypeSpecifier *type)
{
    VString     vstr;
    TypeDerive  *derive_pos;

    dkc_vstr_clear(&vstr);

    if (type->basic_type == DVM_CLASS_TYPE) {
        dkc_vstr_append_string(&vstr, type->class_ref.identifier);
    } else {
        dkc_vstr_append_string(&vstr,
                               dkc_get_basic_type_name(type->basic_type));
    }

    for (derive_pos = type->derive; derive_pos;
         derive_pos = derive_pos->next) {
        switch (derive_pos->tag) {
        case FUNCTION_DERIVE:
            function_type_to_string(&vstr, derive_pos);
            break;
        case ARRAY_DERIVE:
            dkc_vstr_append_string(&vstr, "[]");
            break;
        default:
            DBG_assert(0, ("derive_tag..%d\n", derive_pos->tag));
        }
    }

    return vstr.string;
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

char *
dkc_package_name_to_string(PackageName *src)
{
    int len = 0;
    PackageName *pos;
    char *dest;

    if (src == NULL) {
        return NULL;
    }
    for (pos = src; pos; pos = pos->next) {
        len += strlen(pos->name) + 1;
    }


    dest = MEM_malloc(len);
    dest[0] = '\0';
    for (pos = src; pos; pos = pos->next) {
        strcat(dest, pos->name);
        if (pos->next) {
            strcat(dest, ".");
        }
    }

    return dest;
}
