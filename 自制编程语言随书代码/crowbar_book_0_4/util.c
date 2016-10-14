#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static CRB_Interpreter *st_current_interpreter;

CRB_Interpreter *
crb_get_current_interpreter(void)
{
    return st_current_interpreter;
}

void
crb_set_current_interpreter(CRB_Interpreter *inter)
{
    st_current_interpreter = inter;
}

CRB_FunctionDefinition *
CRB_search_function(CRB_Interpreter *inter, char *name)
{
    CRB_FunctionDefinition *pos;

    for (pos = inter->function_list; pos; pos = pos->next) {
        if (!strcmp(pos->name, name))
            break;
    }
    return pos;
}

CRB_FunctionDefinition *
crb_search_function_in_compile(char *name)
{
    CRB_Interpreter *inter;

    inter = crb_get_current_interpreter();
    return CRB_search_function(inter, name);
}

void *
crb_malloc(size_t size)
{
    void *p;
    CRB_Interpreter *inter;

    inter = crb_get_current_interpreter();
    p = MEM_storage_malloc(inter->interpreter_storage, size);

    return p;
}

void *
crb_execute_malloc(CRB_Interpreter *inter, size_t size)
{
    void *p;

    p = MEM_storage_malloc(inter->execute_storage, size);

    return p;
}

CRB_Value *
CRB_search_local_variable(CRB_LocalEnvironment *env, char *identifier)
{
    CRB_Value *value;
    CRB_Object  *sc; /* scope chain */

    if (env == NULL)
        return NULL;

    DBG_assert(env->variable->type == SCOPE_CHAIN_OBJECT,
               ("type..%d\n", env->variable->type));

    for (sc = env->variable; sc; sc = sc->u.scope_chain.next) {
        DBG_assert(sc->type == SCOPE_CHAIN_OBJECT,
                   ("sc->type..%d\n", sc->type));
        value = CRB_search_assoc_member(sc->u.scope_chain.frame, identifier);
        if (value)
            break;
    }

    return value;
}

CRB_Value *
CRB_search_local_variable_w(CRB_LocalEnvironment *env, char *identifier,
                            CRB_Boolean *is_final)
{
    CRB_Value *value;
    CRB_Object  *sc; /* scope chain */

    if (env == NULL)
        return NULL;

    DBG_assert(env->variable->type == SCOPE_CHAIN_OBJECT,
               ("type..%d\n", env->variable->type));

    for (sc = env->variable; sc; sc = sc->u.scope_chain.next) {
        DBG_assert(sc->type == SCOPE_CHAIN_OBJECT,
                   ("sc->type..%d\n", sc->type));
        value = CRB_search_assoc_member_w(sc->u.scope_chain.frame, identifier,
                                          is_final);
        if (value)
            break;
    }

    return value;
}

CRB_Value *
CRB_search_global_variable(CRB_Interpreter *inter, char *identifier)
{
    Variable    *pos;

    for (pos = inter->variable; pos; pos = pos->next) {
        if (!strcmp(pos->name, identifier))
            break;
    }
    if (pos == NULL) {
        return NULL;
    } else {
        return &pos->value;
    }
}

Variable *
crb_search_global_variable(CRB_Interpreter *inter, char *identifier)
{
    Variable    *pos;

    for (pos = inter->variable; pos; pos = pos->next) {
        if (!strcmp(pos->name, identifier))
            return pos;
    }

    return NULL;
}

CRB_Value *
CRB_search_global_variable_w(CRB_Interpreter *inter, char *identifier,
                             CRB_Boolean *is_final)
{
    Variable    *pos;

    for (pos = inter->variable; pos; pos = pos->next) {
        if (!strcmp(pos->name, identifier))
            break;
    }
    if (pos == NULL) {
        return NULL;
    } else {
        *is_final = pos->is_final;
        return &pos->value;
    }
}

CRB_Value *
CRB_add_local_variable(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                       char *identifier, CRB_Value *value,
                       CRB_Boolean is_final)
{
    CRB_Value *ret;

    DBG_assert(env->variable->type == SCOPE_CHAIN_OBJECT,
               ("type..%d\n", env->variable->type));

    ret = CRB_add_assoc_member(inter, env->variable->u.scope_chain.frame,
                               identifier, value, is_final);

    return ret;
}

CRB_Value *
CRB_add_global_variable(CRB_Interpreter *inter, char *identifier,
                        CRB_Value *value, CRB_Boolean is_final)
{
    Variable    *new_variable;

    new_variable = crb_execute_malloc(inter, sizeof(Variable));
    new_variable->is_final = is_final;
    new_variable->name = crb_execute_malloc(inter, strlen(identifier) + 1);
    strcpy(new_variable->name, identifier);
    new_variable->next = inter->variable;
    inter->variable = new_variable;
    new_variable->value = *value;

    return &new_variable->value;
}

char *
crb_get_operator_string(ExpressionType type)
{
    char        *str;

    switch (type) {
    case BOOLEAN_EXPRESSION:    /* FALLTHRU */
    case INT_EXPRESSION:        /* FALLTHRU */
    case DOUBLE_EXPRESSION:     /* FALLTHRU */
    case STRING_EXPRESSION:     /* FALLTHRU */
    case REGEXP_EXPRESSION:     /* FALLTHRU */
    case IDENTIFIER_EXPRESSION:
        DBG_assert(0, ("bad expression type..%d\n", type));
        break;
    case COMMA_EXPRESSION:
        str = ",";
        break;
    case ASSIGN_EXPRESSION:
        str = "=";
        break;
    case ADD_EXPRESSION:
        str = "+";
        break;
    case SUB_EXPRESSION:
        str = "-";
        break;
    case MUL_EXPRESSION:
        str = "*";
        break;
    case DIV_EXPRESSION:
        str = "/";
        break;
    case MOD_EXPRESSION:
        str = "%";
        break;
    case EQ_EXPRESSION:
        str = "==";
        break;
    case NE_EXPRESSION:
        str = "!=";
        break;
    case GT_EXPRESSION:
        str = "<";
        break;
    case GE_EXPRESSION:
        str = "<=";
        break;
    case LT_EXPRESSION:
        str = ">";
        break;
    case LE_EXPRESSION:
        str = ">=";
        break;
    case LOGICAL_AND_EXPRESSION:
        str = "&&";
        break;
    case LOGICAL_OR_EXPRESSION:
        str = "||";
        break;
    case MINUS_EXPRESSION:
        str = "-";
        break;
    case LOGICAL_NOT_EXPRESSION:
        str = "!";
        break;
    case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
    case MEMBER_EXPRESSION:     /* FALLTHRU */
    case NULL_EXPRESSION:       /* FALLTHRU */
    case ARRAY_EXPRESSION:      /* FALLTHRU */
    case INDEX_EXPRESSION:      /* FALLTHRU */
    case INCREMENT_EXPRESSION:  /* FALLTHRU */
    case DECREMENT_EXPRESSION:  /* FALLTHRU */
    case CLOSURE_EXPRESSION:    /* FALLTHRU */
    case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
    default:
        DBG_assert(0, ("bad expression type..%d\n", type));
    }

    return str;
}

char *
CRB_get_type_name(CRB_ValueType type)
{
    switch (type) {
    case CRB_BOOLEAN_VALUE:
        return "boolean";
        break;
    case CRB_INT_VALUE:
        return "int";
        break;
    case CRB_DOUBLE_VALUE:
        return "dobule";
        break;
    case CRB_STRING_VALUE:
        return "string";
        break;
    case CRB_NATIVE_POINTER_VALUE:
        return "native pointer";
        break;
    case CRB_NULL_VALUE:
        return "null";
        break;
    case CRB_ARRAY_VALUE:
        return "array";
        break;
    case CRB_ASSOC_VALUE:
        return "object";
        break;
    case CRB_CLOSURE_VALUE:
        return "closure";
        break;
    case CRB_FAKE_METHOD_VALUE:
        return "method";
        break;
    case CRB_SCOPE_CHAIN_VALUE:
        return "scope chain";
        break;
    default:
        DBG_panic(("bad type..%d\n", type));
    }
    return NULL; /* make compiler happy */
}

char*
CRB_get_object_type_name(CRB_Object *obj)
{
    switch (obj->type) {
    case ARRAY_OBJECT:
        return "array";
    case STRING_OBJECT:
        return "string";
    case ASSOC_OBJECT:
        return "object";
    case SCOPE_CHAIN_OBJECT:
        return "scope chain";
    case NATIVE_POINTER_OBJECT:
        return "native pointer";
    case OBJECT_TYPE_COUNT_PLUS_1: /* FALLTHRU */
    default:
        DBG_assert(0, ("bad object type..%d\n", obj->type));
    }

    return NULL; /* make compiler happy */
}

void
crb_vstr_clear(VString *v)
{
    v->string = NULL;
}

static int
my_strlen(CRB_Char *str)
{
    if (str == NULL) {
        return 0;
    }
    return CRB_wcslen(str);
}

void
crb_vstr_append_string(VString *v, CRB_Char *str)
{
    int new_size;
    int old_len;

    old_len = my_strlen(v->string);
    new_size = sizeof(CRB_Char) * (old_len + CRB_wcslen(str)  + 1);
    v->string = MEM_realloc(v->string, new_size);
    CRB_wcscpy(&v->string[old_len], str);
}

void
crb_vstr_append_character(VString *v, CRB_Char ch)
{
    int current_len;
    
    current_len = my_strlen(v->string);
    v->string = MEM_realloc(v->string,sizeof(CRB_Char) * (current_len + 2));
    v->string[current_len] = ch;
    v->string[current_len+1] = L'\0';
}

CRB_Char *
CRB_value_to_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    int line_number, CRB_Value *value)
{
    VString     vstr;
    char        buf[LINE_BUF_SIZE];
    CRB_Char    wc_buf[LINE_BUF_SIZE];
    int         i;

    crb_vstr_clear(&vstr);

    switch (value->type) {
    case CRB_BOOLEAN_VALUE:
        if (value->u.boolean_value) {
            CRB_mbstowcs("true", wc_buf);
        } else {
            CRB_mbstowcs("false", wc_buf);
        }
        crb_vstr_append_string(&vstr, wc_buf);
        break;
    case CRB_INT_VALUE:
        sprintf(buf, "%d", value->u.int_value);
        CRB_mbstowcs(buf, wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        break;
    case CRB_DOUBLE_VALUE:
        sprintf(buf, "%f", value->u.double_value);
        CRB_mbstowcs(buf, wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        break;
    case CRB_STRING_VALUE:
        crb_vstr_append_string(&vstr, value->u.object->u.string.string);
        break;
    case CRB_NATIVE_POINTER_VALUE:
        sprintf(buf, "%s(%p)", value->u.object->u.native_pointer.info->name,
                value->u.object->u.native_pointer.pointer);
        CRB_mbstowcs(buf, wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        break;
    case CRB_NULL_VALUE:
        CRB_mbstowcs("null", wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        break;
    case CRB_ARRAY_VALUE:
        CRB_mbstowcs("(", wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        for (i = 0; i < value->u.object->u.array.size; i++) {
            CRB_Char *new_str;
            if (i > 0) {
                CRB_mbstowcs(", ", wc_buf);
                crb_vstr_append_string(&vstr, wc_buf);
            }
            new_str = CRB_value_to_string(inter, env, line_number,
                                          &value->u.object->u.array.array[i]);
            crb_vstr_append_string(&vstr, new_str);
            MEM_free(new_str);
        }
        CRB_mbstowcs(")", wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        break;
    case CRB_ASSOC_VALUE:
        CRB_mbstowcs("(", wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        for (i = 0; i < value->u.object->u.assoc.member_count; i++) {
            CRB_Char *new_str;
            if (i > 0) {
                CRB_mbstowcs(", ", wc_buf);
                crb_vstr_append_string(&vstr, wc_buf);
            }
            new_str
                = CRB_mbstowcs_alloc(inter, env, line_number,
                                     value->u.object->u.assoc.member[i].name);
            DBG_assert(new_str != NULL, ("new_str is null.\n"));
            crb_vstr_append_string(&vstr, new_str);
            MEM_free(new_str);

            CRB_mbstowcs("=>", wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            new_str = CRB_value_to_string(inter, env, line_number,
                                          &value->u.object
                                          ->u.assoc.member[i].value);
            crb_vstr_append_string(&vstr, new_str);
            MEM_free(new_str);
        }
        CRB_mbstowcs(")", wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        break;
    case CRB_CLOSURE_VALUE:
        CRB_mbstowcs("closure(", wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        if (value->u.closure.function->name == NULL) {
            CRB_mbstowcs("null", wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
        } else {
            CRB_Char *new_str;
            
            new_str = CRB_mbstowcs_alloc(inter, env, line_number,
                                         value->u.closure.function->name);
            DBG_assert(new_str != NULL, ("new_str is null.\n"));
            crb_vstr_append_string(&vstr, new_str);
            MEM_free(new_str);
        }
        CRB_mbstowcs(")", wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        break;
    case CRB_FAKE_METHOD_VALUE:
        CRB_mbstowcs("fake_method(", wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        {
            CRB_Char *new_str;

            new_str = CRB_mbstowcs_alloc(inter, env, line_number,
                                         value->u.fake_method.method_name);
            DBG_assert(new_str != NULL, ("new_str is null.\n"));
            crb_vstr_append_string(&vstr, new_str);
            MEM_free(new_str);
        }
        CRB_mbstowcs(")", wc_buf);
        crb_vstr_append_string(&vstr, wc_buf);
        break;
    case CRB_SCOPE_CHAIN_VALUE: /* FALLTHRU*/
    default:
        DBG_panic(("value->type..%d\n", value->type));
    }

    return vstr.string;
}
