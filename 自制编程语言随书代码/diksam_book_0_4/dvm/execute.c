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

    ret = dvm_create_dvm_string_i(dvm, result);

    return ret;
}

static DVM_Context *
create_context(DVM_VirtualMachine *dvm)
{
    DVM_Context *ret;

    ret = MEM_malloc(sizeof(DVM_Context));
    ret->ref_in_native_method = NULL;

    return ret;
}


DVM_Context *
DVM_push_context(DVM_VirtualMachine *dvm)
{
    DVM_Context *ret;

    ret = create_context(dvm);
    ret->next = dvm->current_context;
    dvm->current_context = ret;

    return ret;
}

DVM_Context *
DVM_create_context(DVM_VirtualMachine *dvm)
{
    DVM_Context *ret;

    ret = create_context(dvm);
    ret->next = dvm->free_context;
    dvm->free_context = ret;

    return ret;
}

static void
dispose_context(DVM_Context *context)
{
    RefInNativeFunc *pos;
    RefInNativeFunc *temp;

    for (pos = context->ref_in_native_method; pos; ) {
        temp = pos->next;
        MEM_free(pos);
        pos = temp;
    }
    MEM_free(context);
}

void
DVM_pop_context(DVM_VirtualMachine *dvm, DVM_Context *context)
{
    DBG_assert(dvm->current_context == context,
               ("context is not current context."
                "current_context..%p, context..%p",
                dvm->current_context, context));

    dvm->current_context = context->next;
    dispose_context(context);
}

void
DVM_dispose_context(DVM_VirtualMachine *dvm, DVM_Context *context)
{
    DVM_Context *prev = NULL;
    DVM_Context *pos;

    for (pos = dvm->free_context; pos; pos = pos->next) {
        if (pos == context)
            break;
        prev = pos;
    }
    if (prev == NULL) {
        dvm->free_context = pos->next;
    } else {
        prev->next = pos->next;
    }
    dispose_context(pos);
}

static void
invoke_native_function(DVM_VirtualMachine *dvm,
                       Function *caller, Function *callee,
                       int pc, int *sp_p, int base)
{
    DVM_Value   *stack;
    DVM_Value   ret;
    int         arg_count;
    DVM_Context *context;
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
    context = DVM_push_context(dvm);

    call_info = (CallInfo*)&dvm->stack.stack[*sp_p];
    call_info->caller = caller;
    call_info->caller_address = pc;
    call_info->base = base;
    for (i = 0; i < CALL_INFO_ALIGN_SIZE; i++) {
        dvm->stack.pointer_flags[*sp_p+i] = DVM_FALSE;
    }
    *sp_p += CALL_INFO_ALIGN_SIZE;
    dvm->current_function = callee;
    ret = callee->u.native_f.proc(dvm, context,
                                  callee->u.native_f.arg_count,
                                  &stack[*sp_p - arg_count
                                         - CALL_INFO_ALIGN_SIZE]);
    dvm->current_function = caller;
    *sp_p -= arg_count + CALL_INFO_ALIGN_SIZE;
    stack[*sp_p] = ret;
    dvm->stack.pointer_flags[*sp_p] = callee->u.native_f.return_pointer;
    (*sp_p)++;
    DVM_pop_context(dvm, context);
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
                     .code_block.need_stack_size);

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

    *code_p = (*exe_p)->function[callee->u.diksam_f.index].code_block.code;
    *code_size_p
        = (*exe_p)->function[callee->u.diksam_f.index].code_block.code_size;
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
            *code_p = caller_p->code_block.code;
            *code_size_p = caller_p->code_block.code_size;
        }
    } else {
        *ee_p = dvm->top_level;
        *exe_p = dvm->top_level->executable;
        *code_p = dvm->top_level->executable->top_level.code;
        *code_size_p = dvm->top_level->executable->top_level.code_size;
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
    DVM_ObjectRef exception_dummy;

    size = STI(dvm, -dim);

    if (dim_index == type->derive_count-1) {
        switch (type->basic_type) {
        case DVM_VOID_TYPE:
            DBG_panic(("creating void array"));
            break;
        case DVM_BOOLEAN_TYPE: /* FALLTHRU */
        case DVM_INT_TYPE:
        case DVM_ENUM_TYPE:
            ret = dvm_create_array_int_i(dvm, size);
            break;
        case DVM_DOUBLE_TYPE:
            ret = dvm_create_array_double_i(dvm, size);
            break;
        case DVM_STRING_TYPE: /* FALLTHRU */
        case DVM_NATIVE_POINTER_TYPE:
        case DVM_CLASS_TYPE:
        case DVM_DELEGATE_TYPE:
            ret = dvm_create_array_object_i(dvm, size);
            break;
        case DVM_NULL_TYPE: /* FALLTHRU */
        case DVM_BASE_TYPE: /* FALLTHRU */
        case DVM_UNSPECIFIED_IDENTIFIER_TYPE: /* FALLTHRU */
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
                DVM_array_set_object(dvm, ret, i, child, &exception_dummy);
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

#define is_null_pointer(obj) (((obj)->data == NULL))

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

static DVM_ErrorStatus
check_down_cast(DVM_VirtualMachine *dvm, DVM_ObjectRef *obj, int target_idx,
                DVM_Boolean *is_same_class,
                DVM_Boolean *is_interface, int *interface_index)
{
    if (obj->v_table->exec_class->class_index == target_idx) {
        *is_same_class = DVM_TRUE;
        return DVM_SUCCESS;
    }
    *is_same_class = DVM_FALSE;

    if (!check_instanceof_i(dvm, obj, target_idx,
                            is_interface, interface_index)) {
        return DVM_ERROR;
    }

    return DVM_SUCCESS;
}

static void
reset_stack_pointer(DVM_Function *dvm_func, int *sp_p, int base)
{
    if (dvm_func) {
        *sp_p = base + dvm_func->parameter_count
            + (dvm_func->is_method ? 1 : 0)
            + CALL_INFO_ALIGN_SIZE
            + dvm_func->local_variable_count;
    } else {
        *sp_p = 0;
    }
}

/* This function returns DVM_TRUE, if exception happen in try or catch clause.
 */
static DVM_Boolean
throw_in_try(DVM_VirtualMachine *dvm,
             DVM_Executable *exe, ExecutableEntry *ee, Function *func,
             int *pc_p, int *sp_p, int base)
{
    DVM_CodeBlock *cb;
    int try_idx;
    int catch_idx;
    DVM_Boolean throw_in_try = DVM_FALSE;
    DVM_Boolean throw_in_catch = DVM_FALSE;
    int exception_idx;
    DVM_Function *dvm_func = NULL;

    if (func) {
        cb = &(func->u.diksam_f.executable->executable->function
               [func->u.diksam_f.index]).code_block;
        dvm_func = &(func->u.diksam_f.executable->executable->function
                     [func->u.diksam_f.index]);
    } else {
        cb = &exe->top_level;
    }

    exception_idx = dvm->current_exception.v_table->exec_class->class_index;

    for (try_idx = 0; try_idx < cb->try_size; try_idx++) {
        if ((*pc_p) >= cb->try[try_idx].try_start_pc
            && (*pc_p) <= cb->try[try_idx].try_end_pc) {
            throw_in_try = DVM_TRUE;
            break;
        }
        for (catch_idx = 0; catch_idx < cb->try[try_idx].catch_count;
             catch_idx++) {
            if ((*pc_p) >= cb->try[try_idx].catch_clause[catch_idx].start_pc
                && ((*pc_p)
                    <= cb->try[try_idx].catch_clause[catch_idx].end_pc)) {
                throw_in_catch = DVM_TRUE;
                break;
            }
        }
    }

    if (try_idx == cb->try_size)
        return DVM_FALSE;

    DBG_assert(throw_in_try || throw_in_catch, ("bad flags"));

    if (throw_in_try) {
        for (catch_idx = 0; catch_idx < cb->try[try_idx].catch_count;
             catch_idx++) {
            int class_idx_in_exe
                = cb->try[try_idx].catch_clause[catch_idx].class_index;
            int class_idx_in_dvm = ee->class_table[class_idx_in_exe];
            if (exception_idx == class_idx_in_dvm
                || check_instanceof(dvm, &dvm->current_exception,
                                    class_idx_in_dvm)) {
                *pc_p = cb->try[try_idx].catch_clause[catch_idx].start_pc;
                reset_stack_pointer(dvm_func, sp_p, base);
                STO_WRITE(dvm, 0, dvm->current_exception);
                dvm->stack.stack_pointer++;
                dvm->current_exception = dvm_null_object_ref;
                return DVM_TRUE;
            }
        }
    }
    *pc_p = cb->try[try_idx].finally_start_pc;
    reset_stack_pointer(dvm_func, sp_p, base);
    
    return DVM_TRUE;
}

static void
add_stack_trace(DVM_VirtualMachine *dvm, DVM_Executable *exe,
                Function *func, int pc)
{
    int line_number;
    int class_index;
    DVM_ObjectRef stack_trace;
    int stack_trace_index;
    DVM_Object *stack_trace_array;
    int array_size;
    int line_number_index;
    int file_name_index;
    DVM_Value value;
    DVM_Char *wc_str;
    char *func_name;
    int func_name_index;

    line_number = dvm_conv_pc_to_line_number(exe, func, pc);
    class_index = DVM_search_class(dvm,
                                   DVM_DIKSAM_DEFAULT_PACKAGE,
                                   DIKSAM_STACK_TRACE_CLASS);
    stack_trace = dvm_create_class_object_i(dvm, class_index);
    STO_WRITE(dvm, 0, stack_trace);
    dvm->stack.stack_pointer++;
    
    line_number_index
        = DVM_get_field_index(dvm, stack_trace, "line_number");
    stack_trace.data->u.class_object.field[line_number_index].int_value
        = line_number;

    file_name_index
        = DVM_get_field_index(dvm, stack_trace, "file_name");
    wc_str = dvm_mbstowcs_alloc(dvm, exe->path);
    stack_trace.data->u.class_object.field[file_name_index].object
        = dvm_create_dvm_string_i(dvm, wc_str);

    func_name_index
        = DVM_get_field_index(dvm, stack_trace, "function_name");
    if (func) {
        func_name = exe->function[func->u.diksam_f.index].name;
    } else {
        func_name = "top level";
    }
    wc_str = dvm_mbstowcs_alloc(dvm, func_name);
    stack_trace.data->u.class_object.field[func_name_index].object
        = dvm_create_dvm_string_i(dvm, wc_str);

    stack_trace_index
        = DVM_get_field_index(dvm, dvm->current_exception,
                              "stack_trace");
    stack_trace_array = dvm->current_exception.data
        ->u.class_object.field[stack_trace_index].object.data;
    array_size = DVM_array_size(dvm, stack_trace_array);
    value.object = stack_trace;
    DVM_array_insert(dvm, stack_trace_array, array_size, value);

    dvm->stack.stack_pointer--;
}

DVM_Value dvm_execute_i(DVM_VirtualMachine *dvm, Function *func,
                        DVM_Byte *code, int code_size, int base);

static DVM_Value
invoke_diksam_function_from_native(DVM_VirtualMachine *dvm,
                                   Function *callee, DVM_ObjectRef obj,
                                   DVM_Value *args)
{
    int base;
    int i;
    CallInfo *call_info;
    DVM_Executable *dvm_exe;
    DVM_Function *dvm_func;
    ExecutableEntry *current_executable_backup;
    Function *current_function_backup;
    int current_pc_backup;
    DVM_Byte *code;
    int code_size;
    DVM_Value ret;

    current_executable_backup = dvm->current_executable;
    current_function_backup = dvm->current_function;
    current_pc_backup = dvm->pc;

    dvm_exe = callee->u.diksam_f.executable->executable;
    dvm_func = &dvm_exe->function[callee->u.diksam_f.index];

    base = dvm->stack.stack_pointer;
    for (i = 0; i < dvm_func->parameter_count; i++) {
        dvm->stack.stack[dvm->stack.stack_pointer] = args[i];
        dvm->stack.pointer_flags[dvm->stack.stack_pointer]
            = is_pointer_type(dvm_func->parameter[i].type);
        dvm->stack.stack_pointer++;
    }
    if (!is_null_pointer(&obj)) {
        STO_WRITE(dvm, 0, obj);
        dvm->stack.stack_pointer++;
    }
    call_info = (CallInfo*)&dvm->stack.stack[dvm->stack.stack_pointer];
    call_info->caller = dvm->current_function;
    call_info->caller_address = CALL_FROM_NATIVE;
    call_info->base = 0; /* dummy */
    for (i = 0; i < CALL_INFO_ALIGN_SIZE; i++) {
        dvm->stack.pointer_flags[dvm->stack.stack_pointer + i] = DVM_FALSE;
        dvm->pc = 0;
        dvm->current_executable = callee->u.diksam_f.executable;
    }
    dvm->stack.stack_pointer += CALL_INFO_ALIGN_SIZE;
    initialize_local_variables(dvm, dvm_func, dvm->stack.stack_pointer);
    dvm->stack.stack_pointer += dvm_func->local_variable_count;
    code = dvm_exe->function[callee->u.diksam_f.index].code_block.code;
    code_size = dvm_exe->function[callee->u.diksam_f.index]
        .code_block.code_size;

    ret = dvm_execute_i(dvm, callee, code, code_size, base);
    dvm->stack.stack_pointer--;

    current_executable_backup = dvm->current_executable;
    current_function_backup = dvm->current_function;
    current_pc_backup = dvm->pc;

    return ret;
}

/* This function returns DVM_TRUE if this function was called from native.
 */
static DVM_Boolean
do_throw(DVM_VirtualMachine *dvm,
         Function **func_p, DVM_Byte **code_p, int *code_size_p, int *pc_p,
         int *base_p, ExecutableEntry **ee_p, DVM_Executable **exe_p,
         DVM_ObjectRef *exception)
{
    DVM_Boolean in_try;

    dvm->current_exception = *exception;

    for (;;) {
        in_try = throw_in_try(dvm, *exe_p, *ee_p, *func_p, pc_p,
                              &dvm->stack.stack_pointer, *base_p);
        if (in_try)
            break;

        if (*func_p) {
            add_stack_trace(dvm, *exe_p, *func_p, *pc_p);
            if (do_return(dvm, func_p, code_p, code_size_p, pc_p,
                          base_p, ee_p, exe_p)) {
                return DVM_TRUE;
            }
        } else {
            int func_index
                = dvm_search_function(dvm,
                                      DVM_DIKSAM_DEFAULT_PACKAGE,
                                      DIKSAM_PRINT_STACK_TRACE_FUNC);
            add_stack_trace(dvm, *exe_p, *func_p, *pc_p);

            invoke_diksam_function_from_native(dvm, dvm->function[func_index],
                                               dvm->current_exception, NULL);
            exit(1);
        }
    }
    return DVM_FALSE;
}

DVM_ObjectRef
dvm_create_exception(DVM_VirtualMachine *dvm, char *class_name,
                     RuntimeError id, ...)
{
    int class_index;
    DVM_ObjectRef obj;
    VString     message;
    va_list     ap;
    int message_index;
    int stack_trace_index;

    va_start(ap, id);
    class_index = DVM_search_class(dvm, DVM_DIKSAM_DEFAULT_PACKAGE,
                                   class_name);
    obj = dvm_create_class_object_i(dvm, class_index);

    STO_WRITE(dvm, 0, obj);
    dvm->stack.stack_pointer++;

    dvm_format_message(dvm_error_message_format, (int)id, &message, ap);
    va_end(ap);

    message_index
        = DVM_get_field_index(dvm, obj, "message");
    obj.data->u.class_object.field[message_index].object
        = dvm_create_dvm_string_i(dvm, message.string);

    stack_trace_index
        = DVM_get_field_index(dvm, obj, "stack_trace");
    obj.data->u.class_object.field[stack_trace_index].object
        = dvm_create_array_object_i(dvm, 0);

    dvm->stack.stack_pointer--;

    return obj;
}

/* This function returns DVM_TRUE if this function was called from native.
 */
static DVM_Boolean
throw_null_pointer_exception(DVM_VirtualMachine *dvm, Function **func_p,
                             DVM_Byte **code_p, int *code_size_p, int *pc_p,
                             int *base_p,
                             ExecutableEntry **ee_p, DVM_Executable **exe_p)
{
    DVM_ObjectRef ex;

    ex = dvm_create_exception(dvm, DVM_NULL_POINTER_EXCEPTION_NAME,
                              NULL_POINTER_ERR, DVM_MESSAGE_ARGUMENT_END);
    STO_WRITE(dvm, 0, ex); /* BUGBUG? irane? */
    dvm->stack.stack_pointer++;
    return do_throw(dvm, func_p, code_p, code_size_p, pc_p, base_p,
                    ee_p, exe_p, &ex);
}

static void
clear_stack_trace(DVM_VirtualMachine *dvm, DVM_ObjectRef *ex)
{
    int stack_trace_index
        = DVM_get_field_index(dvm, *ex, "stack_trace");

    ex->data->u.class_object.field[stack_trace_index].object
        = dvm_create_array_object_i(dvm, 0);
}

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
        case DVM_PUSH_CONSTANT_INT:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            STI_WRITE(dvm, 0,
                      dvm->constant[ee->constant_table[idx_in_exe]]
                      ->value.int_value);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case DVM_PUSH_CONSTANT_DOUBLE:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            STD_WRITE(dvm, 0,
                      dvm->constant[ee->constant_table[idx_in_exe]]
                      ->value.double_value);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case DVM_PUSH_CONSTANT_OBJECT:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            STO_WRITE(dvm, 0,
                      dvm->constant[ee->constant_table[idx_in_exe]]
                      ->value.object);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case DVM_POP_CONSTANT_INT:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            dvm->constant[ee->constant_table[idx_in_exe]]->value.int_value
                = STI(dvm, -1);
            dvm->stack.stack_pointer--;
            pc += 3;
            break;
        }
        case DVM_POP_CONSTANT_DOUBLE:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            dvm->constant[ee->constant_table[idx_in_exe]]->value.double_value
                = STD(dvm, -1);
            dvm->stack.stack_pointer--;
            pc += 3;
            break;
        }
        case DVM_POP_CONSTANT_OBJECT:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            dvm->constant[ee->constant_table[idx_in_exe]]->value.object
                = STO(dvm, -1);
            dvm->stack.stack_pointer--;
            pc += 3;
            break;
        }
        case DVM_PUSH_ARRAY_INT:
        {
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);
            int int_value;
            DVM_ErrorStatus status;
            DVM_ObjectRef exception;

            restore_pc(dvm, ee, func, pc);

            status = DVM_array_get_int(dvm, array, index,
                                       &int_value, &exception);
            if (status == DVM_SUCCESS) {
                STI_WRITE(dvm, -2, int_value);
                dvm->stack.stack_pointer--;
                pc++;
            } else {
                if (do_throw(dvm, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case DVM_PUSH_ARRAY_DOUBLE:
        {
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);
            double double_value;
            DVM_ErrorStatus status;
            DVM_ObjectRef exception;

            restore_pc(dvm, ee, func, pc);
            status = DVM_array_get_double(dvm, array, index,
                                          &double_value, &exception);
            if (status == DVM_SUCCESS) {
                STD_WRITE(dvm, -2, double_value);
                dvm->stack.stack_pointer--;
                pc++;
            } else {
                if (do_throw(dvm, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case DVM_PUSH_ARRAY_OBJECT:
        {
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);
            DVM_ObjectRef object;
            DVM_ErrorStatus status;
            DVM_ObjectRef exception;

            restore_pc(dvm, ee, func, pc);
            status = DVM_array_get_object(dvm, array, index,
                                          &object, &exception);
            if (status == DVM_SUCCESS) {
                STO_WRITE(dvm, -2, object);
                dvm->stack.stack_pointer--;
                pc++;
            } else {
                if (do_throw(dvm, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case DVM_POP_ARRAY_INT:
        {
            int value = STI(dvm, -3);
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);
            DVM_ErrorStatus status;
            DVM_ObjectRef exception;

            restore_pc(dvm, ee, func, pc);
            status = DVM_array_set_int(dvm, array, index, value, &exception);
            if (status == DVM_SUCCESS) {
                dvm->stack.stack_pointer -= 3;
                pc++;
            } else {
                if (do_throw(dvm, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case DVM_POP_ARRAY_DOUBLE:
        {
            double value = STD(dvm, -3);
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);
            DVM_ErrorStatus status;
            DVM_ObjectRef exception;

            restore_pc(dvm, ee, func, pc);
            status
                = DVM_array_set_double(dvm, array, index, value, &exception);
            if (status == DVM_SUCCESS) {
                dvm->stack.stack_pointer -= 3;
                pc++;
            } else {
                if (do_throw(dvm, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case DVM_POP_ARRAY_OBJECT:
        {
            DVM_ObjectRef value = STO(dvm, -3);
            DVM_ObjectRef array = STO(dvm, -2);
            int index = STI(dvm, -1);
            DVM_ErrorStatus status;
            DVM_ObjectRef exception;

            restore_pc(dvm, ee, func, pc);
            status
                = DVM_array_set_object(dvm, array, index, value, &exception);
            if (status == DVM_SUCCESS) {
                dvm->stack.stack_pointer -= 3;
                pc++;
            } else {
                if (do_throw(dvm, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case DVM_PUSH_CHARACTER_IN_STRING:
        {
            DVM_ObjectRef str = STO(dvm, -2);
            int index = STI(dvm, -1);
            DVM_ErrorStatus status;
            DVM_ObjectRef exception;
            DVM_Char ch;

            restore_pc(dvm, ee, func, pc);
            status = DVM_string_get_character(dvm, str, index,
                                              &ch, &exception);
            if (status == DVM_SUCCESS) {
                STI_WRITE(dvm, -2, ch);
                dvm->stack.stack_pointer--;
                pc++;
            } else {
                if (do_throw(dvm, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case DVM_PUSH_FIELD_INT:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(dvm, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                STI_WRITE(dvm, -1,
                          obj.data->u.class_object.field[index].int_value);
                pc += 3;
            }
            break;
        }
        case DVM_PUSH_FIELD_DOUBLE:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(dvm, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                STD_WRITE(dvm, -1,
                          obj.data->u.class_object.field[index].double_value);
                pc += 3;
            }
            break;
        }
        case DVM_PUSH_FIELD_OBJECT:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(dvm, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                STO_WRITE(dvm, -1,
                          obj.data->u.class_object.field[index].object);
                pc += 3;
            }
            break;
        }
        case DVM_POP_FIELD_INT:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(dvm, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                obj.data->u.class_object.field[index].int_value
                    = STI(dvm, -2);
                dvm->stack.stack_pointer -= 2;
                pc += 3;
            }
            break;
        }
        case DVM_POP_FIELD_DOUBLE:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(dvm, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                obj.data->u.class_object.field[index].double_value
                    = STD(dvm, -2);
                dvm->stack.stack_pointer -= 2;
                pc += 3;
            }
            break;
        }
        case DVM_POP_FIELD_OBJECT:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(dvm, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                obj.data->u.class_object.field[index].object = STO(dvm, -2);
                dvm->stack.stack_pointer -= 2;
                pc += 3;
            }
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
                DVM_ObjectRef exception;

                exception
                    = dvm_create_exception(dvm,
                                           DIVISION_BY_ZERO_EXCEPTION_NAME,
                                           DIVISION_BY_ZERO_ERR,
                                           DVM_MESSAGE_ARGUMENT_END);
                if (do_throw(dvm, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            } else {
                STI(dvm, -2) = STI(dvm, -2) / STI(dvm, -1);
                dvm->stack.stack_pointer--;
                pc++;
            }
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
        case DVM_BIT_AND:
            STI(dvm, -2) = STI(dvm, -2) & STI(dvm, -1);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_BIT_OR:
            STI(dvm, -2) = STI(dvm, -2) | STI(dvm, -1);
            dvm->stack.stack_pointer--;
            pc++;
            break;
        case DVM_BIT_XOR:
            STI(dvm, -2) = STI(dvm, -2) ^ STI(dvm, -1);
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
        case DVM_BIT_NOT:
            STI(dvm, -1) = ~STI(dvm, -1);
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
                      dvm_create_dvm_string_i(dvm, wc_str));
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
                      dvm_create_dvm_string_i(dvm, wc_str));
            pc++;
            break;
        }
        case DVM_CAST_ENUM_TO_STRING:
        {
            DVM_Char *wc_str;
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            int enum_index = ee->enum_table[idx_in_exe];
            int enum_value = STI(dvm, -1);

            restore_pc(dvm, ee, func, pc);
            wc_str = dvm_mbstowcs_alloc(dvm,
                                        dvm->enums[enum_index]->dvm_enum
                                        ->enumerator[enum_value]);
            STO_WRITE(dvm, -1,
                      dvm_create_dvm_string_i(dvm, wc_str));
            pc += 3;
            break;
        }
        case DVM_UP_CAST:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(dvm, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                obj.v_table
                    = obj.v_table->exec_class->interface_v_table[index];
                STO_WRITE(dvm, -1, obj);
                pc += 3;
            }
            break;
        }
        case DVM_DOWN_CAST:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            int index = ee->class_table[idx_in_exe];
            DVM_Boolean is_same_class;
            DVM_Boolean is_interface;
            int interface_idx;
            DVM_ErrorStatus status;
            DVM_ObjectRef exception;

            do {
                if (is_null_pointer(&obj)) {
                    if (throw_null_pointer_exception(dvm, &func,
                                                     &code, &code_size,
                                                     &pc, &base, &ee, &exe)) {
                        goto EXECUTE_END;
                    }
                    break;
                }
                status = check_down_cast(dvm, &obj, index,
                                         &is_same_class,
                                         &is_interface, &interface_idx);
                if (status != DVM_SUCCESS) {
                    exception
                        = dvm_create_exception(dvm,
                                               CLASS_CAST_EXCEPTION_NAME,
                                               CLASS_CAST_ERR,
                                               DVM_STRING_MESSAGE_ARGUMENT,
                                               "org",
                                               obj.v_table->exec_class->name,
                                               DVM_STRING_MESSAGE_ARGUMENT,
                                               "target",
                                               dvm->class[index]->name,
                                               DVM_MESSAGE_ARGUMENT_END);
                    if (do_throw(dvm, &func, &code, &code_size, &pc,
                                 &base, &ee, &exe, &exception)) {
                        goto EXECUTE_END;
                    }
                    break;
                }
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
            } while (0);
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
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            STI_WRITE(dvm, 0, ee->function_table[idx_in_exe]);
            dvm->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case DVM_PUSH_METHOD:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(dvm, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                STI_WRITE(dvm, 0, obj.v_table->table[index].index);
                dvm->stack.stack_pointer++;
                pc += 3;
            }
            break;
        }
        case DVM_PUSH_DELEGATE:
        {
            int index = GET_2BYTE_INT(&code[pc+1]);
            int dvm_index;
            DVM_ObjectRef delegate;

            dvm_index = ee->function_table[index];
            delegate = dvm_create_delegate(dvm, dvm_null_object_ref,
                                           dvm_index);
            STO_WRITE(dvm, 0, delegate);
            dvm->stack.stack_pointer++;
            pc += 3;

            break;
        }
        case DVM_PUSH_METHOD_DELEGATE:
        {
            DVM_ObjectRef obj = STO(dvm, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);
            DVM_ObjectRef delegate;

            delegate = dvm_create_delegate(dvm, obj, index);
            STO_WRITE(dvm, -1, delegate);
            pc += 3;

            break;
        }
        case DVM_INVOKE: /* FALLTHRU */
        case DVM_INVOKE_DELEGATE:
        {
            int func_idx;

            if ((DVM_Opcode)code[pc] == DVM_INVOKE_DELEGATE) {
                DVM_ObjectRef delegate = STO(dvm, -1);

                if (is_null_pointer(&delegate)) {
                    if (throw_null_pointer_exception(dvm, &func,
                                                     &code, &code_size,
                                                     &pc, &base, &ee, &exe)) {
                        goto EXECUTE_END;
                    }
                }
                if (is_null_pointer(&delegate.data->u.delegate.object)) {
                    func_idx = delegate.data->u.delegate.index;
                } else {
                    func_idx
                        = (delegate.data->u.delegate.object.v_table
                           ->table[delegate.data->u.delegate.index].index);
                    STO_WRITE(dvm, -1, delegate.data->u.delegate.object);
                    dvm->stack.stack_pointer++; /* for func index */
                }
            } else {
                func_idx = STI(dvm, -1);
            }
            if (dvm->function[func_idx]->kind == NATIVE_FUNCTION) {
                restore_pc(dvm, ee, func, pc);
                dvm->current_exception = dvm_null_object_ref;
                invoke_native_function(dvm, func, dvm->function[func_idx],
                                       pc, &dvm->stack.stack_pointer, base);
                if (!is_object_null(dvm->current_exception)) {
                    if (do_throw(dvm, &func, &code, &code_size, &pc,
                                 &base, &ee, &exe, &dvm->current_exception)) {
                        goto EXECUTE_END;
                    }
                } else {
                    pc++;
                }
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
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            int class_index = ee->class_table[idx_in_exe];
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
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            int target_idx = ee->class_table[idx_in_exe];

            if (obj->v_table->exec_class->class_index == target_idx) {
                STI_WRITE(dvm, -1, DVM_TRUE);
            } else {
                STI_WRITE(dvm, -1, check_instanceof(dvm, obj, target_idx));
            }
            pc += 3;
            break;
        }
        case DVM_THROW:
        {
            DVM_ObjectRef* exception = &STO(dvm, -1);

            clear_stack_trace(dvm, exception);
            if (do_throw(dvm, &func, &code, &code_size, &pc,
                         &base, &ee, &exe, exception)) {
                goto EXECUTE_END;
            }
            break;
        }
        case DVM_RETHROW:
        {
            DVM_ObjectRef* exception = &STO(dvm, -1);

            if (do_throw(dvm, &func, &code, &code_size, &pc,
                         &base, &ee, &exe, exception)) {
                goto EXECUTE_END;
            }
            break;
        }
        case DVM_GO_FINALLY:
            STI_WRITE(dvm, 0, pc);
            dvm->stack.stack_pointer++;
            pc = GET_2BYTE_INT(&code[pc+1]);
            break;
        case DVM_FINALLY_END:
            if (!is_object_null(dvm->current_exception)) {
                if (do_throw(dvm, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &dvm->current_exception)) {
                    goto EXECUTE_END;
                }
            } else {
                pc = STI(dvm, -1) + 3;
                dvm->stack.stack_pointer--;
            }
            break;
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
                     dvm->top_level->executable->top_level.need_stack_size);
    dvm_execute_i(dvm, NULL, dvm->top_level->executable->top_level.code,
                  dvm->top_level->executable->top_level.code_size, 0);

    return ret;
}


DVM_Value
DVM_invoke_delegate(DVM_VirtualMachine *dvm, DVM_Value delegate,
                    DVM_Value *args)
{
    DVM_Value ret;
    int func_idx;
    Function *callee;
    DVM_ObjectRef del_obj = delegate.object;

    if (is_null_pointer(&del_obj.data->u.delegate.object)) {
        func_idx = del_obj.data->u.delegate.index;
    } else {
        func_idx
            = (del_obj.data->u.delegate.object.v_table
               ->table[del_obj.data->u.delegate.index].index);
    }
    callee = dvm->function[func_idx];

    if (callee->kind == DIKSAM_FUNCTION) {
        invoke_diksam_function_from_native(dvm, callee,
                                           del_obj.data->u.delegate.object,
                                           args);
    } else {
        DBG_assert((callee->kind == NATIVE_FUNCTION),
                   ("kind..%d", callee->kind));
    }

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

        MEM_free(ee_temp->function_table);
        MEM_free(ee_temp->class_table);
        MEM_free(ee_temp->enum_table);
        MEM_free(ee_temp->constant_table);
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

    for (i = 0; i < dvm->enum_count; i++) {
        MEM_free(dvm->enums[i]->name);
        MEM_free(dvm->enums[i]->package_name);
        MEM_free(dvm->enums[i]);
    }
    MEM_free(dvm->enums);

    for (i = 0; i < dvm->constant_count; i++) {
        MEM_free(dvm->constant[i]->name);
        MEM_free(dvm->constant[i]->package_name);
        MEM_free(dvm->constant[i]);
    }
    MEM_free(dvm->constant);

    MEM_free(dvm->array_v_table->table);
    MEM_free(dvm->array_v_table);
    MEM_free(dvm->string_v_table->table);
    MEM_free(dvm->string_v_table);
    MEM_free(dvm->class);

    MEM_free(dvm);
}
