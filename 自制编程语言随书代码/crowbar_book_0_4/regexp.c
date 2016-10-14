#include <limits.h>
#include "DBG.h"
#include "crowbar.h"


static OnigUChar *
encode_utf16_be(CRB_Char *src)
{
    OnigUChar *dest = NULL;
    int dest_size;
    int src_idx;
    int dest_idx;
    
    dest_size = CRB_wcslen(src) * 2 + 2;
    dest = MEM_malloc(dest_size);

    for (src_idx = dest_idx = 0; ; src_idx++) {
        if (dest_idx + 1 >= dest_size) {
            MEM_free(dest);
            return NULL;
        }
        dest[dest_idx] = (src[src_idx] >> 8) & 0xff;
        dest[dest_idx+1] = src[src_idx] & 0xff;
        dest_idx += 2;
        if (src[src_idx] == L'\0')
            break;
    }

    return dest;
}

static CRB_Regexp *
alloc_crb_regexp(regex_t *reg, CRB_Boolean is_literal)
{
    CRB_Regexp *crb_reg;

    crb_reg = MEM_malloc(sizeof(CRB_Regexp));
    crb_reg->is_literal = is_literal;
    crb_reg->regexp = reg;
    crb_reg->next = NULL;

    return crb_reg;
}

CRB_Regexp *
crb_create_regexp_in_compile(CRB_Char *str)
{
    int         r;
    regex_t     *reg;
    OnigErrorInfo einfo;
    CRB_Regexp  *regexp;
    CRB_Interpreter *inter;
    OnigUChar   *pattern;

    pattern = encode_utf16_be(str);
    if (pattern == NULL) {
        crb_compile_error(UNEXPECTED_WIDE_STRING_IN_COMPILE_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    r = onig_new(&reg, pattern,
                 pattern
                 + onigenc_str_bytelen_null(ONIG_ENCODING_UTF16_BE, pattern),
                 ONIG_OPTION_DEFAULT, ONIG_ENCODING_UTF16_BE,
                 ONIG_SYNTAX_PERL, &einfo);
    if (r != ONIG_NORMAL) {
        char s[ONIG_MAX_ERROR_MESSAGE_LEN];
        onig_error_code_to_str(s, r, &einfo);
        crb_compile_error(CAN_NOT_CREATE_REGEXP_IN_COMPILE_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "message", s,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    regexp = alloc_crb_regexp(reg, CRB_TRUE);
    inter = crb_get_current_interpreter();
    regexp->next = inter->regexp_literals;
    inter->regexp_literals = regexp;

    MEM_free(pattern);

    return regexp;
}

void
crb_dispose_regexp_literals(CRB_Interpreter *inter)
{
    CRB_Regexp *tmp;

    while (inter->regexp_literals) {
        tmp = inter->regexp_literals;
        inter->regexp_literals = inter->regexp_literals->next;
        onig_free(tmp->regexp);
        MEM_free(tmp);
    }
}

static void
regexp_finalizer(CRB_Interpreter *inter, CRB_Object *obj)
{
    CRB_Regexp *regexp;

    regexp = (CRB_Regexp*)CRB_object_get_native_pointer(obj);
    if (!regexp->is_literal) {
        onig_free(regexp->regexp);
        MEM_free(regexp);
    }
}

static CRB_NativePointerInfo st_regexp_type_info = {
    "crowbar.lang.regexp",
    regexp_finalizer
};

CRB_NativePointerInfo *
crb_get_regexp_info(void)
{
    return &st_regexp_type_info;
}

static CRB_Boolean
match_sub(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
              regex_t *regexp, OnigUChar *subject,
              OnigUChar *end_p, OnigUChar *at_p, OnigUChar **next_at,
              OnigRegion *region_org)
{
    int r;
    OnigRegion *region = NULL;
    
    if (region_org == NULL && next_at != NULL) {
        region = onig_region_new();
    } else {
        region = region_org;
    }
    r = onig_search(regexp, subject, end_p, at_p, end_p,
                    region, ONIG_OPTION_NONE);
    if (r < 0 && r != ONIG_MISMATCH) {
        char s[ONIG_MAX_ERROR_MESSAGE_LEN];
        onig_region_free(region, 1);
        MEM_free(subject);
        onig_error_code_to_str(s, r);
        crb_runtime_error(inter, env, __LINE__,
                          ONIG_SEARCH_FAIL_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "message", s,
                          CRB_MESSAGE_ARGUMENT_END);
    }

    if (next_at != NULL) {
        *next_at = subject + region->end[0];

        if (region_org == NULL) {
            onig_region_free(region, 1);
        }
    }

    return r >= 0;
}

static void
onig_region_to_crb_region(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          OnigRegion *onig_region, CRB_Object *crb_region,
                          CRB_Object *crb_subject)
{
    int i;
    CRB_Value begin_val;
    CRB_Value end_val;
    CRB_Value string_val;
    int stack_count = 0;
    CRB_Value tmp_val;

    begin_val.type = CRB_ARRAY_VALUE;
    begin_val.u.object = crb_create_array_i(inter, onig_region->num_regs);
    CRB_push_value(inter, &begin_val);
    stack_count++;
    CRB_add_assoc_member2(inter, crb_region, "begin", &begin_val);

    end_val.type = CRB_ARRAY_VALUE;
    end_val.u.object = crb_create_array_i(inter, onig_region->num_regs);
    CRB_push_value(inter, &end_val);
    stack_count++;
    CRB_add_assoc_member2(inter, crb_region, "end", &end_val);

    string_val.type = CRB_ARRAY_VALUE;
    string_val.u.object = crb_create_array_i(inter, onig_region->num_regs);
    CRB_push_value(inter, &string_val);
    stack_count++;
    CRB_add_assoc_member2(inter, crb_region, "string", &string_val);
    
    for (i = 0; i < onig_region->num_regs; i++) {
        tmp_val.type = CRB_INT_VALUE;
        tmp_val.u.int_value = onig_region->beg[i] / 2;
        CRB_array_set(inter, env, begin_val.u.object, i, &tmp_val);

        tmp_val.type = CRB_INT_VALUE;
        tmp_val.u.int_value = onig_region->end[i] / 2;
        CRB_array_set(inter, env, end_val.u.object, i, &tmp_val);

        tmp_val.type = CRB_STRING_VALUE;
        tmp_val.u.object
            =  crb_string_substr_i(inter, env, crb_subject,
                                   onig_region->beg[i] / 2,
                                   (onig_region->end[i]
                                    - onig_region->beg[i])/2,
                                   __LINE__);
        CRB_array_set(inter, env, string_val.u.object, i, &tmp_val);
    }

    CRB_shrink_stack(inter, stack_count);
}

static CRB_Boolean
match_crb_if(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
             CRB_Regexp* crb_reg, CRB_Object *crb_subject,
             CRB_Value *crb_region)
{
    OnigUChar *subject;
    OnigUChar *end_p;
    OnigRegion *onig_region = NULL;
    CRB_Boolean matched;


    subject = encode_utf16_be(crb_subject->u.string.string);
    if (subject == NULL) {
        crb_runtime_error(inter, env, __LINE__, UNEXPECTED_WIDE_STRING_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    if (crb_region) {
        onig_region = onig_region_new();
    }
    end_p = subject
        + onigenc_str_bytelen_null(ONIG_ENCODING_UTF16_BE, subject);

    matched = match_sub(inter, env, crb_reg->regexp, subject, end_p,
                        subject, NULL, onig_region);

    MEM_free(subject);
    if (crb_region) {
        onig_region_to_crb_region(inter, env,
                                  onig_region, crb_region->u.object,
                                  crb_subject);
        onig_region_free(onig_region, 1);
    }

    return matched;
}

static CRB_Value
nv_match_proc(CRB_Interpreter *inter,
              CRB_LocalEnvironment *env,
              int arg_count, CRB_Value *args)
{
    static CRB_ValueType arg_type[] = {
        CRB_NATIVE_POINTER_VALUE,       /* pattern */
        CRB_STRING_VALUE,               /* subject */
    };
    char *FUNC_NAME = "reg_match";
    CRB_Regexp *crb_reg;
    CRB_Value *region = NULL;
    CRB_Value result;

    if (arg_count > 3) {
        crb_runtime_error(inter, env, __LINE__, ARGUMENT_TOO_MANY_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_check_argument_type(inter, env, arg_count, ARRAY_SIZE(arg_type),
                            args, arg_type, FUNC_NAME);
    if (arg_count == 3) {
        CRB_check_one_argument_type(inter, env, &args[2], CRB_ASSOC_VALUE,
                                    FUNC_NAME, 3);
        region = &args[2];
    }
    if (!CRB_check_native_pointer_type(args[0].u.object,
                                       &st_regexp_type_info)) {
        crb_runtime_error(inter, env, __LINE__,
                          ARGUMENT_TYPE_MISMATCH_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "func_name", FUNC_NAME,
                          CRB_INT_MESSAGE_ARGUMENT, "idx", 1,
                          CRB_STRING_MESSAGE_ARGUMENT,
                          "type",
                          CRB_get_native_pointer_type(args[0].u.object)
                          ->name,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    crb_reg = CRB_object_get_native_pointer(args[0].u.object);

    result.type = CRB_BOOLEAN_VALUE;
    result.u.boolean_value
        = match_crb_if(inter, env, crb_reg, args[1].u.object, region);

    return result;
}

static void
replace_matched_place(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                      CRB_Char *replacement, OnigUChar *subject,
                      OnigRegion *region, VString *vs)
{
    int i;
    char g_idx_str[REGEXP_GROUP_INDEX_MAX_COLUMN+1];
    int g_idx_col;
    int scanf_result;
    int g_idx;
    int g_pos;

    for (i = 0; replacement[i] != L'\0'; i++) {
        if (replacement[i] != L'\\') {
            crb_vstr_append_character(vs, replacement[i]);
            continue;
        }
        if (replacement[i+1] == L'\\') {
            crb_vstr_append_character(vs, replacement[i]);
            i++;
            continue;
        }
        i++;
        for (g_idx_col = 0; g_idx_col < REGEXP_GROUP_INDEX_MAX_COLUMN;
             g_idx_col++) {
            if (CRB_iswdigit(replacement[i])) {
                if (g_idx_col >= REGEXP_GROUP_INDEX_MAX_COLUMN) {
                    MEM_free(subject);
                    MEM_free(vs->string);
                    onig_region_free(region, 1);
                    crb_runtime_error(inter, env, __LINE__,
                                      GROUP_INDEX_OVERFLOW_ERR,
                                      CRB_MESSAGE_ARGUMENT_END);
                }
                g_idx_str[g_idx_col] = '0' + (replacement[i] - L'0');
                i++;
            } else {
                i--;
                break;
            }
        }
        if (g_idx_col == 0) {
            crb_vstr_append_character(vs, L'\\');
            continue;
        }
        g_idx_str[g_idx_col] = '\0';

        scanf_result = sscanf(g_idx_str, "%d", &g_idx);
        DBG_assert(scanf_result == 1, ("sscanf failed. str..%s, result..%d",
                                       g_idx_str, scanf_result));
        if (g_idx <= 0 || g_idx >= region->num_regs) {
            MEM_free(subject);
            MEM_free(vs->string);
            onig_region_free(region, 1);
            crb_runtime_error(inter, env, __LINE__,
                              NO_SUCH_GROUP_INDEX_ERR,
                              CRB_INT_MESSAGE_ARGUMENT, "g_idx", g_idx,
                              CRB_MESSAGE_ARGUMENT_END);
        }

        for (g_pos = region->beg[g_idx]; g_pos < region->end[g_idx];
             g_pos += 2) {
            crb_vstr_append_character(vs, (subject[g_pos] << 8)
                                      + subject[g_pos+1]);
        }
    }
}

static CRB_Object *
replace_crb_if(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
               CRB_Regexp* crb_reg,
               CRB_Object *replacement, CRB_Object *crb_subject,
               int match_limit)
{
    OnigUChar *subject;
    OnigUChar *end_p;
    OnigUChar *at_p;
    OnigUChar *next_at;
    CRB_Boolean matched;
    OnigRegion *region;
    int matched_count = 0;
    int i;
    VString  vs;
    CRB_Object *result;

    subject = encode_utf16_be(crb_subject->u.string.string);
    if (subject == NULL) {
        crb_runtime_error(inter, env, __LINE__, UNEXPECTED_WIDE_STRING_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    end_p = subject
        + onigenc_str_bytelen_null(ONIG_ENCODING_UTF16_BE, subject);
    at_p = subject;

    region = onig_region_new();

    crb_vstr_clear(&vs);

    while (matched_count < match_limit) {
        matched = match_sub(inter, env, crb_reg->regexp, subject, end_p,
                            at_p, &next_at, region);
        if (!matched) {
            break;
        }
        for (i = at_p - subject; i < region->beg[0]; i += 2) {
            crb_vstr_append_character(&vs, (subject[i] << 8)
                                      + subject[i+1]);
        }
        replace_matched_place(inter, env, replacement->u.string.string,
                              subject, region, &vs);

        matched_count++;
        at_p = next_at;
    }
    if (matched_count == 0) {
        MEM_free(subject);
        onig_region_free(region, 1);
        return crb_subject;
    }
    for (i = at_p - subject; i < end_p - subject; i += 2) {
        crb_vstr_append_character(&vs, (subject[i] << 8) + subject[i+1]);
    }
    result = crb_create_crowbar_string_i(inter, vs.string);

    MEM_free(subject);
    onig_region_free(region, 1);

    return result;
}

static CRB_Value
nv_replace_proc(CRB_Interpreter *inter,
                CRB_LocalEnvironment *env,
                int arg_count, CRB_Value *args)
{
    static CRB_ValueType arg_type[] = {
        CRB_NATIVE_POINTER_VALUE,       /* pattern */
        CRB_STRING_VALUE,               /* replacement */
        CRB_STRING_VALUE,               /* subject */
    };
    char *FUNC_NAME = "reg_replace";
    CRB_Regexp *crb_reg;
    CRB_Value result;
    
    CRB_check_argument(inter, env, arg_count, ARRAY_SIZE(arg_type),
                       args, arg_type, FUNC_NAME);

    crb_reg = CRB_object_get_native_pointer(args[0].u.object);

    result.type = CRB_STRING_VALUE;
    result.u.object
        = replace_crb_if(inter, env, crb_reg,
                         args[1].u.object, args[2].u.object, 1);

    return result;
}

static CRB_Value
nv_replace_all_proc(CRB_Interpreter *inter,
                    CRB_LocalEnvironment *env,
                    int arg_count, CRB_Value *args)
{
    static CRB_ValueType arg_type[] = {
        CRB_NATIVE_POINTER_VALUE,       /* pattern */
        CRB_STRING_VALUE,               /* replacement */
        CRB_STRING_VALUE,               /* subject */
    };
    char *FUNC_NAME = "reg_replace_all";
    CRB_Regexp *crb_reg;
    CRB_Value result;
    
    CRB_check_argument(inter, env, arg_count, ARRAY_SIZE(arg_type),
                       args, arg_type, FUNC_NAME);

    crb_reg = CRB_object_get_native_pointer(args[0].u.object);

    result.type = CRB_STRING_VALUE;
    result.u.object
        = replace_crb_if(inter, env, crb_reg,
                         args[1].u.object, args[2].u.object, INT_MAX);

    return result;
}

static void
add_splitted_string(CRB_Interpreter *inter, CRB_Object *array, VString *vs)
{
    CRB_Value str;

    str.type = CRB_STRING_VALUE;
    str.u.object = crb_create_crowbar_string_i(inter, vs->string);
    CRB_push_value(inter, &str);

    CRB_array_add(inter, array, &str);
    
    CRB_pop_value(inter);
}

static void
split_crb_if(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
             CRB_Regexp* crb_reg, CRB_Object *crb_subject,
             CRB_Value *result)
{
    OnigUChar *subject;
    OnigUChar *end_p;
    OnigUChar *at_p;
    OnigUChar *next_at;
    OnigRegion *region;
    int i;
    VString  vs;

    subject = encode_utf16_be(crb_subject->u.string.string);
    if (subject == NULL) {
        crb_runtime_error(inter, env, __LINE__, UNEXPECTED_WIDE_STRING_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    end_p = subject
        + onigenc_str_bytelen_null(ONIG_ENCODING_UTF16_BE, subject);
    at_p = subject;

    region = onig_region_new();

    while (match_sub(inter, env, crb_reg->regexp, subject, end_p,
                     at_p, &next_at, region)) {
        crb_vstr_clear(&vs);
        for (i = at_p - subject; i < region->beg[0]; i += 2) {
            crb_vstr_append_character(&vs, (subject[i] << 8)
                                      + subject[i+1]);
        }
        add_splitted_string(inter, result->u.object, &vs);

        at_p = next_at;
    }

    crb_vstr_clear(&vs);
    for (i = at_p - subject; i < end_p - subject; i += 2) {
        crb_vstr_append_character(&vs, (subject[i] << 8) + subject[i+1]);
    }
    add_splitted_string(inter, result->u.object, &vs);

    MEM_free(subject);
    onig_region_free(region, 1);
}

static CRB_Value
nv_split_proc(CRB_Interpreter *inter,
              CRB_LocalEnvironment *env,
              int arg_count, CRB_Value *args)
{
    static CRB_ValueType arg_type[] = {
        CRB_NATIVE_POINTER_VALUE,       /* pattern */
        CRB_STRING_VALUE,               /* subject */
    };
    char *FUNC_NAME = "reg_split";
    CRB_Regexp *crb_reg;
    CRB_Value result;
    
    CRB_check_argument(inter, env, arg_count, ARRAY_SIZE(arg_type),
                       args, arg_type, FUNC_NAME);

    crb_reg = CRB_object_get_native_pointer(args[0].u.object);

    result.type = CRB_ARRAY_VALUE;
    result.u.object = crb_create_array_i(inter, 0);
    CRB_push_value(inter, &result);
    
    split_crb_if(inter, env, crb_reg, args[1].u.object, &result);
    CRB_pop_value(inter);

    return result;
}

void
crb_add_regexp_functions(CRB_Interpreter *inter)
{
    CRB_add_native_function(inter, "reg_match", nv_match_proc);
    CRB_add_native_function(inter, "reg_replace", nv_replace_proc);
    CRB_add_native_function(inter, "reg_replace_all", nv_replace_all_proc);
    CRB_add_native_function(inter, "reg_split", nv_split_proc);
}
