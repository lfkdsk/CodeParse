#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

static void
check_gc(DVM_VirtualMachine *dvm)
{
#if 0
    dvm_garbage_collect(dvm);
#endif
    if (dvm->heap.current_heap_size > dvm->heap.current_threshold) {
        /* fprintf(stderr, "garbage collecting..."); */
        dvm_garbage_collect(dvm);
        /* fprintf(stderr, "done.\n"); */

        dvm->heap.current_threshold
            = dvm->heap.current_heap_size + HEAP_THRESHOLD_SIZE;
    }
}

static DVM_Object *
alloc_object(DVM_VirtualMachine *dvm, ObjectType type)
{
    DVM_Object *ret;

    check_gc(dvm);
    ret = MEM_malloc(sizeof(DVM_Object));
    dvm->heap.current_heap_size += sizeof(DVM_Object);
    ret->type = type;
    ret->marked = DVM_FALSE;
    ret->prev = NULL;
    ret->next = dvm->heap.header;
    dvm->heap.header = ret;
    if (ret->next) {
        ret->next->prev = ret;
    }

    return ret;
}

DVM_Object *
dvm_literal_to_dvm_string_i(DVM_VirtualMachine *dvm, DVM_Char *str)
{
    DVM_Object *ret;

    ret = alloc_object(dvm, STRING_OBJECT);
    ret->u.string.string = str;
    ret->u.string.is_literal = DVM_TRUE;

    return ret;
}

DVM_Object *
dvm_create_dvm_string_i(DVM_VirtualMachine *dvm, DVM_Char *str)
{
    DVM_Object *ret;

    ret = alloc_object(dvm, STRING_OBJECT);
    ret->u.string.string = str;
    dvm->heap.current_heap_size += sizeof(DVM_Char) * (dvm_wcslen(str) + 1);
    ret->u.string.is_literal = DVM_FALSE;

    return ret;
}

static void
gc_mark(DVM_Object *obj)
{
    if (obj == NULL)
        return;

    if (obj->marked)
        return;

    obj->marked = DVM_TRUE;
}

static void
gc_reset_mark(DVM_Object *obj)
{
    obj->marked = DVM_FALSE;
}

static void
gc_mark_objects(DVM_VirtualMachine *dvm)
{
    DVM_Object *obj;
    int i;

    for (obj = dvm->heap.header; obj; obj = obj->next) {
        gc_reset_mark(obj);
    }
    
    for (i = 0; i < dvm->static_v.variable_count; i++) {
        if (dvm->executable->global_variable[i].type->basic_type
            == DVM_STRING_TYPE) {
            gc_mark(dvm->static_v.variable[i].object);
        }
    }

    for (i = 0; i < dvm->stack.stack_pointer; i++) {
        if (dvm->stack.pointer_flags[i]) {
            gc_mark(dvm->stack.stack[i].object);
        }
    }
}

static void
gc_dispose_object(DVM_VirtualMachine *dvm, DVM_Object *obj)
{
    switch (obj->type) {
    case STRING_OBJECT:
        if (!obj->u.string.is_literal) {
            dvm->heap.current_heap_size
                -= sizeof(DVM_Char) * (dvm_wcslen(obj->u.string.string) + 1);
            MEM_free(obj->u.string.string);
        }
        break;
    case OBJECT_TYPE_COUNT_PLUS_1:
    default:
        DBG_assert(0, ("bad type..%d\n", obj->type));
    }
    dvm->heap.current_heap_size -= sizeof(DVM_Object);
    MEM_free(obj);
}

static void
gc_sweep_objects(DVM_VirtualMachine *dvm)
{
    DVM_Object *obj;
    DVM_Object *tmp;

    for (obj = dvm->heap.header; obj; ) {
        if (!obj->marked) {
            if (obj->prev) {
                obj->prev->next = obj->next;
            } else {
                dvm->heap.header = obj->next;
            }
            if (obj->next) {
                obj->next->prev = obj->prev;
            }
            tmp = obj->next;
            gc_dispose_object(dvm, obj);
            obj = tmp;
        } else {
            obj = obj->next;
        }
    }
}

void
dvm_garbage_collect(DVM_VirtualMachine *dvm)
{
    gc_mark_objects(dvm);
    gc_sweep_objects(dvm);
}
