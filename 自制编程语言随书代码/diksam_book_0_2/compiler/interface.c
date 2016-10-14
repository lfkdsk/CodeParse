#include "MEM.h"
#include "DBG.h"
#define GLOBAL_VARIABLE_DEFINE
#include "diksamc.h"

DKC_Compiler *
DKC_create_compiler(void)
{
    MEM_Storage storage;
    DKC_Compiler *compiler;

    storage = MEM_open_storage(0);
    compiler = MEM_storage_malloc(storage,
                                  sizeof(struct DKC_Compiler_tag));
    compiler->compile_storage = storage;
    compiler->function_list = NULL;
    compiler->function_count = 0;
    compiler->declaration_list = NULL;
    compiler->statement_list = NULL;
    compiler->current_block = NULL;
    compiler->current_line_number = 1;
    compiler->input_mode = DKC_FILE_INPUT_MODE;
#ifdef EUC_SOURCE
    compiler->source_encoding = EUC_ENCODING;
#else
#ifdef SHIFT_JIS_SOURCE
    compiler->source_encoding = SHIFT_JIS_ENCODING;
#else
#ifdef UTF_8_SOURCE
    compiler->source_encoding = UTF_8_ENCODING;
#else
    DBG_panic(("source encoding is not defined.\n"));
#endif
#endif
#endif

    dkc_set_current_compiler(compiler);

    return compiler;
}

static DVM_Executable *do_compile(DKC_Compiler *compiler)
{
    extern int yyparse(void);
    DVM_Executable *exe;

    dkc_set_current_compiler(compiler);
    if (yyparse()) {
        fprintf(stderr, "Error ! Error ! Error !\n");
        exit(1);
    }
    dkc_fix_tree(compiler);
    exe = dkc_generate(compiler);
/*
    dvm_disassemble(exe);
*/

    return exe;
}

DVM_Executable *
DKC_compile(DKC_Compiler *compiler, FILE *fp)
{
    extern FILE *yyin;
    DVM_Executable *exe;

    compiler->current_line_number = 1;
    compiler->input_mode = DKC_FILE_INPUT_MODE;

    yyin = fp;

    exe = do_compile(compiler);

    dkc_reset_string_literal_buffer();

    return exe;
}

DVM_Executable *
DKC_compile_string(DKC_Compiler *compiler, char **lines)
{
    extern int yyparse(void);
    DVM_Executable *exe;

    dkc_set_source_string(lines);
    compiler->current_line_number = 1;
    compiler->input_mode = DKC_STRING_INPUT_MODE;

    exe = do_compile(compiler);

    dkc_reset_string_literal_buffer();

    return exe;
}

void
DKC_dispose_compiler(DKC_Compiler *compiler)
{
    FunctionDefinition *fd_pos;

    for (fd_pos = compiler->function_list; fd_pos; fd_pos = fd_pos->next) {
        MEM_free(fd_pos->local_variable);
    }
    MEM_dispose_storage(compiler->compile_storage);
}
