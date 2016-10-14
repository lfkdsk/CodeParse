#include "MEM.h"
#include "DBG.h"
#define GLOBAL_VARIABLE_DEFINE
#include "crowbar.h"

static void
add_native_functions(CRB_Interpreter *inter)
{
    CRB_add_native_function(inter, "print", crb_nv_print_proc);
    CRB_add_native_function(inter, "fopen", crb_nv_fopen_proc);
    CRB_add_native_function(inter, "fclose", crb_nv_fclose_proc);
    CRB_add_native_function(inter, "fgets", crb_nv_fgets_proc);
    CRB_add_native_function(inter, "fputs", crb_nv_fputs_proc);
}

CRB_Interpreter *
CRB_create_interpreter(void)
{
    MEM_Storage storage;
    CRB_Interpreter *interpreter;

    storage = MEM_open_storage(0);
    interpreter = MEM_storage_malloc(storage,
                                     sizeof(struct CRB_Interpreter_tag));
    interpreter->interpreter_storage = storage;
    interpreter->execute_storage = NULL;
    interpreter->variable = NULL;
    interpreter->function_list = NULL;
    interpreter->statement_list = NULL;
    interpreter->current_line_number = 1;

    crb_set_current_interpreter(interpreter);
    add_native_functions(interpreter);

    return interpreter;
}

void
CRB_compile(CRB_Interpreter *interpreter, FILE *fp)
{
    extern int yyparse(void);
    extern FILE *yyin;

    crb_set_current_interpreter(interpreter);

    yyin = fp;
    if (yyparse()) {
        /* BUGBUG */
        fprintf(stderr, "Error ! Error ! Error !\n");
        exit(1);
    }
    crb_reset_string_literal_buffer();
}

void
CRB_interpret(CRB_Interpreter *interpreter)
{
    interpreter->execute_storage = MEM_open_storage(0);
    crb_add_std_fp(interpreter);
    crb_execute_statement_list(interpreter, NULL, interpreter->statement_list);
}

static void
release_global_strings(CRB_Interpreter *interpreter) {
    while (interpreter->variable) {
        Variable *temp = interpreter->variable;
        interpreter->variable = temp->next;
        if (temp->value.type == CRB_STRING_VALUE) {
            crb_release_string(temp->value.u.string_value);
        }
    }
}

void
CRB_dispose_interpreter(CRB_Interpreter *interpreter)
{
    release_global_strings(interpreter);

    if (interpreter->execute_storage) {
        MEM_dispose_storage(interpreter->execute_storage);
    }

    MEM_dispose_storage(interpreter->interpreter_storage);
}

void
CRB_add_native_function(CRB_Interpreter *interpreter,
                        char *name, CRB_NativeFunctionProc *proc)
{
    FunctionDefinition *fd;

    fd = crb_malloc(sizeof(FunctionDefinition));
    fd->name = name;
    fd->type = NATIVE_FUNCTION_DEFINITION;
    fd->u.native_f.proc = proc;
    fd->next = interpreter->function_list;

    interpreter->function_list = fd;
}
