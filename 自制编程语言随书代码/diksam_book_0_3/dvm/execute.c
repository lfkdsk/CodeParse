#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

static DVM_ObjectRef
chain_string(DVM_VirtualMachine *dvm, DVM_ObjectRef str1, DVM_ObjectRef str2)
{
    int result_len;
    DVM_Char    *left;
    DVM_Char    *right;
    int         left_len;
    int         right_len;
    DVM_Char    *result;
    DVM_ObjectRef ret;

    if (str1.data == NULL) {
        left = NULL_STRING;
        left_len = dvm_wcslen(NULL_STRING);
    } else {
        left = str1.data->u.string.string;
        left_len = str1.data->u.string.length;
    }
    if (str2.data == NULL) {
        right = NULL_STRING;
        right_len = dvm_wcslen(NULL_STRING);
    } else {
        right = str2.data->u.string.string;
        right_len = str2.data->u.string.length;
    }
    result_len = left_len + right_len;
    result = MEM_malloc(sizeof(DVM_Char) * (result_len + 1));

    dvm_wcscpy(result, left);
    dvm_wcscat(result, right);

    ret = DVM_create_dvm_string(dvm, result);

    return ret;
}

static void
invoke_native_function(DVM_VirtualMachine *dvm,
                       Function *caller, Function *callee,
                       int pc, int *sp_p, int base)
{
    DVM_Value   *stack;
    DVM_Value   ret;
    int         arg_count;
    CallInfo *call_info;
    int i;

    (*sp_p)--; /* function index */
    stack = dvm->stack.stack;
    DBG_assert(callee->kind == NATIVE_FUNCTION,
               ("callee->kind..%d", callee->kind));

    if (callee->u.native_f.is_method) {
        arg_count = callee->u.native_f.arg_count + 1;
    } else {
        arg_count = callee->u.native_f.arg_count;
    }

    call_info = (CallInfo*)&dvm->stack.stack[*sp_p];
    call_info->caller = caller;
    call_info->caller_address = pc;
    call_info->base = base;
    for (i = 0; i < CALL_INFO_ALIGN_SIZE; i++) {
        dvm->stack.pointer_flags[*sp_p+i] = DVM_FALSE;
    }
    *sp_p += CALL_INFO_ALIGN_SIZE;
    dvm->current_function = callee;
    ret = callee->u.native_f.proc(dvm, callee->u.native_f.arg_count,
                                  &stack[*sp_p - arg_count
                                         - CALL_INFO_ALIGN_SIZE]);
    dvm->current_function = caller;
    *sp_p -= arg_count + CALL_INFO_ALIGN_SIZE;
    stack[*sp_p] = ret;
    dvm->stack.pointer_flags[*sp_p] = callee->u.native_f.return_pointer;
    (*sp_p)++;
}

static void
initialize_local_variables(DVM_VirtualMachine *dvm,
                           DVM_Function *func, int from_sp)
{
    int i;
    int sp_idx;

    for (i = 0, sp_idx = from_sp; i < func->local_variable_count;
         i++, sp_idx++) {
        dvm->stack.pointer_flags[sp_idx] = DVM_FALSE;
    }

    for (i = 0, sp_idx = from_sp; i < func->local_variable_count;
         i++, sp_idx++) {
        dvm_initialize_value(func->local_variable[i].type,
                             &dvm->stack.stack[sp_idx]);
        if (is_pointer_type(func->local_variable[i].type)) {
            dvm->stack.pointer_flags[sp_idx] = DVM_TRUE;
        }
    }
}

void
dvm_expand_stack(DVM_VirtualMachine *dvm, int need_stack_size)
{
    int revalue_up;
    int rest;

    rest = dvm->stack.alloc_size - dvm->stack.stack_pointer;
    if (rest <= need_stack_size) {
        revalue_up = ((rest / STACK_ALLOC_SIZE) + 1) * STACK_ALLOC_SIZE;

        dvm->stack.alloc_size += revalue_up;
        dvm->stack.stack
            = MEM_realloc(dvm->stack.stack,
                          dvm->stack.alloc_size * sizeof(DVM_Value));
        dvm->stack.pointer_flags
            = MEM_realloc(dvm->stack.pointer_flags,
                          dvm->stack.alloc_size * sizeof(DVM_Boolean));
    }
}

static void
invoke_diksam_function(DVM_VirtualMachine *dvm,
                       Function **caller_p, Function *callee,
                       DVM_Byte **code_p, int *code_size_p, int *pc_p,
                       int *sp_p, int *base_p,
                       ExecutableEntry **ee_p, DVM_Executable **exe_p)
{
    CallInfo *call_info;
    DVM_Function *callee_p;
    int i;

    if (!callee->is_implemented) {
        dvm_dynamic_load(dvm, *exe_p, *caller_p, *pc_p, callee);
    }
    *ee_p = callee->u.diksam_f.executable;
    *exe_p = (*ee_p)->executable;
    callee_p = &(*exe_p)->function[callee->u.diksam_f.index];

    dvm_expand_stack(dvm,
                     CALL_INFO_ALIGN_SIZE
                     + callee_p->local_variable_count
                     + (*exe_p)->function[callee->u.diksam_f.index]
                     .need_stack_size);

    call_info = (CallInfo*)&dvm->stack.stack[*sp_p-1];
    call_info->caller = *caller_p;
    call_info->caller_address = *pc_p;
    call_info->base = *base_p;
    for (i = 0; i < CALL_INFO_ALIGN_SIZE; i++) {
        dvm->stack.pointer_flags[*sp_p-1+i] = DVM_FALSE;
    }

    *base_p = *sp_p - callee_p->parameter_count - 1;
    if (callee_p->is_method) {
        (*base_p)--; /* for this */
    }
    *caller_p = callee;

    initialize_local_variables(dvm, callee_p,
                               *sp_p + CALL_INFO_ALIGN_SIZE - 1);

    *sp_p += CALL_INFO_ALIGN_SIZE + callee_p->local_variable_count - 1;
    *pc_p = 0;

    *code_p = (*exe_p)->function[callee->u.diksam_f.index].code;
    *code_size_p
        = (*exe_p)->function[callee->u.diksam_f.index].code_size;
}

/* This function returns DVM_TRUE if this function was called from native.
 */
static DVM_Boolean
do_return(DVM_VirtualMachine *dvm, Function **func_p,
          DVM_Byte **code_p, int *code_size_p, int *pc_p, int *base_p,
          ExecutableEntry **ee_p, DVM_Executable **exe_p)
{
    CallInfo *call_info;
    DVM_Function *caller_p;
    DVM_Function *callee_p;
    int arg_count;

    callee_p = &(*exe_p)->function[(*func_p)->u.diksam_f.index];

    arg_count = callee_p->parameter_count;
    if (callee_p->is_method) {
        arg_count++; /* for this */
    }
    call_info = (CallInfo*)&dvm->stack.stack[*base_p + arg_count];

    if (call_info->caller) {
        *ee_p = call_info->caller->u.diksam_f.executable;
        *exe_p = (*ee_p)->executable;
        if (call_info->caller->kind == DIKSAM_FUNCTION) {
            caller_p
                = &(*exe_p)->function[call_info->caller->u.diksam_f.index];
            *code_p = caller_p->code;
            *code_size_p = caller_p->code_size;
        }
    } else {
        *ee_p = dvm->top_level;
        *exe_p = dvm->top_level->executable;
        *code_p = dvm->top_level->executable->code;
        *code_size_p = dvm->top_level->executable->code_size;
    }
    *func_p = call_info->caller;

    dvm->stack.stack_pointer = *base_p;
    *pc_p = call_info->caller_address + 1;
    *base_p = call_info->base;

    return call_info->caller_address == CALL_FROM_NATIVE;
}

/* This function returns DVM_TRUE if this function was called from native.
 */
static DVM_Boolean
return_function(DVM_VirtualMachine *dvm, Function **func_p,
                DVM_Byte **code_p, int *code_size_p, int *pc_p, int *base_p,
                ExecutableEntry **ee_p, DVM_Executable **exe_p)
{
    DVM_Value return_value;
    DVM_Boolean ret;
    DVM_Function *callee_func;

    return_value = dvm->stack.stack[dvm->stack.stack_pointer-1];
    dvm->stack.stack_pointer--;
    callee_func = &(*exe_p)->function[(*func_p)->u.diksam_f.index];

    ret = do_return(dvm, func_p, code_p, code_size_p, pc_p, base_p,
                    ee_p, exe_p);

    dvm->stack.stack[dvm->stack.stack_pointer] = return_value;
    dvm->stack.pointer_flags[dvm->stack.stack_pointer]
        = is_pointer_type(callee_func->type);
    dvm->stack.stack_pointer++;

    return ret;
}

#define STI(dvm, sp) \
  ((dvm)->stack.stack[(dvm)->stack.stack_pointer+(sp)].int_value)
#define STD(dvm, sp) \
  ((dvm)->stack.stack[(dvm)->stack.stack_pointer+(sp)].double_value)
#define STO(dvm, sp) \
  ((dvm)->stack.stack[(dvm)->stack.stack_pointer+(sp)].object)

#define STI_I(dvm, sp) \
  ((dvm)->stack.stack[(sp)].int_value)
#define STD_I(dvm, sp) \
  ((dvm)->stack.stack[(sp)].double_value)
#define STO_I(dvm, sp) \
  ((dvm)->stack.stack[(sp)].object)

#define STI_WRITE(dvm, sp, r) \
  ((dvm)->stack.stack[(dvm)->stack.stack_pointer+(sp)].int_value = r,\
   (dvm)->stack.pointer_flags[(dvm)->stack.stack_pointer+(sp)] = DVM_FALSE)
#define STD_WRITE(dvm, sp, r) \
  ((dvm)->stack.stack[(dvm)->stack.stack_pointer+(sp)].double_value = r, \
   (dvm)->stack.pointer_flags[(dvm)->stack.stack_pointer+(sp)] = DVM_FALSE)
#define STO_WRITE(dvm, sp, r) \
  ((dvm)->stack.stack[(dvm)->stack.stack_pointer+(sp)].object = r, \
   (dvm)->stack.pointer_flags[(dvm)->stack.stack_pointer+(sp)] = DVM_TRUE)

#define STI_WRITE_I(dvm, sp, r) \
  ((dvm)->stack.stack[(sp)].int_value = r,\
   (dvm)->stack.pointer_flags[(sp)] = DVM_FALSE)
#define STD_WRITE_I(dvm, sp, r) \
  ((dvm)->stack.stack[(sp)].double_value = r, \
   (dvm)->stack.pointer_flags[(sp)] = DVM_FALSE)
#define STO_WRITE_I(dvm, sp, r) \
  ((dvm)->stack.stack[(sp)].object = r, \
   (dvm)->stack.pointer_flags[(sp)] = DVM_TRUE)

DVM_ObjectRef
create_array_sub(DVM_VirtualMachine *dvm, int dim, int dim_index,
                 DVM_TypeSpecifier *type)
{
    DVM_ObjectRef ret;
    int size;
    int i;

    size = STI(dvm, -dim);

    if (dim_index == type->derive_count-1) {
        switch (type->basic_type) {
        case DVM_VOID_TYPE:
            DBG_panic(("creating void array"));
            break;
        case DVM_BOOLEAN_TYPE: /* FALLTHRU */
        case DVM_INT_TYPE:
            ret = dvm_create_array_int_i(dvm, size);
            break;
        case DVM_DOUBLE_TYPE:
            ret = dvm_create_array_double_i(dvm, size);
            break;
        case DVM_STRING_TYPE: /* FALLTHRU */
        case DVM_CLASS_TYPE:
            ret = dvm_create_array_object_i(dvm, size);
            break;
        case DVM_NULL_TYPE: /* FALLTHRU */
        case DVM_BASE_TYPE: /* FALLTHRU */
        default:
            DBG_assert(0, ("type->basic_type..%d\n", type->basic_type));
            break;
        }
    } else if (type->derive[dim_index].tag == DVM_FUNCTION_DERIVE) {
        DBG_panic(("Function type in array literal.\n"));
    } else {
        ret = dvm_create_array_object_i(dvm, size);
        if (dim_index < dim - 1) {
            STO_WRITE(dvm, 0, ret);
            dvm->stack.stack_pointer++;
            for (i = 0; i < size; i++) {
                DVM_ObjectRef child;
                child = create_array_sub(dvm, dim, dim_index+1, type);
                DVM_array_set_object(dvm, ret, i, child);
            }
            dvm->stack.stack_pointer--;
        }
    }
    return ret;
}

DVM_ObjectRef
create_array(DVM_VirtualMachine *dvm, int dim, DVM_TypeSpecifier *type)
{
    return create_array_sub(dvm, dim, 0, type);
}

DVM_ObjectRef
create_array_literal_int(DVM_VirtualMachine *dvm, int size)
{
    DVM_ObjectRef array;
    int i;

    array = dvm_create_array_int_i(dvm, size);
    for (i = 0; i < size; i++) {
        array.data->u.array.u.int_array[i] = STI(dvm, -size+i);
    }

    return array;
}

DVM_ObjectRef
create_array_literal_double(DVM_VirtualMachine *dvm, int size)
{
    DVM_ObjectRef array;
    int i;

    array = dvm_create_array_double_i(dvm, size);
    for (i = 0; i < size; i++) {
        array.data->u.array.u.double_array[i] = STD(dvm, -size+i);
    }

    return array;
}

DVM_ObjectRef
create_array_literal_object(DVM_VirtualMachine *dvm, int size)
{
    DVM_ObjectRef array;
    int i;

    array = dvm_create_array_object_i(dvm, size);
    for (i = 0; i < size; i++) {
        array.data->u.array.u.object[i] = STO(dvm, -size+i);
    }

    return array;
}

static void
restore_pc(DVM_VirtualMachine *dvm, ExecutableEntry *ee,
           Function *func, int pc)
{
    dvm->current_executable = ee;
    dvm->current_function = func;
    dvm->pc = pc;
}

static void
check_null_pointer_func(DVM_Executable *exe, Function *func, int pc,
                        DVM_ObjectRef *obj)
{
    if (obj->data == NULL) {
        dvm_error_i(exe, func, pc, NULL_POINTER_ERR, DVM_MESSAGE_ARGUMENT_END);
    }
}

#define check_null_pointer(exe, func, pc, obj)\
  {if ((obj)->data == NULL) \
    check_null_pointer_func((exe), (func), (pc), (obj));}

static DVM_Boolean
check_instanceof_i(DVM_VirtualMachine *dvm, DVM_ObjectRef *obj,
                   int target_idx,
                   DVM_Boolean *is_interface, int *interface_idx)
{
    ExecClass *pos;
    int i;

    for (pos = obj->v_table->exec_class->super_class; pos;
         pos = pos->super_class) {
        if (pos->class_index == target_idx) {
            *is_interface = DVM_FALSE;
            return DVM_TRUE;
        }
    }

    for (i = 0; i < obj->v_table->exec_class->interface_count; i++) {
        if (obj->v_table->exec_class->interface[i]->class_index
            == target_idx) {
            *is_interface = DVM_TRUE;
            *interface_idx = i;
            return DVM_TRUE;
        }
    }
    return DVM_FALSE;
}

static DVM_Boolean
check_instanceof(DVM_VirtualMachine *dvm, DVM_ObjectRef *obj,
                 int target_idx)
{
    DVM_Boolean is_interface_dummy;
    int interface_idx_dummy;

    return check_instanceof_i(dvm, obj, target_idx,
                              &is_interface_dummy, &interface_idx_dummy);
}

static void
check_down_cast(DVM_VirtualMachine *dvm, DVM_Executable *exe,
                Function *func, int pc,
                DVM_ObjectRef *obj, int target_idx,
                DVM_Boolean *is_same_class,
                DVM_Boolean *is_interface, int *interface_index)
{
    if (obj->v_table->exec_class->class_index == target_idx) {
        *is_same_class = DVM_TRUE;
        return;
    }
    *is_same_class = DVM_FALSE;

    if (!check_instanceof_i(dvm, obj, target_idx,
                            is_interface, interface_index)) {
        dvm_error_i(exe, func, pc, CLASS_CAST_ERR,
                    DVM_STRING_MESSAGE_ARGUMENT, "org",
                    obj->v_table->exec_class->name,
                    DVM_STRING_MESSAGE_ARGUMENT, "target",
                    dvm->class[target_idx]->name,
                    DVM_MESSAGE_ARGUMENT_END);

    }
}

DVM_Value dvm_execute_i(DVM_VirtualMachine *dvm, Function *func,
                        DVM_Byte *code, int code_size, int base);

DVM_Value
dvm_execute_i(DVM_VirtualMachine *dvm, Function *func,
              DVM_Byte *code, int code_size, int base)
{
    ExecutableEntry *ee;
    DVM_Executable *exe;
    int         pc;
    DVM_Value   ret;

    pc = dvm->pc;
    ee = dvm->current_executable;
    exe = dvm->current_executable->executable;

    while (pc < code_size) {
        /*
        dvm_dump_instruction(stderr, code, pc);
        fprintf(stderr, "\tsp(%d)\n", dvm->stack.stack_pointer);
        */
        switch ((DVM_Opcode)code[pc]) {
        case DVM_PUSH_INT_1BYTE:
            STI_WRITE(dvm, 0, code[pc+1]);
            dvm->stack.stack_pointer++;
            pc += 2;
            break;
        case DVM_PUSH_INT_2BYTE:
            STI_WRITE(dvm, 0, GET_2BYTE_INT(&code[pc+1]));
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        case DVM_PUSH_INT:
            STI_WRITE(dvm, 0,
                      exe->constant_pool[GET_2BYTE_INT(&code[pc+1])]
                      .u.c_int);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        case DVM_PUSH_DOUBLE_0:
            STD_WRITE(dvm, 0, 0.0);
            dvm->stack.stack_pointer++;
            pc++;
            break;
        case DVM_PUSH_DOUBLE_1:
            STD_WRITE(dvm, 0, 1.0);
            dvm->stack.stack_pointer++;
            pc++;
            break;
        case DVM_PUSH_DOUBLE:
            STD_WRITE(dvm, 0, 
                      exe->constant_pool[GET_2BYTE_INT(&code[pc+1])]
                      .u.c_double);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        case DVM_PUSH_STRING:
            STO_WRITE(dvm, 0,
                      dvm_literal_to_dvm_string_i(dvm,
                                                  exe->constant_pool
                                                  [GET_2BYTE_INT(&code
                                                                 [pc+1])]
                                                  .u.c_string));
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        case DVM_PUSH_NULL:
            STO_WRITE(dvm, 0, dvm_null_object_ref);
            dvm->stack.stack_pointer++;
            pc++;
            break;
        case DVM_PUSH_STACK_INT:
            STI_WRITE(dvm, 0,
                      STI_I(dvm, base + GET_2BYTE_INT(&code[pc+1])));
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        case DVM_PUSH_STACK_DOUBLE:
            STD_WRITE(dvm, 0,
                      STD_I(dvm, base + GET_2BYTE_INT(&code[pc+1])));
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        case DVM_PUSH_STACK_OBJECT:
            STO_WRITE(dvm, 0,
                      STO_I(dvm, base + GET_2BYTE_INT(&code[pc+1])));
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        case DVM_POP_STACK_INT:
            STI_WRITE_I(dvm, base + GET_2BYTE_INT(&code[pc+1]),
                        STI(dvm, -1));
            dvm->stack.stack_pointer--;
            pc += 3;
            break;
        case DVM_POP_STACK_DOUBLE:
            STD_WRITE_I(dvm, base + GET_2BYTE_INT(&code[pc+1]),
                        STD(dvm, -1));
            dvm->stack.stack_pointer--;
            pc += 3;
            break;
        case DVM_POP_STACK_OBJECT:
            STO_WRITE_I(dvm, base + GET_2BYTE_INT(&code[pc+1]),
                        STO(dvm, -1));
            dvm->stack.stack_pointer--;
            pc += 3;
            break;
        case DVM_PUSH_STATIC_INT:
            STI_WRITE(dvm, 0,
                      ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])]
                      .int_value);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        case DVM_PUSH_STATIC_DOUBLE:
            STD_WRITE(dvm, 0,
                      ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])]
                      .double_value);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        case DVM_PUSH_STATIC_OBJECT:
            STO_WRITE(dvm, 0,
                      ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])]
                      .object);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        case DVM_POP_STATIC_INT:
            ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])].int_value
                = STI(dvm, -1);
            dvm->stack.stack_pointer--;
            pc += 3;
            break;
        case DVM_POP_STATIC_DOUBLE:
            ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])]
                .double_value
                = STD(dvm, -1);
            dvm->stack.stack_pointer--;
            pc += 3;
            break;
        case DVM_POP_STATIC_OBJECT:
            ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])].object
                = STO(dvm, -1);
            dvm->stack.stack_pointer--;
            pc += 3;
            break;
        case DVM_PUSH_ARRAY_INT:
        {
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);
            int int_value;

            restore_pc(dvm, ee, func, pc);

            int_value = DVM_array_get_int(dvm, array, index);
            STI_WRITE(dvm, -2, int_value);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        }
        case DVM_PUSH_ARRAY_DOUBLE:
        {
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);
            double double_value;

            restore_pc(dvm, ee, func, pc);
            double_value = DVM_array_get_double(dvm, array, index);
            STD_WRITE(dvm, -2, double_value);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        }
        case DVM_PUSH_ARRAY_OBJECT:
        {
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);
            DVM_ObjectRef object;

            restore_pc(dvm, ee, func, pc);
            object = DVM_array_get_object(dvm, array, index);

            STO_WRITE(dvm, -2, object);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        }
        case DVM_POP_ARRAY_INT:
        {
            int value = STI(dvm, -3);
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);

            restore_pc(dvm, ee, func, pc);
            DVM_array_set_int(dvm, array, index, value);
            dvm->stack.stack_pointer -= 3;
            pc++;
            break;
        }
        case DVM_POP_ARRAY_DOUBLE:
        {
            double value = STD(dvm, -3);
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);

            restore_pc(dvm, ee, func, pc);
            DVM_array_set_double(dvm, array, index, value);
            dvm->stack.stack_pointer -= 3;
            pc++;
            break;
        }
        case DVM_POP_ARRAY_OBJECT:
        {
            DVM_ObjectRef value = STO(dvm, -3);
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);

            restore_pc(dvm, ee, func, pc);
            DVM_array_set_object(dvm, array, index, value);
            dvm->stack.stack_pointer -= 3;
            pc++;
            break;
        }
        case DVM_PUSH_FIELD_INT:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            check_null_pointer(exe, func, pc, &obj);
            STI_WRITE(dvm, -1,
                      obj.data->u.class_object.field[index].int_value);
            pc += 3;
            break;
        }
        case DVM_PUSH_FIELD_DOUBLE:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            check_null_pointer(exe, func, pc, &obj);
            STD_WRITE(dvm, -1,
                      obj.data->u.class_object.field[index].double_value);
            pc += 3;
            break;
        }
        case DVM_PUSH_FIELD_OBJECT:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            check_null_pointer(exe, func, pc, &obj);
            STO_WRITE(dvm, -1, obj.data->u.class_object.field[index].object);
            pc += 3;
            break;
        }
        case DVM_POP_FIELD_INT:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            check_null_pointer(exe, func, pc, &obj);
            obj.data->u.class_object.field[index].int_value = STI(dvm, -2);
            dvm->stack.stack_pointer -= 2;
            pc += 3;
            break;
        }
        case DVM_POP_FIELD_DOUBLE:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            check_null_pointer(exe, func, pc, &obj);
            obj.data->u.class_object.field[index].double_value = STD(dvm, -2);
            dvm->stack.stack_pointer -= 2;
            pc += 3;
            break;
        }
        case DVM_POP_FIELD_OBJECT:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            check_null_pointer(exe, func, pc, &obj);
            obj.data->u.class_object.field[index].object = STO(dvm, -2);
            dvm->stack.stack_pointer -= 2;
            pc += 3;
            break;
        }
        case DVM_ADD_INT:
            STI(dvm, -2) = STI(dvm, -2) + STI(dvm, -1);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_ADD_DOUBLE:
            STD(dvm, -2) = STD(dvm, -2) + STD(dvm, -1);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_ADD_STRING:
            STO(dvm, -2) = chain_string(dvm,
                                        STO(dvm, -2),
                                        STO(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_SUB_INT:
            STI(dvm, -2) = STI(dvm, -2) - STI(dvm, -1);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_SUB_DOUBLE:
            STD(dvm, -2) = STD(dvm, -2) - STD(dvm, -1);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_MUL_INT:
            STI(dvm, -2) = STI(dvm, -2) * STI(dvm, -1);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_MUL_DOUBLE:
            STD(dvm, -2) = STD(dvm, -2) * STD(dvm, -1);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_DIV_INT:
            if (STI(dvm, -1) == 0) {
                dvm_error_i(exe, func, pc, DIVISION_BY_ZERO_ERR,
                            DVM_MESSAGE_ARGUMENT_END);
            }
            STI(dvm, -2) = STI(dvm, -2) / STI(dvm, -1);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_DIV_DOUBLE:
            STD(dvm, -2) = STD(dvm, -2) / STD(dvm, -1);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_MOD_INT:
            STI(dvm, -2) = STI(dvm, -2) % STI(dvm, -1);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_MOD_DOUBLE:
            STD(dvm, -2) = fmod(STD(dvm, -2), STD(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_MINUS_INT:
            STI(dvm, -1) = -STI(dvm, -1);
            pc++;
            break;
        case DVM_MINUS_DOUBLE:
            STD(dvm, -1) = -STD(dvm, -1);
            pc++;
            break;
        case DVM_INCREMENT:
            STI(dvm, -1)++;
            pc++;
            break;
        case DVM_DECREMENT:
            STI(dvm, -1)--;
            pc++;
            break;
        case DVM_CAST_INT_TO_DOUBLE:
            STD(dvm, -1) = (double)STI(dvm, -1);
            pc++;
            break;
        case DVM_CAST_DOUBLE_TO_INT:
            STI(dvm, -1) = (int)STD(dvm, -1);
            pc++;
            break;
        case DVM_CAST_BOOLEAN_TO_STRING:
            if (STI(dvm, -1)) {
                STO_WRITE(dvm, -1,
                          dvm_literal_to_dvm_string_i(dvm, TRUE_STRING));
            } else {
                STO_WRITE(dvm, -1,
                          dvm_literal_to_dvm_string_i(dvm, FALSE_STRING));
            }
            pc++;
            break;
        case DVM_CAST_INT_TO_STRING:
        {
            char buf[LINE_BUF_SIZE];
            DVM_Char *wc_str;

            sprintf(buf, "%d", STI(dvm, -1));
            restore_pc(dvm, ee, func, pc);
            wc_str = dvm_mbstowcs_alloc(dvm, buf);
            STO_WRITE(dvm, -1,
                      DVM_create_dvm_string(dvm, wc_str));
            pc++;
            break;
        }
        case DVM_CAST_DOUBLE_TO_STRING:
        {
            char buf[LINE_BUF_SIZE];
            DVM_Char *wc_str;

            sprintf(buf, "%f", STD(dvm, -1));
            restore_pc(dvm, ee, func, pc);
            wc_str = dvm_mbstowcs_alloc(dvm, buf);
            STO_WRITE(dvm, -1,
                      DVM_create_dvm_string(dvm, wc_str));
            pc++;
            break;
        }
        case DVM_UP_CAST:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            check_null_pointer(exe, func, pc, &obj);
            obj.v_table = obj.v_table->exec_class->interface_v_table[index];
            STO_WRITE(dvm, -1, obj);
            pc += 3;
            break;
        }
        case DVM_DOWN_CAST:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);
            DVM_Boolean is_same_class;
            DVM_Boolean is_interface;
            int interface_idx;

            check_null_pointer(exe, func, pc, &obj);
            check_down_cast(dvm, exe, func, pc, &obj, index,
                            &is_same_class, &is_interface, &interface_idx);

            check_down_cast(dvm, exe, func, pc, &obj, index,
                            &is_same_class, &is_interface, &interface_idx);
            if (!is_same_class) {
                if (is_interface) {
                    obj.v_table
                        = obj.v_table->exec_class
                        ->interface_v_table[interface_idx];
                } else {
                    obj.v_table = obj.v_table->exec_class->class_table;
                }
            }
            STO_WRITE(dvm, -1, obj);
            pc += 3;
            break;
        }
        case DVM_EQ_INT:
            STI(dvm, -2) = (STI(dvm, -2) == STI(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_EQ_DOUBLE:
            STI(dvm, -2) = (STD(dvm, -2) == STD(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_EQ_OBJECT:
            STI_WRITE(dvm, -2, STO(dvm, -2).data == STO(dvm, -1).data);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_EQ_STRING:
            STI_WRITE(dvm, -2,
                      !dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                  STO(dvm, -1).data->u.string.string));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_GT_INT:
            STI(dvm, -2) = (STI(dvm, -2) > STI(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_GT_DOUBLE:
            STI(dvm, -2) = (STD(dvm, -2) > STD(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_GT_STRING:
            STI_WRITE(dvm, -2,
                      dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                 STO(dvm, -1).data->u.string.string) > 0);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_GE_INT:
            STI(dvm, -2) = (STI(dvm, -2) >= STI(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_GE_DOUBLE:
            STI(dvm, -2) = (STD(dvm, -2) >= STD(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_GE_STRING:
            STI_WRITE(dvm, -2,
                      dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                 STO(dvm, -1).data->u.string.string) >= 0);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_LT_INT:
            STI(dvm, -2) = (STI(dvm, -2) < STI(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_LT_DOUBLE:
            STI(dvm, -2) = (STD(dvm, -2) < STD(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_LT_STRING:
            STI_WRITE(dvm, -2,
                      dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                 STO(dvm, -1).data->u.string.string) < 0);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_LE_INT:
            STI(dvm, -2) = (STI(dvm, -2) <= STI(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_LE_DOUBLE:
            STI(dvm, -2) = (STD(dvm, -2) <= STD(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_LE_STRING:
            STI_WRITE(dvm, -2,
                      dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                 STO(dvm, -1).data->u.string.string) <= 0);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_NE_INT:
            STI(dvm, -2) = (STI(dvm, -2) != STI(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_NE_DOUBLE:
            STI(dvm, -2) = (STD(dvm, -2) != STD(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_NE_OBJECT:
            STI_WRITE(dvm, -2, STO(dvm, -2).data != STO(dvm, -1).data);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_NE_STRING:
            STI_WRITE(dvm, -2,
                      dvm_wcscmp(STO(dvm, -2).data->u.string.string,
                                 STO(dvm, -1).data->u.string.string) != 0);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_LOGICAL_AND:
            STI(dvm, -2) = (STI(dvm, -2) && STI(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_LOGICAL_OR:
            STI(dvm, -2) = (STI(dvm, -2) || STI(dvm, -1));
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_LOGICAL_NOT:
            STI(dvm, -1) = !STI(dvm, -1);
            pc++;
            break;
        case DVM_POP:
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_DUPLICATE:
            dvm->stack.stack[dvm->stack.stack_pointer]
                = dvm->stack.stack[dvm->stack.stack_pointer-1];
            dvm->stack.pointer_flags[dvm->stack.stack_pointer]
                = dvm->stack.pointer_flags[dvm->stack.stack_pointer-1];
            dvm->stack.stack_pointer++;
            pc++;
            break;
        case DVM_DUPLICATE_OFFSET:
            {
                int offset = GET_2BYTE_INT(&code[pc+1]);
                dvm->stack.stack[dvm->stack.stack_pointer]
                    = dvm->stack.stack[dvm->stack.stack_pointer-1-offset];
                dvm->stack.pointer_flags[dvm->stack.stack_pointer]
                    = dvm->stack.pointer_flags[dvm->stack.stack_pointer-1
                                               -offset];
                dvm->stack.stack_pointer++;
                pc += 3;
                break;
            }
        case DVM_JUMP:
            pc = GET_2BYTE_INT(&code[pc+1]);
            break;
        case DVM_JUMP_IF_TRUE:
            if (STI(dvm, -1)) {
                pc = GET_2BYTE_INT(&code[pc+1]);
            } else {
                pc += 3;
            }
            dvm->stack.stack_pointer--;
            break;
        case DVM_JUMP_IF_FALSE:
            if (!STI(dvm, -1)) {
                pc = GET_2BYTE_INT(&code[pc+1]);
            } else {
                pc += 3;
            }
            dvm->stack.stack_pointer--;
            break;
        case DVM_PUSH_FUNCTION:
        {
            STI_WRITE(dvm, 0, GET_2BYTE_INT(&code[pc+1]));
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case DVM_PUSH_METHOD:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            check_null_pointer(exe, func, pc, &obj);
            STI_WRITE(dvm, 0, obj.v_table->table[index].index);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case DVM_INVOKE:
        {
            int func_idx;

            func_idx = STI(dvm, -1);
            if (dvm->function[func_idx]->kind == NATIVE_FUNCTION) {
                restore_pc(dvm, ee, func, pc);
                invoke_native_function(dvm, func, dvm->function[func_idx],
                                       pc, &dvm->stack.stack_pointer, base);
                pc++;
            } else {
                invoke_diksam_function(dvm, &func, dvm->function[func_idx],
                                       &code, &code_size, &pc,
                                       &dvm->stack.stack_pointer, &base,
                                       &ee, &exe);
            }
            break;
        }
        case DVM_RETURN:
            if (return_function(dvm, &func, &code, &code_size, &pc,
                                &base, &ee, &exe)) {
                ret = dvm->stack.stack[dvm->stack.stack_pointer-1];
                goto EXECUTE_END;
            }
            break;
        case DVM_NEW:
        {
            int class_index = GET_2BYTE_INT(&code[pc+1]);
            STO_WRITE(dvm, 0, dvm_create_class_object_i(dvm, class_index));
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case DVM_NEW_ARRAY:
        {
            int dim = code[pc+1];
            DVM_TypeSpecifier *type
                = &exe->type_specifier[GET_2BYTE_INT(&code[pc+2])];
            DVM_ObjectRef array;

            restore_pc(dvm, ee, func, pc);
            array = create_array(dvm, dim, type);
            dvm->stack.stack_pointer -= dim;
            STO_WRITE(dvm, 0, array);
            dvm->stack.stack_pointer++;
            pc += 4;
            break;
        }
        case DVM_NEW_ARRAY_LITERAL_INT:
        {
            int size = GET_2BYTE_INT(&code[pc+1]);
            DVM_ObjectRef array;

            restore_pc(dvm, ee, func, pc);
            array = create_array_literal_int(dvm, size);
            dvm->stack.stack_pointer -= size;
            STO_WRITE(dvm, 0, array);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case DVM_NEW_ARRAY_LITERAL_DOUBLE:
        {
            int size = GET_2BYTE_INT(&code[pc+1]);
            DVM_ObjectRef array;

            restore_pc(dvm, ee, func, pc);
            array = create_array_literal_double(dvm, size);
            dvm->stack.stack_pointer -= size;
            STO_WRITE(dvm, 0, array);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case DVM_NEW_ARRAY_LITERAL_OBJECT:
        {
            int size = GET_2BYTE_INT(&code[pc+1]);
            DVM_ObjectRef array;

            restore_pc(dvm, ee, func, pc);
            array = create_array_literal_object(dvm, size);
            dvm->stack.stack_pointer -= size;
            STO_WRITE(dvm, 0, array);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case DVM_SUPER:
        {
            DVM_ObjectRef* obj = &STO(dvm, -1);
            ExecClass *this_class;
            
            this_class = obj->v_table->exec_class;
            obj->v_table = this_class->super_class->class_table;
            pc++;
            break;
        }
        case DVM_INSTANCEOF:
        {
            DVM_ObjectRef* obj = &STO(dvm, -1);
            int target_idx = GET_2BYTE_INT(&code[pc+1]);

            if (obj->v_table->exec_class->class_index == target_idx) {
                STI_WRITE(dvm, -1, DVM_TRUE);
            } else {
                STI_WRITE(dvm, -1, check_instanceof(dvm, obj, target_idx));
            }
            pc += 3;
            break;
        }
        default:
            DBG_assert(0, ("code[%d]..%d\n", pc, code[pc]));
        }
        /* MEM_check_all_blocks(); */
    }
 EXECUTE_END:
    ;
    return ret;
}

void
dvm_push_object(DVM_VirtualMachine *dvm, DVM_Value value)
{
    STO_WRITE(dvm, 0, value.object);
    dvm->stack.stack_pointer++;
}

DVM_Value
dvm_pop_object(DVM_VirtualMachine *dvm)
{
    DVM_Value ret;

    ret.object = STO(dvm, -1);
    dvm->stack.stack_pointer--;

    return ret;
}

DVM_Value
DVM_execute(DVM_VirtualMachine *dvm)
{
    DVM_Value ret;

    dvm->current_executable = dvm->top_level;
    dvm->current_function = NULL;
    dvm->pc = 0;
    dvm_expand_stack(dvm,
                     dvm->top_level->executable->need_stack_size);
    dvm_execute_i(dvm, NULL, dvm->top_level->executable->code,
                  dvm->top_level->executable->code_size, 0);

    return ret;
}

void DVM_dispose_executable_list(DVM_ExecutableList *list)
{
    DVM_ExecutableItem *temp;

    while (list->list) {
        temp = list->list;
        list->list = temp->next;
        dvm_dispose_executable(temp->executable);
        MEM_free(temp);
    }
    MEM_free(list);
}

static void
dispose_v_table(DVM_VTable *v_table)
{
    int i;

    for (i = 0; i < v_table->table_size; i++) {
        MEM_free(v_table->table[i].name);
    }
    MEM_free(v_table->table);
    MEM_free(v_table);
}

void
DVM_dispose_virtual_machine(DVM_VirtualMachine *dvm)
{
    ExecutableEntry *ee_temp;
    int i;
    int j;

    while (dvm->executable_entry) {
        ee_temp = dvm->executable_entry;
        dvm->executable_entry = ee_temp->next;

        MEM_free(ee_temp->static_v.variable);
        MEM_free(ee_temp);
    }
    dvm_garbage_collect(dvm);

    MEM_free(dvm->stack.stack);
    MEM_free(dvm->stack.pointer_flags);


    for (i = 0; i < dvm->function_count; i++) {
        MEM_free(dvm->function[i]->name);
        MEM_free(dvm->function[i]->package_name);
        MEM_free(dvm->function[i]);
    }
    MEM_free(dvm->function);

    for (i = 0; i < dvm->class_count; i++) {
        MEM_free(dvm->class[i]->package_name);
        MEM_free(dvm->class[i]->name);
        dispose_v_table(dvm->class[i]->class_table);
        for (j = 0; j < dvm->class[i]->interface_count; j++) {
            dispose_v_table(dvm->class[i]->interface_v_table[j]);
        }
        MEM_free(dvm->class[i]->interface_v_table);
        MEM_free(dvm->class[i]->interface);
        MEM_free(dvm->class[i]->field_type);
        MEM_free(dvm->class[i]);
    }

    MEM_free(dvm->array_v_table->table);
    MEM_free(dvm->array_v_table);
    MEM_free(dvm->string_v_table->table);
    MEM_free(dvm->string_v_table);
    MEM_free(dvm->class);

    MEM_free(dvm);
}
