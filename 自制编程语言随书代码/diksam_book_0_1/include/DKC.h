#ifndef PUBLIC_DKC_H_INCLUDED
#define PUBLIC_DKC_H_INCLUDED
#include <stdio.h>
#include "DVM_code.h"

typedef struct DKC_Compiler_tag DKC_Compiler;

typedef enum {
    DKC_FILE_INPUT_MODE = 1,
    DKC_STRING_INPUT_MODE
} DKC_InputMode;

DKC_Compiler *DKC_create_compiler(void);
DVM_Executable *DKC_compile(DKC_Compiler *compiler, FILE *fp);
DVM_Executable *DKC_compile_string(DKC_Compiler *compiler, char **lines);
void DKC_dispose_compiler(DKC_Compiler *compiler);

#endif /* PUBLIC_DKC_H_INCLUDED */
