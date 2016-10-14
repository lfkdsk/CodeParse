#include <stdio.h>
#include <locale.h>
#include "DKC.h"
#include "DVM.h"
#include "MEM.h"

int
main(int argc, char **argv)
{
    DKC_Compiler *compiler;
    FILE *fp;
    DVM_ExecutableList *list;
    DVM_VirtualMachine *dvm;

    if (argc < 2) {
        fprintf(stderr, "usage:%s filename arg1, arg2, ...", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", argv[1]);
        exit(1);
    }

    setlocale(LC_CTYPE, "");
    compiler = DKC_create_compiler();
    list = DKC_compile(compiler, fp, argv[1]);
    dvm = DVM_create_virtual_machine();
    DVM_set_executable(dvm, list);
    DKC_dispose_compiler(compiler);
    DVM_execute(dvm);
    DVM_dispose_virtual_machine(dvm);
    DVM_dispose_executable_list(list);

    MEM_check_all_blocks();
    MEM_dump_blocks(stdout);

    return 0;
}
