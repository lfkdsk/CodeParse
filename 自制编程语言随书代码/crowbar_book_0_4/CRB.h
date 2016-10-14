#ifndef PUBLIC_CRB_H_INCLUDED
#define PUBLIC_CRB_H_INCLUDED
#include <stdio.h>

typedef struct CRB_Interpreter_tag CRB_Interpreter;

typedef enum {
    CRB_FILE_INPUT_MODE = 1,
    CRB_STRING_INPUT_MODE
} CRB_InputMode;

CRB_Interpreter *CRB_create_interpreter(void);
void CRB_compile(CRB_Interpreter *interpreter, FILE *fp);
void CRB_compile_string(CRB_Interpreter *interpreter, char **lines);
void CRB_set_command_line_args(CRB_Interpreter *interpreter,
                               int argc, char **argv);
void CRB_interpret(CRB_Interpreter *interpreter);
void CRB_dispose_interpreter(CRB_Interpreter *interpreter);

#endif /* PUBLIC_CRB_H_INCLUDED */
