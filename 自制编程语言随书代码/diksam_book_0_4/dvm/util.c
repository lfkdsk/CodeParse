#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

void
dvm_vstr_clear(VString *v)
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
dvm_vstr_append_string(VString *v, DVM_Char *str)
{
    int new_size;
    int old_len;

    old_len = my_strlen(v->string);
    new_size = sizeof(DVM_Char) * (old_len + dvm_wcslen(str)  + 1);
    v->string = MEM_realloc(v->string, new_size);
    dvm_wcscpy(&v->string[old_len], str);
}

void
dvm_vstr_append_character(VString *v, DVM_Char ch)
{
    int current_len;
    
    current_len = my_strlen(v->string);
    v->string = MEM_realloc(v->string,sizeof(DVM_Char) * (current_len + 2));
    v->string[current_len] = ch;
    v->string[current_len+1] = L'\0';
}

void
dvm_initialize_value(DVM_TypeSpecifier *type, DVM_Value *value)
{
    if (type->derive_count > 0) {
        if (type->derive[0].tag == DVM_ARRAY_DERIVE) {
            value->object = dvm_null_object_ref;
        } else {
            DBG_assert(0, ("tag..%d", type->derive[0].tag));
        }
    } else {
        switch (type->basic_type) {
        case DVM_VOID_TYPE:  /* FALLTHRU */
        case DVM_BOOLEAN_TYPE: /* FALLTHRU */
        case DVM_INT_TYPE: /* FALLTHRU */
        case DVM_ENUM_TYPE: /* FALLTHRU */
            value->int_value = 0;
            break;
        case DVM_DOUBLE_TYPE:
            value->double_value = 0.0;
            break;
        case DVM_STRING_TYPE: /* FALLTHRU */
        case DVM_NATIVE_POINTER_TYPE: /* FALLTHRU */
        case DVM_CLASS_TYPE: /* FALLTHRU */
        case DVM_DELEGATE_TYPE:
            value->object = dvm_null_object_ref;
            break;
        case DVM_NULL_TYPE: /* FALLTHRU */
        case DVM_BASE_TYPE: /* FALLTHRU */
        case DVM_UNSPECIFIED_IDENTIFIER_TYPE: /* FALLTHRU */
        default:
            DBG_assert(0, ("basic_type..%d", type->basic_type));
        }
    }
}
