#include <stdio.h>
#include <string.h>
#include "DKC.h"
#include "MEM.h"
#include "DBG.h"
#include "dvm_pri.h"

static void
implement_diksam_function(DVM_VirtualMachine *dvm, int dest_idx,
                          ExecutableEntry *ee, int src_idx)
{
    dvm->function[dest_idx]->u.diksam_f.executable
        = ee;
    dvm->function[dest_idx]->u.diksam_f.index = src_idx;
}

static void
add_functions(DVM_VirtualMachine *dvm, ExecutableEntry *ee)
{
    int src_idx;
    int dest_idx;
    int add_func_count = 0;
    DVM_Boolean *new_func_flags;

    new_func_flags = MEM_malloc(sizeof(DVM_Boolean)
                                * ee->executable->function_count);

    for (src_idx = 0; src_idx < ee->executable->function_count; src_idx++) {
        for (dest_idx = 0; dest_idx < dvm->function_count; dest_idx++) {
            if (!strcmp(dvm->function[dest_idx]->name,
                        ee->executable->function[src_idx].name)
                && dvm_compare_package_name(dvm->function[dest_idx]
                                            ->package_name,
                                            ee->executable->function
                                            [src_idx].package_name)) {
                if (ee->executable->function[src_idx].is_implemented
                    && dvm->function[dest_idx]->is_implemented) {
                    char *package_name;

                    if (dvm->function[dest_idx]->package_name) {
                        package_name = dvm->function[dest_idx]->package_name;
                    } else {
                        package_name = "";
                    }
                    dvm_error_i(NULL, NULL, NO_LINE_NUMBER_PC,
                                FUNCTION_MULTIPLE_DEFINE_ERR,
                                DVM_STRING_MESSAGE_ARGUMENT, "package",
                                package_name,
                                DVM_STRING_MESSAGE_ARGUMENT, "name",
                                dvm->function[dest_idx]->name,
                                DVM_MESSAGE_ARGUMENT_END);
                }
                new_func_flags[src_idx] = DVM_FALSE;
                if (ee->executable->function[src_idx].is_implemented) {
                    implement_diksam_function(dvm, dest_idx, ee, src_idx);
                }
                break;
            }
        }
        if (dest_idx == dvm->function_count) {
            new_func_flags[src_idx] = DVM_TRUE;
            add_func_count++;
        }
    }
    dvm->function
        = MEM_realloc(dvm->function,
                      sizeof(Function*)
                      * (dvm->function_count + add_func_count));

    for (src_idx = 0, dest_idx = dvm->function_count;
         src_idx < ee->executable->function_count; src_idx++) {
        if (!new_func_flags[src_idx])
            continue;

        dvm->function[dest_idx] = MEM_malloc(sizeof(Function));
        if (ee->executable->function[src_idx].package_name) {
            dvm->function[dest_idx]->package_name
                = MEM_strdup(ee->executable->function[src_idx].package_name);
        } else {
            dvm->function[dest_idx]->package_name = NULL;
        }
        dvm->function[dest_idx]->name
            = MEM_strdup(ee->executable->function[src_idx].name);
        dvm->function[dest_idx]->kind = DIKSAM_FUNCTION;
        dvm->function[dest_idx]->is_implemented
            = ee->executable->function[src_idx].is_implemented;
        if (ee->executable->function[src_idx].is_implemented) {
            implement_diksam_function(dvm, dest_idx, ee, src_idx);
        }
        dest_idx++;
    }
    dvm->function_count += add_func_count;
    MEM_free(new_func_flags);
}

static int
search_function(DVM_VirtualMachine *dvm, char *package_name, char *name)
{
    int i;

    for (i = 0; i < dvm->function_count; i++) {
        if (dvm_compare_package_name(dvm->function[i]->package_name,
                                     package_name)
            && !strcmp(dvm->function[i]->name, name)) {
            return i;
        }
    }
    return FUNCTION_NOT_FOUND;
}

static int
search_class(DVM_VirtualMachine *dvm, char *package_name, char *name)
{
    int i;

    for (i = 0; i < dvm->class_count; i++) {
        if (dvm_compare_package_name(dvm->class[i]->package_name,
                                     package_name)
            && !strcmp(dvm->class[i]->name, name)) {
            return i;
        }
    }
    dvm_error_i(NULL, NULL, NO_LINE_NUMBER_PC, CLASS_NOT_FOUND_ERR,
                DVM_STRING_MESSAGE_ARGUMENT, "name", name,
                DVM_MESSAGE_ARGUMENT_END);
    return 0; /* make compiler happy */
}

static void
convert_code(DVM_VirtualMachine *dvm, DVM_Executable *exe,
             DVM_Byte *code, int code_size, DVM_Function *func)
{
    int i;
    int j;
    OpcodeInfo *info;
    int src_idx;
    unsigned int dest_idx;

    for (i = 0; i < code_size; i++) {
        if (code[i] == DVM_PUSH_STACK_INT
            || code[i] == DVM_PUSH_STACK_DOUBLE
            || code[i] == DVM_PUSH_STACK_OBJECT
            || code[i] == DVM_POP_STACK_INT
            || code[i] == DVM_POP_STACK_DOUBLE
            || code[i] == DVM_POP_STACK_OBJECT) {
            int parameter_count;

            DBG_assert(func != NULL, ("func == NULL!\n"));
            
            if (func->is_method) {
                parameter_count = func->parameter_count + 1; /* for this */
            } else{
                parameter_count = func->parameter_count;
            }

            src_idx = GET_2BYTE_INT(&code[i+1]);
            if (src_idx >= parameter_count) {
                dest_idx = src_idx + CALL_INFO_ALIGN_SIZE;
            } else {
                dest_idx = src_idx;
            }
            SET_2BYTE_INT(&code[i+1], dest_idx);

        } else if (code[i] == DVM_PUSH_FUNCTION) {
            int idx_in_exe;
            unsigned int func_idx;

            idx_in_exe = GET_2BYTE_INT(&code[i+1]);
            func_idx = search_function(dvm,
                                       exe->function[idx_in_exe].package_name,
                                       exe->function[idx_in_exe].name);
            SET_2BYTE_INT(&code[i+1], func_idx);

        } else if (code[i] == DVM_NEW || code[i] == DVM_INSTANCEOF
                   || code[i] == DVM_DOWN_CAST) {
            int idx_in_exe;
            unsigned int class_idx;

            idx_in_exe = GET_2BYTE_INT(&code[i+1]);
            class_idx = search_class(dvm,
                                     exe->class_definition[idx_in_exe]
                                     .package_name,
                                     exe->class_definition[idx_in_exe].name);
            SET_2BYTE_INT(&code[i+1], class_idx);
        }
        info = &dvm_opcode_info[code[i]];
        for (j = 0; info->parameter[j] != '\0'; j++) {
            switch (info->parameter[j]) {
            case 'b':
                i++;
                break;
            case 's': /* FALLTHRU */
            case 'p':
                i += 2;
                break;
            default:
                DBG_assert(0, ("param..%s, j..%d", info->parameter, j));
            }
        }
    }
}


static void
add_static_variables(ExecutableEntry *entry, DVM_Executable *exe)
{
    int i;

    entry->static_v.variable
        = MEM_malloc(sizeof(DVM_Value) * exe->global_variable_count);
    entry->static_v.variable_count = exe->global_variable_count;

    for (i = 0; i < exe->global_variable_count; i++) {
        if (exe->global_variable[i].type->basic_type == DVM_STRING_TYPE) {
            entry->static_v.variable[i].object = dvm_null_object_ref;
        }
    }
    for (i = 0; i < exe->global_variable_count; i++) {
        dvm_initialize_value(exe->global_variable[i].type,
                             &entry->static_v.variable[i]);
    }
}

static DVM_Class *
search_class_from_executable(DVM_Executable *exe, char *name)
{
    int i;

    for (i = 0; i < exe->class_count; i++) {
        if (!strcmp(exe->class_definition[i].name, name)) {
            return &exe->class_definition[i];
        }
    }
    DBG_panic(("class %s not found.", name));

    return NULL; /* make compiler happy */
}

static int
set_field_types(DVM_Executable *exe, DVM_Class *pos,
                DVM_TypeSpecifier **field_type, int index)
{
    DVM_Class *next;
    int i;

    if (pos->super_class) {
        next = search_class_from_executable(exe, pos->super_class->name);
        index = set_field_types(exe, next, field_type, index);
    }
    for (i = 0; i < pos->field_count; i++) {
        field_type[index] = pos->field[i].type;
        index++;
    }

    return index;
}

static void
add_fields(DVM_Executable *exe, DVM_Class *src, ExecClass *dest)
{
    int field_count = 0;
    DVM_Class *pos;

    for (pos = src; ; ) {
        field_count += pos->field_count;
        if (pos->super_class == NULL)
            break;
        pos = search_class_from_executable(exe, pos->super_class->name);
    }
    dest->field_count = field_count;
    dest->field_type = MEM_malloc(sizeof(DVM_TypeSpecifier*) * field_count);
    set_field_types(exe, src, dest->field_type, 0);
}

static void
set_v_table(DVM_VirtualMachine *dvm, DVM_Class *class_p,
            DVM_Method *src, VTableItem *dest, DVM_Boolean set_name)
{
    char *func_name;
    int  func_idx;

    if (set_name) {
        dest->name = MEM_strdup(src->name);
    }
    func_name = dvm_create_method_function_name(class_p->name, src->name);
    func_idx = search_function(dvm, class_p->package_name, func_name);

    if (func_idx == FUNCTION_NOT_FOUND && !src->is_abstract) {
        dvm_error_i(NULL, NULL, NO_LINE_NUMBER_PC, FUNCTION_NOT_FOUND_ERR,
                    DVM_STRING_MESSAGE_ARGUMENT, "name", func_name,
                    DVM_MESSAGE_ARGUMENT_END);
    }
    MEM_free(func_name);
    dest->index = func_idx;
}

static int
add_method(DVM_VirtualMachine *dvm, DVM_Executable *exe, DVM_Class *pos,
           DVM_VTable *v_table)
{
    DVM_Class   *next;
    int         i;
    int         j;
    int         super_method_count = 0;
    int         method_count;

    if (pos->super_class) {
        next = search_class_from_executable(exe, pos->super_class->name);
        super_method_count = add_method(dvm, exe, next, v_table);
    }

    method_count = super_method_count;
    for (i = 0; i < pos->method_count; i++) {
        for (j = 0; j < super_method_count; j++) {
            if (!strcmp(pos->method[i].name, v_table->table[j].name)) {
                set_v_table(dvm, pos, &pos->method[i], &v_table->table[j],
                            DVM_FALSE);
                break;
            }
        }
        /* if pos->method[i].is_override == true,
         * this method implements interface method.
         */
        if (j == super_method_count && !pos->method[i].is_override) {
            v_table->table
                = MEM_realloc(v_table->table,
                              sizeof(VTableItem) * (method_count + 1));
            set_v_table(dvm, pos, &pos->method[i],
                        &v_table->table[method_count], DVM_TRUE);
            method_count++;
            v_table->table_size = method_count;
        }
    }

    return method_count;
}

static DVM_VTable *
alloc_v_table(ExecClass *exec_class)
{
    DVM_VTable *v_table;

    v_table = MEM_malloc(sizeof(DVM_VTable));
    v_table->exec_class = exec_class;
    v_table->table = NULL;

    return v_table;
}

static void
add_methods(DVM_VirtualMachine *dvm, DVM_Executable *exe,
            DVM_Class *src, ExecClass *dest)
{
    int         method_count;
    DVM_VTable  *v_table;
    int         i;
    DVM_Class   *interface;
    int         method_idx;

    v_table = alloc_v_table(dest);
    method_count = add_method(dvm, exe, src, v_table);
    dest->class_table = v_table;
    dest->interface_count = src->interface_count;
    dest->interface_v_table
        = MEM_malloc(sizeof(DVM_VTable*) * src->interface_count);

    for (i = 0; i < src->interface_count; i++) {
        dest->interface_v_table[i] = alloc_v_table(dest);
        interface = search_class_from_executable(exe,
                                                 src->interface_[i].name);
        dest->interface_v_table[i]->table
            = MEM_malloc(sizeof(VTableItem) * interface->method_count);
        dest->interface_v_table[i]->table_size = interface->method_count;
        for (method_idx = 0; method_idx < interface->method_count;
             method_idx++) {
            set_v_table(dvm, src, &interface->method[method_idx],
                        &dest->interface_v_table[i]->table[method_idx],
                        DVM_TRUE);
        }
    }
}

static void
add_class(DVM_VirtualMachine *dvm, DVM_Executable *exe,
          DVM_Class *src, ExecClass *dest)
{
    
    add_fields(exe, src, dest);
    add_methods(dvm, exe, src, dest);
}

static void
set_super_class(DVM_VirtualMachine *dvm, DVM_Executable *exe,
                int old_class_count)
{
    int class_idx;
    DVM_Class *dvm_class;
    int super_class_index;
    int if_idx;
    int interface_index;

    for (class_idx = old_class_count; class_idx < dvm->class_count;
         class_idx++) {
        dvm_class = search_class_from_executable(exe,
                                                 dvm->class[class_idx]->name);
        if (dvm_class->super_class == NULL) {
            dvm->class[class_idx]->super_class = NULL;
        } else {
            super_class_index
                = search_class(dvm,
                               dvm_class->super_class->package_name,
                               dvm_class->super_class->name);
            dvm->class[class_idx]->super_class = dvm->class[super_class_index];
        }
        dvm->class[class_idx]->interface
            = MEM_malloc(sizeof(ExecClass*) * dvm_class->interface_count);
        for (if_idx = 0; if_idx < dvm_class->interface_count; if_idx++) {
            interface_index
                = search_class(dvm,
                               dvm_class->interface_[if_idx].package_name,
                               dvm_class->interface_[if_idx].name);
            dvm->class[class_idx]->interface[if_idx]
                = dvm->class[interface_index];
        }
    }
}

static void
add_classes(DVM_VirtualMachine *dvm, ExecutableEntry *ee)
{
    int src_idx;
    int dest_idx;
    int add_class_count = 0;
    DVM_Boolean *new_class_flags;
    int old_class_count;
    DVM_Executable *exe = ee->executable;

    new_class_flags = MEM_malloc(sizeof(DVM_Boolean)
                                 * exe->class_count);

    for (src_idx = 0; src_idx < exe->class_count; src_idx++) {
        for (dest_idx = 0; dest_idx < dvm->class_count; dest_idx++) {
            if (!strcmp(dvm->class[dest_idx]->name,
                        exe->class_definition[src_idx].name)
                && dvm_compare_package_name(dvm->class[dest_idx]
                                            ->package_name,
                                            exe->class_definition[src_idx]
                                            .package_name)) {
                if (exe->class_definition[src_idx].is_implemented
                    && dvm->class[dest_idx]->is_implemented) {
                    char *package_name;

                    if (dvm->class[dest_idx]->package_name) {
                        package_name = dvm->class[dest_idx]->package_name;
                    } else {
                        package_name = "";
                    }
                    dvm_error_i(NULL, NULL, NO_LINE_NUMBER_PC,
                                CLASS_MULTIPLE_DEFINE_ERR,
                                DVM_STRING_MESSAGE_ARGUMENT, "package",
                                package_name,
                                DVM_STRING_MESSAGE_ARGUMENT, "name",
                                dvm->class[dest_idx]->name,
                                DVM_MESSAGE_ARGUMENT_END);
                }
                new_class_flags[src_idx] = DVM_FALSE;
                if (exe->class_definition[src_idx].is_implemented) {
                    add_class(dvm, exe, &exe->class_definition[src_idx],
                              dvm->class[dest_idx]);
                }
                break;
            }
        }
        if (dest_idx == dvm->class_count) {
            new_class_flags[src_idx] = DVM_TRUE;
            add_class_count++;
        }
    }
    dvm->class
        = MEM_realloc(dvm->class, sizeof(ExecClass*)
                      * (dvm->class_count + add_class_count));

    for (src_idx = 0, dest_idx = dvm->class_count;
         src_idx < exe->class_count; src_idx++) {
        if (!new_class_flags[src_idx])
            continue;

        dvm->class[dest_idx] = MEM_malloc(sizeof(ExecClass));
        dvm->class[dest_idx]->dvm_class = &exe->class_definition[src_idx];
        dvm->class[dest_idx]->executable = ee;
        if (exe->class_definition[src_idx].package_name) {
            dvm->class[dest_idx]->package_name
                = MEM_strdup(exe->class_definition[src_idx].package_name);
        } else {
            dvm->class[dest_idx]->package_name = NULL;
        }
        dvm->class[dest_idx]->name
            = MEM_strdup(exe->class_definition[src_idx].name);
        dvm->class[dest_idx]->is_implemented
            = exe->class_definition[src_idx].is_implemented;
        dvm->class[dest_idx]->class_index = dest_idx;
        if (exe->class_definition[src_idx].is_implemented) {
            add_class(dvm, exe, &exe->class_definition[src_idx],
                      dvm->class[dest_idx]);
        }
        dest_idx++;
    }
    old_class_count = dvm->class_count;
    dvm->class_count += add_class_count;

    set_super_class(dvm, exe, old_class_count);

    MEM_free(new_class_flags);
}

static ExecutableEntry *
add_executable_to_dvm(DVM_VirtualMachine *dvm, DVM_Executable *executable,
                      DVM_Boolean is_top_level)
{
    int i;
    ExecutableEntry *ee_pos;
    ExecutableEntry *new_entry;
    
    new_entry = MEM_malloc(sizeof(ExecutableEntry));
    new_entry->executable = executable;
    new_entry->next = NULL;

    if (dvm->executable_entry == NULL) {
        dvm->executable_entry = new_entry;
    } else {
        for (ee_pos = dvm->executable_entry; ee_pos->next;
             ee_pos = ee_pos->next)
            ;
        ee_pos->next = new_entry;
    }
    
    add_functions(dvm, new_entry);
    add_classes(dvm, new_entry);

    convert_code(dvm, executable,
                 executable->code, executable->code_size,
                 NULL);

    for (i = 0; i < executable->function_count; i++) {
        if (executable->function[i].is_implemented) {
            convert_code(dvm, executable,
                         executable->function[i].code,
                         executable->function[i].code_size,
                         &executable->function[i]);
        }
    }
    add_static_variables(new_entry, executable);

    if (is_top_level) {
        dvm->top_level = new_entry;
    }
    return new_entry;
}

void
DVM_set_executable(DVM_VirtualMachine *dvm, DVM_ExecutableList *list)
{
    DVM_ExecutableItem *pos;
    int old_class_count;
    ExecutableEntry *ee;

    dvm->executable_list = list;

    old_class_count = dvm->class_count;
    for (pos = list->list; pos; pos = pos->next) {

        ee = add_executable_to_dvm(dvm, pos->executable,
                                   (pos->executable == list->top_level));
    }
}

static VTableItem st_array_method_v_table[] = {
    {ARRAY_PREFIX ARRAY_METHOD_SIZE, FUNCTION_NOT_FOUND},
    {ARRAY_PREFIX ARRAY_METHOD_RESIZE, FUNCTION_NOT_FOUND},
    {ARRAY_PREFIX ARRAY_METHOD_INSERT, FUNCTION_NOT_FOUND},
    {ARRAY_PREFIX ARRAY_METHOD_REMOVE, FUNCTION_NOT_FOUND},
    {ARRAY_PREFIX ARRAY_METHOD_ADD, FUNCTION_NOT_FOUND}
};

static VTableItem st_string_method_v_table[] = {
    {STRING_PREFIX STRING_METHOD_LENGTH, FUNCTION_NOT_FOUND},
    {STRING_PREFIX STRING_METHOD_SUBSTR, FUNCTION_NOT_FOUND},
};

static void
set_built_in_methods(DVM_VirtualMachine *dvm)
{
    DVM_VTable *array_v_table;
    DVM_VTable *string_v_table;
    int i;

    array_v_table = alloc_v_table(NULL);
    array_v_table->table_size = ARRAY_SIZE(st_array_method_v_table);
    array_v_table->table = MEM_malloc(sizeof(VTableItem)
                                      * array_v_table->table_size);
    for (i = 0; i < array_v_table->table_size; i++) {
        array_v_table->table[i] = st_array_method_v_table[i];
        array_v_table->table[i].index
            = search_function(dvm, BUILT_IN_METHOD_PACKAGE_NAME,
                              array_v_table->table[i].name);
    }
    dvm->array_v_table = array_v_table;

    string_v_table = alloc_v_table(NULL);
    string_v_table->table_size = ARRAY_SIZE(st_string_method_v_table);
    string_v_table->table = MEM_malloc(sizeof(VTableItem)
                                      * string_v_table->table_size);
    for (i = 0; i < string_v_table->table_size; i++) {
        string_v_table->table[i] = st_string_method_v_table[i];
        string_v_table->table[i].index
            = search_function(dvm, BUILT_IN_METHOD_PACKAGE_NAME,
                              string_v_table->table[i].name);
    }
    dvm->string_v_table = string_v_table;
}

DVM_VirtualMachine *
DVM_create_virtual_machine(void)
{
    DVM_VirtualMachine *dvm;

    dvm = MEM_malloc(sizeof(DVM_VirtualMachine));
    dvm->stack.alloc_size = STACK_ALLOC_SIZE;
    dvm->stack.stack = MEM_malloc(sizeof(DVM_Value) * STACK_ALLOC_SIZE);
    dvm->stack.pointer_flags
        = MEM_malloc(sizeof(DVM_Boolean) * STACK_ALLOC_SIZE);
    dvm->stack.stack_pointer = 0;
    dvm->heap.current_heap_size = 0;
    dvm->heap.header = NULL;
    dvm->heap.current_threshold = HEAP_THRESHOLD_SIZE;
    dvm->current_executable = NULL;
    dvm->current_function = NULL;
    dvm->current_exception = dvm_null_object_ref;
    dvm->function = NULL;
    dvm->function_count = 0;
    dvm->class = NULL;
    dvm->class_count = 0;
    dvm->executable_list = NULL;
    dvm->executable_entry = NULL;
    dvm->top_level = NULL;

    dvm_add_native_functions(dvm);

    set_built_in_methods(dvm);

    return dvm;
}

void
DVM_add_native_function(DVM_VirtualMachine *dvm,
                        char *package_name, char *func_name,
                        DVM_NativeFunctionProc *proc, int arg_count,
                        DVM_Boolean is_method, DVM_Boolean return_pointer)
{
    dvm->function
        = MEM_realloc(dvm->function,
                      sizeof(Function*) * (dvm->function_count + 1));

    dvm->function[dvm->function_count] = MEM_malloc(sizeof(Function));
    dvm->function[dvm->function_count]->package_name
        = MEM_strdup(package_name);
    dvm->function[dvm->function_count]->name = MEM_strdup(func_name);
    dvm->function[dvm->function_count]->kind = NATIVE_FUNCTION;
    dvm->function[dvm->function_count]->is_implemented = DVM_TRUE;
    dvm->function[dvm->function_count]->u.native_f.proc = proc;
    dvm->function[dvm->function_count]->u.native_f.arg_count = arg_count;
    dvm->function[dvm->function_count]->u.native_f.is_method = is_method;
    dvm->function[dvm->function_count]->u.native_f.return_pointer
        = return_pointer;
    dvm->function_count++;
}

void
dvm_dynamic_load(DVM_VirtualMachine *dvm,
                 DVM_Executable *caller_exe, Function *caller, int pc,
                 Function *func)
{
    DKC_Compiler *compiler;
    DVM_ExecutableItem *pos;
    DVM_ExecutableItem *add_top;
    SearchFileStatus status;
    char search_file[FILENAME_MAX];

    compiler = DKC_create_compiler();
    if (func->package_name == NULL) {
        dvm_error_i(caller_exe, caller, pc,
                    DYNAMIC_LOAD_WITHOUT_PACKAGE_ERR,
                    DVM_STRING_MESSAGE_ARGUMENT, "name", func->name,
                    DVM_MESSAGE_ARGUMENT_END);
    }
    status = dkc_dynamic_compile(compiler, func->package_name,
                                 dvm->executable_list, &add_top,
                                 search_file);
    if (status != SEARCH_FILE_SUCCESS) {
        if (status == SEARCH_FILE_NOT_FOUND) {
            dvm_error_i(caller_exe, caller, pc,
                        LOAD_FILE_NOT_FOUND_ERR,
                        DVM_STRING_MESSAGE_ARGUMENT, "file", search_file,
                        DVM_MESSAGE_ARGUMENT_END);
        } else {
            dvm_error_i(caller_exe, caller, pc,
                        LOAD_FILE_ERR,
                        DVM_INT_MESSAGE_ARGUMENT, "status", (int)status,
                        DVM_MESSAGE_ARGUMENT_END);
        }
    }

    for (pos = add_top; pos; pos = pos->next) {
        ExecutableEntry *ee;

        ee = add_executable_to_dvm(dvm, pos->executable, DVM_FALSE);
    }
    DKC_dispose_compiler(compiler);
}
