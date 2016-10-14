#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static void
check_gc(CRB_Interpreter *inter)
{
#if 0
    crb_garbage_collect(inter);
#endif
    if (inter->heap.current_heap_size > inter->heap.current_threshold) {
        /* fprintf(stderr, "garbage collecting..."); */
        crb_garbage_collect(inter);
        /* fprintf(stderr, "done.\n"); */

        inter->heap.current_threshold
            = inter->heap.current_heap_size + HEAP_THRESHOLD_SIZE;
    }
}

static CRB_Object *
alloc_object(CRB_Interpreter *inter, ObjectType type)
{
    CRB_Object *ret;

    check_gc(inter);
    ret = MEM_malloc(sizeof(CRB_Object));
    inter->heap.current_heap_size += sizeof(CRB_Object);
    ret->type = type;
    ret->marked = CRB_FALSE;
    ret->prev = NULL;
    ret->next = inter->heap.header;
    inter->heap.header = ret;
    if (ret->next) {
        ret->next->prev = ret;
    }

    return ret;
}

static void
add_ref_in_native_method(CRB_LocalEnvironment *env, CRB_Object *obj)
{
    RefInNativeFunc *new_ref;

    new_ref = MEM_malloc(sizeof(RefInNativeFunc));
    new_ref->object = obj;
    new_ref->next = env->ref_in_native_method;
    env->ref_in_native_method = new_ref;
}

CRB_Object *
crb_literal_to_crb_string_i(CRB_Interpreter *inter, CRB_Char *str)
{
    CRB_Object *ret;

    ret = alloc_object(inter, STRING_OBJECT);
    ret->u.string.string = str;
    ret->u.string.is_literal = CRB_TRUE;

    return ret;
}

CRB_Object *
CRB_literal_to_crb_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          CRB_Char *str)
{
    CRB_Object *ret;

    ret = crb_literal_to_crb_string_i(inter, str);
    add_ref_in_native_method(env, ret);

    return ret;
}

CRB_Object *
crb_create_crowbar_string_i(CRB_Interpreter *inter, CRB_Char *str)
{
    CRB_Object *ret;

    ret = alloc_object(inter, STRING_OBJECT);
    ret->u.string.string = str;
    inter->heap.current_heap_size += sizeof(CRB_Char) * (CRB_wcslen(str) + 1);
    ret->u.string.is_literal = CRB_FALSE;

    return ret;
}

CRB_Object *
CRB_create_crowbar_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          CRB_Char *str)
{
    CRB_Object *ret;

    ret = crb_create_crowbar_string_i(inter, str);
    add_ref_in_native_method(env, ret);

    return ret;
}

CRB_Object *
crb_string_substr_i(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    CRB_Object *str, int from, int len, int line_number)
{
    int org_len = CRB_wcslen(str->u.string.string);
    CRB_Char *new_str;

    if (from < 0 || from >= org_len) {
        crb_runtime_error(inter, env, line_number,
                          STRING_POS_OUT_OF_BOUNDS_ERR,
                          CRB_INT_MESSAGE_ARGUMENT, "len", org_len,
                          CRB_INT_MESSAGE_ARGUMENT, "pos", from,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    if (len < 0 || from + len > org_len) {
        crb_runtime_error(inter, env, line_number, STRING_SUBSTR_LEN_ERR,
                          CRB_INT_MESSAGE_ARGUMENT, "len", len,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    new_str = MEM_malloc(sizeof(CRB_Char) * (len+1));
    CRB_wcsncpy(new_str, str->u.string.string + from, len);
    new_str[len] = L'\0';

    return crb_create_crowbar_string_i(inter, new_str);
}

CRB_Object *
CRB_string_substr(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                  CRB_Object *str, int from, int len, int line_number)
{
    CRB_Object *ret;

    ret = crb_string_substr_i(inter, env, str, from, len, line_number);
    add_ref_in_native_method(env, ret);

    return ret;
}

CRB_Object *
crb_create_array_i(CRB_Interpreter *inter, int size)
{
    CRB_Object *ret;

    ret = alloc_object(inter, ARRAY_OBJECT);
    ret->u.array.size = size;
    ret->u.array.alloc_size = size;
    ret->u.array.array = MEM_malloc(sizeof(CRB_Value) * size);
    inter->heap.current_heap_size += sizeof(CRB_Value) * size;

    return ret;
}

CRB_Object *
CRB_create_array(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                 int size)
{
    CRB_Object *ret;

    ret = crb_create_array_i(inter, size);
    add_ref_in_native_method(env, ret);

    return ret;
}

void
CRB_array_resize(CRB_Interpreter *inter, CRB_Object *obj, int new_size)
{
    int new_alloc_size;
    CRB_Boolean need_realloc;
    int i;

    check_gc(inter);
    
    if (new_size > obj->u.array.alloc_size) {
        new_alloc_size = obj->u.array.alloc_size * 2;
        if (new_alloc_size < new_size) {
            new_alloc_size = new_size + ARRAY_ALLOC_SIZE;
        } else if (new_alloc_size - obj->u.array.alloc_size
                   > ARRAY_ALLOC_SIZE) {
            new_alloc_size = obj->u.array.alloc_size + ARRAY_ALLOC_SIZE;
        }
        need_realloc = CRB_TRUE;
    } else if (obj->u.array.alloc_size - new_size > ARRAY_ALLOC_SIZE) {
        new_alloc_size = new_size;
        need_realloc = CRB_TRUE;
    } else {
        need_realloc = CRB_FALSE;
    }
    if (need_realloc) {
        check_gc(inter);
        obj->u.array.array = MEM_realloc(obj->u.array.array,
                                         new_alloc_size * sizeof(CRB_Value));
        inter->heap.current_heap_size
            += (new_alloc_size - obj->u.array.alloc_size) * sizeof(CRB_Value);
        obj->u.array.alloc_size = new_alloc_size;
    }
    for (i = obj->u.array.size; i < new_size; i++) {
        obj->u.array.array[i].type = CRB_NULL_VALUE;
    }
    obj->u.array.size = new_size;
}

void
CRB_array_add(CRB_Interpreter *inter, CRB_Object *obj, CRB_Value *v)
{
    DBG_assert(obj->type == ARRAY_OBJECT, ("bad type..%d\n", obj->type));

    CRB_array_resize(inter, obj, obj->u.array.size + 1);
    obj->u.array.array[obj->u.array.size-1] = *v;
}

void
CRB_array_insert(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                 CRB_Object *obj, int pos,
                 CRB_Value *new_value, int line_number)
{
    int i;
    DBG_assert(obj->type == ARRAY_OBJECT, ("bad type..%d\n", obj->type));

    if (pos < 0 || pos > obj->u.array.size) {
        crb_runtime_error(inter, env, line_number,
                          ARRAY_INDEX_OUT_OF_BOUNDS_ERR,
                          CRB_INT_MESSAGE_ARGUMENT, "size", obj->u.array.size,
                          CRB_INT_MESSAGE_ARGUMENT, "index", pos,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_array_resize(inter, obj, obj->u.array.size + 1);
    for (i = obj->u.array.size-2; i >= pos; i--) {
        obj->u.array.array[i+1] = obj->u.array.array[i];
    }
    obj->u.array.array[pos] = *new_value;
}

void
CRB_array_remove(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                 CRB_Object *obj, int pos, int line_number)
{
    int i;

    DBG_assert(obj->type == ARRAY_OBJECT, ("bad type..%d\n", obj->type));
    if (pos < 0 || pos > obj->u.array.size-1) {
        crb_runtime_error(inter, env, line_number,
                          ARRAY_INDEX_OUT_OF_BOUNDS_ERR,
                          CRB_INT_MESSAGE_ARGUMENT, "size", obj->u.array.size,
                          CRB_INT_MESSAGE_ARGUMENT, "index", pos,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    for (i = pos+1; i < obj->u.array.size; i++) {
        obj->u.array.array[i-1] = obj->u.array.array[i];
    }
    CRB_array_resize(inter, obj, obj->u.array.size - 1);
}

CRB_Object *
crb_create_assoc_i(CRB_Interpreter *inter)
{
    CRB_Object *ret;

    ret = alloc_object(inter, ASSOC_OBJECT);
    ret->u.assoc.member_count = 0;
    ret->u.assoc.member = NULL;

    return ret;
}

CRB_Object *
CRB_create_assoc(CRB_Interpreter *inter, CRB_LocalEnvironment *env)
{
    CRB_Object *ret;

    ret = crb_create_assoc_i(inter);
    add_ref_in_native_method(env, ret);

    return ret;
}

CRB_Value *
CRB_add_assoc_member(CRB_Interpreter *inter, CRB_Object *assoc,
                     char *name, CRB_Value *value, CRB_Boolean is_final)
{
    AssocMember *member_p;

    check_gc(inter);
    member_p = MEM_realloc(assoc->u.assoc.member,
                           sizeof(AssocMember)
                           * (assoc->u.assoc.member_count+1));
    member_p[assoc->u.assoc.member_count].name = name;
    member_p[assoc->u.assoc.member_count].value = *value;
    member_p[assoc->u.assoc.member_count].is_final = is_final;
    assoc->u.assoc.member = member_p;
    assoc->u.assoc.member_count++;
    inter->heap.current_heap_size += sizeof(AssocMember);

    return &member_p[assoc->u.assoc.member_count-1].value;
}

void
CRB_add_assoc_member2(CRB_Interpreter *inter, CRB_Object *assoc,
                      char *name, CRB_Value *value)
{
    int i;

    if (assoc->u.assoc.member_count > 0) {
        for (i = 0; i < assoc->u.assoc.member_count; i++) {
            if (!strcmp(assoc->u.assoc.member[i].name, name)) {
                break;
            }
        }
        if (i < assoc->u.assoc.member_count) {
            /* BUGBUG */
            assoc->u.assoc.member[i].value = *value;
            return;
        }
    }
    CRB_add_assoc_member(inter, assoc, name, value, CRB_FALSE);
}

CRB_Value *
CRB_search_assoc_member(CRB_Object *assoc, char *member_name)
{
    CRB_Value *ret = NULL;
    int i;

    if (assoc->u.assoc.member_count == 0)
        return NULL;

    for (i = 0; i < assoc->u.assoc.member_count; i++) {
        if (!strcmp(assoc->u.assoc.member[i].name, member_name)) {
            ret = &assoc->u.assoc.member[i].value;
            break;
        }
    }

    return ret;
}

CRB_Value *
CRB_search_assoc_member_w(CRB_Object *assoc, char *member_name,
                          CRB_Boolean *is_final)
{
    CRB_Value *ret = NULL;
    int i;

    if (assoc->u.assoc.member_count == 0)
        return NULL;

    for (i = 0; i < assoc->u.assoc.member_count; i++) {
        if (!strcmp(assoc->u.assoc.member[i].name, member_name)) {
            ret = &assoc->u.assoc.member[i].value;
            *is_final = assoc->u.assoc.member[i].is_final;
            break;
        }
    }
    return ret;
}

CRB_Object *
crb_create_scope_chain(CRB_Interpreter *inter)
{
    CRB_Object *ret;

    ret = alloc_object(inter, SCOPE_CHAIN_OBJECT);
    ret->u.scope_chain.frame = NULL;
    ret->u.scope_chain.next = NULL;

    return ret;
}

CRB_Object *
crb_create_native_pointer_i(CRB_Interpreter *inter, void *pointer,
                            CRB_NativePointerInfo *info)
{
    CRB_Object *ret;

    ret = alloc_object(inter, NATIVE_POINTER_OBJECT);
    ret->u.native_pointer.pointer = pointer;
    ret->u.native_pointer.info = info;

    return ret;
}

CRB_Object *
CRB_create_native_pointer(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          void *pointer, CRB_NativePointerInfo *info)
{
    CRB_Object *ret;

    ret = crb_create_native_pointer_i(inter, pointer, info);
    add_ref_in_native_method(env, ret);

    return ret;
}

CRB_NativePointerInfo *
CRB_get_native_pointer_type(CRB_Object *native_pointer)
{
    return native_pointer->u.native_pointer.info;
}


CRB_Boolean
CRB_check_native_pointer_type(CRB_Object *native_pointer,
                              CRB_NativePointerInfo *info)
{
    return native_pointer->u.native_pointer.info == info;
}

static int
count_stack_trace_depth(CRB_LocalEnvironment *top)
{
    CRB_LocalEnvironment *pos;
    int count = 0;

    for (pos = top; pos; pos = pos->next) {
        count++;
    }

    return count + 1;
}

static CRB_Object *
create_stack_trace_line(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                        char *func_name, int line_number)
{
    CRB_Char    *wc_func_name;
    CRB_Object  *new_line;
    CRB_Value   new_line_value;
    CRB_Value   value;
    int         stack_count = 0;

    new_line = crb_create_assoc_i(inter);
    new_line_value.type = CRB_ASSOC_VALUE;
    new_line_value.u.object = new_line;
    CRB_push_value(inter, &new_line_value);
    stack_count++;

    wc_func_name = CRB_mbstowcs_alloc(inter, env, line_number, func_name);
    DBG_assert(wc_func_name != NULL, ("wc_func_name is null.\n"));

    value.type = CRB_STRING_VALUE;
    value.u.object = crb_create_crowbar_string_i(inter, wc_func_name);
    CRB_push_value(inter, &value);
    stack_count++;

    CRB_add_assoc_member(inter, new_line, EXCEPTION_MEMBER_FUNCTION_NAME,
                         &value, CRB_TRUE);

    value.type = CRB_INT_VALUE;
    value.u.int_value = line_number;
    CRB_add_assoc_member(inter, new_line, EXCEPTION_MEMBER_LINE_NUMBER,
                         &value, CRB_TRUE);

    CRB_shrink_stack(inter, stack_count);

    return new_line;
}

static CRB_Value
print_stack_trace(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                  int arg_count, CRB_Value *args)
{
    CRB_Value *message;
    CRB_Value *stack_trace;
    CRB_Value ret;
    int i;

    message = CRB_search_local_variable(env, EXCEPTION_MEMBER_MESSAGE);
    if (message == NULL) {
        crb_runtime_error(inter, env, __LINE__,
                          EXCEPTION_HAS_NO_MESSAGE_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    if (message->type != CRB_STRING_VALUE) {
        crb_runtime_error(inter, env, __LINE__,
                          EXCEPTION_MESSAGE_IS_NOT_STRING_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "type",
                          CRB_get_type_name(message->type),
                          CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_print_wcs_ln(stderr, message->u.object->u.string.string);

    stack_trace = CRB_search_local_variable(env, EXCEPTION_MEMBER_STACK_TRACE);
    if (stack_trace == NULL) {
        crb_runtime_error(inter, env, __LINE__,
                          EXCEPTION_HAS_NO_STACK_TRACE_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    if (stack_trace->type != CRB_ARRAY_VALUE) {
        crb_runtime_error(inter, env, __LINE__,
                          STACK_TRACE_IS_NOT_ARRAY_ERR,
                          CRB_STRING_MESSAGE_ARGUMENT, "type",
                          CRB_get_type_name(stack_trace->type),
                          CRB_MESSAGE_ARGUMENT_END);
    }

    for (i = 0; i < stack_trace->u.object->u.array.size; i++) {
        CRB_Object *assoc;
        CRB_Value *line_number;
        CRB_Value *function_name;
        CRB_Char *str;

        if (stack_trace->u.object->u.array.array[i].type != CRB_ASSOC_VALUE) {
            crb_runtime_error(inter, env, __LINE__,
                              STACK_TRACE_LINE_IS_NOT_ASSOC_ERR,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        assoc = stack_trace->u.object->u.array.array[i].u.object;

        line_number
            = CRB_search_assoc_member(assoc,
                                      EXCEPTION_MEMBER_LINE_NUMBER);
        if (line_number == NULL
            || line_number->type != CRB_INT_VALUE) {
            crb_runtime_error(inter, env, __LINE__,
                              STACK_TRACE_LINE_HAS_NO_LINE_NUMBER_ERR,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        function_name
            = CRB_search_assoc_member(assoc,
                                      EXCEPTION_MEMBER_FUNCTION_NAME);
        if (function_name == NULL
            || function_name->type != CRB_STRING_VALUE) {
            crb_runtime_error(inter, env, __LINE__,
                              STACK_TRACE_LINE_HAS_NO_FUNC_NAME_ERR,
                              CRB_MESSAGE_ARGUMENT_END);
        }
        str = CRB_value_to_string(inter, NULL,
                                  line_number->u.int_value,
                                  function_name);
        CRB_print_wcs(stderr, str);
        MEM_free(str);
            
        fprintf(stderr, " at ");

        str = CRB_value_to_string(inter, NULL,
                                  line_number->u.int_value, line_number);
        CRB_print_wcs_ln(stderr, str);
        MEM_free(str);
    }
    
    ret.type = CRB_NULL_VALUE;
    return ret;
}

CRB_Object *
CRB_create_exception(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                     CRB_Object *message, int line_number)
{
    CRB_Object  *ret;
    CRB_Value   value;
    CRB_Object  *stack_trace; /* CRB_Array */
    int         stack_count = 0;
    int         stack_trace_depth;
    CRB_LocalEnvironment *env_pos;
    int         stack_trace_idx;
    CRB_Object  *line; /* CRB_Assoc */
    char        *func_name;
    int         next_line_number;
    static      CRB_FunctionDefinition print_stack_trace_fd;
    CRB_Value   scope_chain;

    ret = crb_create_assoc_i(inter);
    value.type = CRB_ASSOC_VALUE;
    value.u.object = ret;
    CRB_push_value(inter, &value);
    stack_count++;

    value.type = CRB_STRING_VALUE;
    value.u.object = message;
    CRB_push_value(inter, &value);
    stack_count++;
    CRB_add_assoc_member(inter, ret, EXCEPTION_MEMBER_MESSAGE, &value,
                         CRB_TRUE);

    stack_trace_depth = count_stack_trace_depth(env);
    stack_trace = crb_create_array_i(inter, stack_trace_depth);
    value.type = CRB_ARRAY_VALUE;
    value.u.object = stack_trace;
    CRB_push_value(inter, &value);
    stack_count++;

    CRB_add_assoc_member(inter, ret, EXCEPTION_MEMBER_STACK_TRACE, &value,
                         CRB_TRUE);

    next_line_number = line_number;
    for (env_pos = env, stack_trace_idx = 0; env_pos;
         env_pos = env_pos->next, stack_trace_idx++) {
        if (env_pos->current_function_name) {
            func_name = env_pos->current_function_name;
        } else {
            func_name = "anonymous closure";
        }
        line = create_stack_trace_line(inter, env, func_name,
                                       next_line_number);
        value.type = CRB_ASSOC_VALUE;
        value.u.object = line;
        CRB_array_set(inter, env, stack_trace, stack_trace_idx,
                      &value);
        next_line_number = env_pos->caller_line_number;
    }
    
    line = create_stack_trace_line(inter, env, "top_level",
                                   next_line_number);
    value.type = CRB_ASSOC_VALUE;
    value.u.object = line;
    CRB_array_set(inter, env, stack_trace, stack_trace_idx,
                  &value);

    CRB_set_function_definition(NULL, print_stack_trace,
                                &print_stack_trace_fd);

    CRB_push_value(inter, &value);
    stack_count++;

    value.type = CRB_CLOSURE_VALUE;
    value.u.closure.function = &print_stack_trace_fd;
    value.u.closure.environment = NULL; /* to stop marking by GC */

    scope_chain.type = CRB_SCOPE_CHAIN_VALUE;
    scope_chain.u.object = crb_create_scope_chain(inter);
    scope_chain.u.object->u.scope_chain.frame = ret;
    CRB_push_value(inter, &scope_chain);
    stack_count++;

    value.u.closure.environment = scope_chain.u.object;

    CRB_add_assoc_member(inter, ret, EXCEPTION_MEMBER_PRINT_STACK_TRACE,
                         &value, CRB_TRUE);

    CRB_shrink_stack(inter, stack_count);

    return ret;
}

static void gc_mark_value(CRB_Value *v);

static void
gc_mark(CRB_Object *obj)
{
    if (obj == NULL)
        return;

    if (obj->marked)
        return;

    obj->marked = CRB_TRUE;

    if (obj->type == ARRAY_OBJECT) {
        int i;
        for (i = 0; i < obj->u.array.size; i++) {
            gc_mark_value(&obj->u.array.array[i]);
        }
    } else if (obj->type == ASSOC_OBJECT) {
        int i;
        for (i = 0; i < obj->u.assoc.member_count; i++) {
            gc_mark_value(&obj->u.assoc.member[i].value);
        }
    } else if (obj->type == SCOPE_CHAIN_OBJECT) {
        gc_mark(obj->u.scope_chain.frame);
        gc_mark(obj->u.scope_chain.next);
    }
}

static void
gc_reset_mark(CRB_Object *obj)
{
    obj->marked = CRB_FALSE;
}


static void
gc_mark_value(CRB_Value *v)
{
    if (crb_is_object_value(v->type)) {
        gc_mark(v->u.object);
    } else if (v->type == CRB_CLOSURE_VALUE) {
        if (v->u.closure.environment) {
            gc_mark(v->u.closure.environment);
        }
    } else if (v->type == CRB_FAKE_METHOD_VALUE) {
        gc_mark(v->u.fake_method.object);
    }
}

static void
gc_mark_ref_in_native_method(CRB_LocalEnvironment *env)
{
    RefInNativeFunc *ref;

    for (ref = env->ref_in_native_method; ref; ref = ref->next) {
        gc_mark(ref->object);
    }
}

static void
gc_mark_objects(CRB_Interpreter *inter)
{
    CRB_Object *obj;
    Variable *v;
    CRB_LocalEnvironment *lv;
    int i;

    for (obj = inter->heap.header; obj; obj = obj->next) {
        gc_reset_mark(obj);
    }
    
    for (v = inter->variable; v; v = v->next) {
        gc_mark_value(&v->value);
    }
    
    for (lv = inter->top_environment; lv; lv = lv->next) {
        gc_mark(lv->variable);
        gc_mark_ref_in_native_method(lv);
    }

    for (i = 0; i < inter->stack.stack_pointer; i++) {
        gc_mark_value(&inter->stack.stack[i]);
    }

    gc_mark_value(&inter->current_exception);
}

static void
gc_dispose_object(CRB_Interpreter *inter, CRB_Object *obj)
{
    switch (obj->type) {
    case ARRAY_OBJECT:
        inter->heap.current_heap_size
            -= sizeof(CRB_Value) * obj->u.array.alloc_size;
        MEM_free(obj->u.array.array);
        break;
    case STRING_OBJECT:
        if (!obj->u.string.is_literal) {
            inter->heap.current_heap_size
                -= sizeof(CRB_Char) * (CRB_wcslen(obj->u.string.string) + 1);
            MEM_free(obj->u.string.string);
        }
        break;
    case ASSOC_OBJECT:
        inter->heap.current_heap_size
            -= sizeof(AssocMember) * obj->u.assoc.member_count;
        MEM_free(obj->u.assoc.member);
        break;
    case SCOPE_CHAIN_OBJECT:
        break;
    case NATIVE_POINTER_OBJECT:
        if (obj->u.native_pointer.info->finalizer) {
            obj->u.native_pointer.info->finalizer(inter, obj);
        }
        break;
    case OBJECT_TYPE_COUNT_PLUS_1:
    default:
        DBG_assert(0, ("bad type..%d\n", obj->type));
    }
    inter->heap.current_heap_size -= sizeof(CRB_Object);
    MEM_free(obj);
}

static void
gc_sweep_objects(CRB_Interpreter *inter)
{
    CRB_Object *obj;
    CRB_Object *tmp;

    for (obj = inter->heap.header; obj; ) {
        if (!obj->marked) {
            if (obj->prev) {
                obj->prev->next = obj->next;
            } else {
                inter->heap.header = obj->next;
            }
            if (obj->next) {
                obj->next->prev = obj->prev;
            }
            tmp = obj->next;
            gc_dispose_object(inter, obj);
            obj = tmp;
        } else {
            obj = obj->next;
        }
    }
}

void
crb_garbage_collect(CRB_Interpreter *inter)
{
    gc_mark_objects(inter);
    gc_sweep_objects(inter);
}
