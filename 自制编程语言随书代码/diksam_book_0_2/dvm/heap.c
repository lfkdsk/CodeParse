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

DVM_Object *
alloc_array(DVM_VirtualMachine *dvm, ArrayType type, int size)
{
    DVM_Object *ret;

    ret = alloc_object(dvm, ARRAY_OBJECT);
    ret->u.array.type = type;
    ret->u.array.size = size;
    ret->u.array.alloc_size = size;

    return ret;
}

DVM_Object *
dvm_create_array_int_i(DVM_VirtualMachine *dvm, int size)
{
    DVM_Object *ret;

    ret = alloc_array(dvm, INT_ARRAY, size);
    ret->u.array.u.int_array = MEM_malloc(sizeof(int) * size);
    dvm->heap.current_heap_size += sizeof(int) * size;

    return ret;
}

DVM_Object *
DVM_create_array_int(DVM_VirtualMachine *dvm, int size)
{
    DVM_Object *ret;
    int i;

    ret = dvm_create_array_int_i(dvm, size);
    for (i = 0; i < size; i++) {
        ret->u.array.u.int_array[i] = 0;
    }

    return ret;
}

DVM_Object *
dvm_create_array_double_i(DVM_VirtualMachine *dvm, int size)
{
    DVM_Object *ret;

    ret = alloc_array(dvm, DOUBLE_ARRAY, size);
    ret->u.array.u.double_array = MEM_malloc(sizeof(double) * size);
    dvm->heap.current_heap_size += sizeof(double) * size;

    return ret;
}

DVM_Object *
DVM_create_array_double(DVM_VirtualMachine *dvm, int size)
{
    DVM_Object *ret;
    int i;

    ret = dvm_create_array_double_i(dvm, size);
    for (i = 0; i < size; i++) {
        ret->u.array.u.double_array[i] = 0.0;
    }

    return ret;
}

DVM_Object *
dvm_create_array_object_i(DVM_VirtualMachine *dvm, int size)
{
    DVM_Object *ret;

    ret = alloc_array(dvm, OBJECT_ARRAY, size);
    ret->u.array.u.object = MEM_malloc(sizeof(DVM_Object*) * size);
    dvm->heap.current_heap_size += sizeof(DVM_Object*) * size;

    return ret;
}

DVM_Object *
DVM_create_array_object(DVM_VirtualMachine *dvm, int size)
{
    DVM_Object *ret;
    int i;

    ret = dvm_create_array_object_i(dvm, size);
    for (i = 0; i < size; i++) {
        ret->u.array.u.object[i] = NULL;
    }

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

    if (obj->type == ARRAY_OBJECT && obj->u.array.type == OBJECT_ARRAY) {
        int i;

        for (i = 0; i < obj->u.array.size; i++) {
            gc_mark(obj->u.array.u.object[i]);
        }
    }
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
            == DVM_STRING_TYPE
            || (dvm->executable->global_variable[i].type->derive != NULL
                && (dvm->executable->global_variable[i].type->derive[0].tag
                    == DVM_ARRAY_DERIVE))) {
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
    case ARRAY_OBJECT:
        switch (obj->u.array.type) {
        case INT_ARRAY:
            dvm->heap.current_heap_size
                -= sizeof(int) * obj->u.array.alloc_size;
            MEM_free(obj->u.array.u.int_array);
            break;
        case DOUBLE_ARRAY:
            dvm->heap.current_heap_size
                -= sizeof(double) * obj->u.array.alloc_size;
            MEM_free(obj->u.array.u.double_array);
            break;
        case OBJECT_ARRAY:
            dvm->heap.current_heap_size
                -= sizeof(DVM_Object*) * obj->u.array.alloc_size;
            MEM_free(obj->u.array.u.object);
            break;
        default:
            DBG_assert(0, ("array.type..%d", obj->u.array.type));
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
