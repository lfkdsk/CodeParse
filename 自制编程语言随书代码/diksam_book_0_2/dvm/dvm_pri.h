#ifndef DVM_PRI_H_INCLUDED
#define DVM_PRI_H_INCLUDED
#include "DVM_code.h"
#include "DVM_dev.h"
#include "share.h"

#define STACK_ALLOC_SIZE (4096)
#define HEAP_THRESHOLD_SIZE     (1024 * 256)
#define NULL_STRING (L"null")
#define TRUE_STRING (L"true")
#define FALSE_STRING (L"false")

#define NO_LINE_NUMBER_PC (-1)

#define MESSAGE_ARGUMENT_MAX    (256)
#define LINE_BUF_SIZE (1024)


#define GET_2BYTE_INT(p) (((p)[0] << 8) + (p)[1])
#define SET_2BYTE_INT(p, value) \
  (((p)[0] = (value) >> 8), ((p)[1] = value & 0xff))

typedef enum {
    INT_MESSAGE_ARGUMENT = 1,
    DOUBLE_MESSAGE_ARGUMENT,
    STRING_MESSAGE_ARGUMENT,
    CHARACTER_MESSAGE_ARGUMENT,
    POINTER_MESSAGE_ARGUMENT,
    MESSAGE_ARGUMENT_END
} MessageArgumentType;

typedef struct {
    char *format;
} ErrorDefinition;

typedef enum {
    BAD_MULTIBYTE_CHARACTER_ERR = 1,
    FUNCTION_NOT_FOUND_ERR,
    FUNCTION_MULTIPLE_DEFINE_ERR,
    INDEX_OUT_OF_BOUNDS_ERR,
    DIVISION_BY_ZERO_ERR,
    NULL_POINTER_ERR,
    RUNTIME_ERROR_COUNT_PLUS_1
} RuntimeError;

typedef struct {
    DVM_Char    *string;
} VString;

typedef enum {
    NATIVE_FUNCTION,
    DIKSAM_FUNCTION
} FunctionKind;

typedef struct {
    DVM_NativeFunctionProc *proc;
    int arg_count;
} NativeFunction;

typedef struct {
    DVM_Executable      *executable;
    int                 index;
} DiksamFunction;

typedef struct {
    char                *name;
    FunctionKind        kind;
    union {
        NativeFunction native_f;
        DiksamFunction diksam_f;
    } u;
} Function;

typedef struct {
    Function    *caller;
    int         caller_address;
    int         base;
} CallInfo;

#define revalue_up_align(val)   ((val) ? (((val) - 1) / sizeof(DVM_Value) + 1)\
                                 : 0)
#define CALL_INFO_ALIGN_SIZE    (revalue_up_align(sizeof(CallInfo)))

typedef union {
    CallInfo    s;
    DVM_Value   u[CALL_INFO_ALIGN_SIZE];
} CallInfoUnion;

typedef struct {
    int         alloc_size;
    int         stack_pointer;
    DVM_Value   *stack;
    DVM_Boolean *pointer_flags;
} Stack;

typedef enum {
    STRING_OBJECT = 1,
    ARRAY_OBJECT,
    OBJECT_TYPE_COUNT_PLUS_1
} ObjectType;

struct DVM_String_tag {
    DVM_Boolean is_literal;
    DVM_Char    *string;
};

typedef enum {
    INT_ARRAY = 1,
    DOUBLE_ARRAY,
    OBJECT_ARRAY
} ArrayType;

struct DVM_Array_tag {
    ArrayType   type;
    int         size;
    int         alloc_size;
    union {
        int             *int_array;
        double          *double_array;
        DVM_Object      **object;
    } u;
};

struct DVM_Object_tag {
    ObjectType  type;
    unsigned int        marked:1;
    union {
        DVM_String      string;
        DVM_Array       array;
    } u;
    struct DVM_Object_tag *prev;
    struct DVM_Object_tag *next;
};

typedef struct {
    int         current_heap_size;
    int         current_threshold;
    DVM_Object  *header;
} Heap;

typedef struct {
    int         variable_count;
    DVM_Value   *variable;
} Static;

struct DVM_VirtualMachine_tag {
    Stack       stack;
    Heap        heap;
    Static      static_v;
    DVM_Executable      *current_executable;
    Function    *current_function;
    int         pc;
    Function    *function;
    int         function_count;
    DVM_Executable      *executable;
};

/* heap.c */
DVM_Object *
dvm_literal_to_dvm_string_i(DVM_VirtualMachine *inter, DVM_Char *str);
DVM_Object *
dvm_create_dvm_string_i(DVM_VirtualMachine *dvm, DVM_Char *str);
DVM_Object *dvm_create_array_int_i(DVM_VirtualMachine *dvm, int size);
DVM_Object *dvm_create_array_double_i(DVM_VirtualMachine *dvm, int size);
DVM_Object *dvm_create_array_object_i(DVM_VirtualMachine *dvm, int size);
void dvm_garbage_collect(DVM_VirtualMachine *dvm);
/* native.c */
void dvm_add_native_functions(DVM_VirtualMachine *dvm);
/* util.c */
void dvm_vstr_clear(VString *v);
void dvm_vstr_append_string(VString *v, DVM_Char *str);
void dvm_vstr_append_character(VString *v, DVM_Char ch);
/* error.c */
void dvm_error(DVM_Executable *exe, Function *func,
               int pc, RuntimeError id, ...);

#endif /* DVM_PRI_H_INCLUDED */
