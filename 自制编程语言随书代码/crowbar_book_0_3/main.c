#include <stdio.h>
#include <locale.h>
#include "CRB.h"
#include "MEM.h"

int
main(int argc, char **argv)
{
    CRB_Interpreter     *interpreter;
    FILE *fp;

    if (argc != 2) {
        fprintf(stderr, "usage:%s filename", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", argv[1]);
        exit(1);
    }
    setlocale(LC_CTYPE, "");
    interpreter = CRB_create_interpreter();
    CRB_compile(interpreter, fp);
    CRB_interpret(interpreter);
    CRB_dispose_interpreter(interpreter);

    MEM_dump_blocks(stdout);

    return 0;
}
