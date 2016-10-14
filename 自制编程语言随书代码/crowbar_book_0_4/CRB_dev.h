#ifndef PUBLIC_CRB_DEV_H_INCLUDED
#define PUBLIC_CRB_DEV_H_INCLUDED
#include <wchar.h>
#include "CRB.h"

typedef wchar_t CRB_Char;

typedef enum {
    CRB_FALSE = 0,
    CRB_TRUE = 1
} CRB_Boolean;

typedef enum {
    CRB_CROWBAR_FUNCTION_DEFINITION = 1,
    CRB_NATIVE_FUNCTION_DEFINITION,
    CRB_FUNCTION_DEFINITION_TYPE_COUNT_PLUS_1
} CRB_FunctionDefinitionType;

typedef struct CRB_Object_tag CRB_Object;
typedef struct CRB_Array_tag CRB_Array;
typedef struct CRB_String_tag CRB_String;
typedef struct CRB_Assoc_tag CRB_Assoc;
typedef struct CRB_ParameterList_tag CRB_ParameterList;
typedef struct CRB_Block_tag CRB_Block;
typedef struct CRB_FunctionDefinition_tag CRB_FunctionDefinition;
typedef struct CRB_LocalEnvironment_tag CRB_LocalEnvironment;

typedef enum {
    CRB_BOOLEAN_VALUE = 1,
    CRB_INT_VALUE,
    CRB_DOUBLE_VALUE,
    CRB_STRING_VALUE,
    CRB_NATIVE_POINTER_VALUE,
    CRB_NULL_VALUE,
    CRB_ARRAY_VALUE,
    CRB_ASSOC_VALUE,
    CRB_CLOSURE_VALUE,
    CRB_FAKE_METHOD_VALUE,
    CRB_SCOPE_CHAIN_VALUE
} CRB_ValueType;

typedef struct {
    CRB_FunctionDefinition *function;
    CRB_Object          *environment; /* CRB_ScopeChain */
} CRB_Closure;

typedef struct {
    char        *method_name;
    CRB_Object  *object;
} CRB_FakeMethod;

typedef struct {
    CRB_ValueType       type;
    union {
        CRB_Boolean     boolean_value;
        int             int_value;
        double          double_value;
        CRB_Object      *object;
        CRB_Closure     closure;
        CRB_FakeMethod  fake_method;
    } u;
} CRB_Value;

typedef enum {
    CRB_INT_MESSAGE_ARGUMENT = 1,
    CRB_DOUBLE_MESSAGE_ARGUMENT,
    CRB_STRING_MESSAGE_ARGUMENT,
    CRB_CHARACTER_MESSAGE_ARGUMENT,
    CRB_POINTER_MESSAGE_ARGUMENT,
    CRB_MESSAGE_ARGUMENT_END
} CRB_MessageArgumentType;

typedef struct {
    char *format;
    char *class_name;
} CRB_ErrorDefinition;

typedef struct {
    CRB_ErrorDefinition *message_format;
} CRB_NativeLibInfo;

typedef void CRB_NativePointerFinalizeProc(CRB_Interpreter *inter,
                                           CRB_Object *obj);

typedef struct {
    char                                *name;
    CRB_NativePointerFinalizeProc       *finalizer;
} CRB_NativePointerInfo;

typedef CRB_Value CRB_NativeFunctionProc(CRB_Interpreter *interpreter,
                                         CRB_LocalEnvironment *env,
                                         int arg_count, CRB_Value *args);

struct CRB_FunctionDefinition_tag {
    char                *name;
    CRB_FunctionDefinitionType  type;
    CRB_Boolean         is_closure;
    union {
        struct {
            CRB_ParameterList   *parameter;
            CRB_Block           *block;
        } crowbar_f;
        struct {
            CRB_NativeFunctionProc      *proc;
        } native_f;
    } u;
    struct CRB_FunctionDefinition_tag   *next;
};

typedef struct CRB_Regexp_tag CRB_Regexp;

/* interface.c */
CRB_FunctionDefinition *
CRB_add_native_function(CRB_Interpreter *interpreter,
                        char *name, CRB_NativeFunctionProc *proc);

/* nativeif.c */
CRB_Char *CRB_object_get_string(CRB_Object *obj);
void *CRB_object_get_native_pointer(CRB_Object *obj);
void CRB_object_set_native_pointer(CRB_Object *obj, void *p);
CRB_Value *CRB_array_get(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                         CRB_Object *obj, int index);
void CRB_array_set(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                   CRB_Object *obj, int index, CRB_Value *value);
CRB_Value
CRB_create_closure(CRB_LocalEnvironment *env, CRB_FunctionDefinition *fd);
void CRB_set_function_definition(char *name, CRB_NativeFunctionProc *proc,
                                 CRB_FunctionDefinition *fd);

/* eval.c */
void CRB_push_value(CRB_Interpreter *inter, CRB_Value *value);
CRB_Value CRB_pop_value(CRB_Interpreter *inter);
void CRB_shrink_stack(CRB_Interpreter *inter, int shrink_size);
CRB_Value *CRB_peek_stack(CRB_Interpreter *inter, int index);
CRB_Value
CRB_call_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                  int line_number, CRB_Value *func,
                  int arg_count, CRB_Value *args);
CRB_Value
CRB_call_function_by_name(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          int line_number, char *func_name,
                          int arg_count, CRB_Value *args);
CRB_Value
CRB_call_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                int line_number, CRB_Object *obj, char *method_name,
                int arg_count, CRB_Value *args);

/* heap.c */
CRB_Object *
CRB_create_crowbar_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          CRB_Char *str);
CRB_Object *CRB_create_array(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                             int size);
CRB_Object *CRB_create_assoc(CRB_Interpreter *inter,
                             CRB_LocalEnvironment *env);
void CRB_array_add(CRB_Interpreter *inter, CRB_Object *obj, CRB_Value *v);
void CRB_array_resize(CRB_Interpreter *inter, CRB_Object *obj, int new_size);
void CRB_array_insert(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                      CRB_Object *obj, int pos,
                      CRB_Value *v, int line_number);
void CRB_array_remove(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                      CRB_Object *obj, int pos, int line_number);
CRB_Object *CRB_literal_to_crb_string(CRB_Interpreter *inter,
                                      CRB_LocalEnvironment *env,
                                      CRB_Char *str);
CRB_Object *CRB_string_substr(CRB_Interpreter *inter,
                              CRB_LocalEnvironment *env,
                              CRB_Object *str,
                              int form, int len, int line_number);
CRB_Value *CRB_add_assoc_member(CRB_Interpreter *inter, CRB_Object *assoc,
                                char *name, CRB_Value *value,
                                CRB_Boolean is_final);
/* if the same name member exists, overwrite that.
 * otherwise, add member.
 */
void
CRB_add_assoc_member2(CRB_Interpreter *inter, CRB_Object *assoc,
                      char *name, CRB_Value *value);
CRB_Value *CRB_search_assoc_member(CRB_Object *assoc, char *member_name);
CRB_Value *CRB_search_assoc_member_w(CRB_Object *assoc, char *member_name,
                                     CRB_Boolean *is_final);
CRB_Object *CRB_create_native_pointer(CRB_Interpreter *inter,
                                      CRB_LocalEnvironment *env,
                                      void *pointer,
                                      CRB_NativePointerInfo *info);
CRB_Boolean CRB_check_native_pointer_type(CRB_Object *native_pointer,
                                          CRB_NativePointerInfo *info);
CRB_NativePointerInfo *CRB_get_native_pointer_type(CRB_Object *native_pointer);
CRB_Object *CRB_create_exception(CRB_Interpreter *inter,
                                 CRB_LocalEnvironment *env,
                                 CRB_Object *message, int line_number);

/* util.c */
CRB_FunctionDefinition *CRB_search_function(CRB_Interpreter *inter,
                                            char *name);
CRB_Value *CRB_search_local_variable(CRB_LocalEnvironment *env,
                                          char *identifier);
CRB_Value *CRB_search_local_variable_w(CRB_LocalEnvironment *env,
                                       char *identifier,
                                       CRB_Boolean *is_final);
CRB_Value *
CRB_search_global_variable(CRB_Interpreter *inter, char *identifier);
CRB_Value *
CRB_search_global_variable_w(CRB_Interpreter *inter, char *identifier,
                             CRB_Boolean *is_final);

CRB_Value *
CRB_add_local_variable(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                        char *identifier, CRB_Value *value,
                        CRB_Boolean is_final);
CRB_Value *
CRB_add_global_variable(CRB_Interpreter *inter, char *identifier,
                        CRB_Value *value, CRB_Boolean is_final);
CRB_Char *
CRB_value_to_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    int line_number, CRB_Value *value);
char *CRB_get_type_name(CRB_ValueType type);
char *CRB_get_object_type_name(CRB_Object *obj);

/* wchar.c */
size_t CRB_wcslen(CRB_Char *str);
CRB_Char *CRB_wcscpy(CRB_Char *dest, CRB_Char *src);
CRB_Char *CRB_wcsncpy(CRB_Char *dest, CRB_Char *src, size_t n);
int CRB_wcscmp(CRB_Char *s1, CRB_Char *s2);
CRB_Char *CRB_wcscat(CRB_Char *s1, CRB_Char *s2);
int CRB_mbstowcs_len(const char *src);
CRB_Char *CRB_mbstowcs_alloc(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                            int line_number,const char *src);
void CRB_mbstowcs(const char *src, CRB_Char *dest);
int CRB_wcstombs_len(const CRB_Char *src);
void CRB_wcstombs(const CRB_Char *src, char *dest);
char *CRB_wcstombs_alloc(const CRB_Char *src);
char CRB_wctochar(CRB_Char src);
int CRB_print_wcs(FILE *fp, CRB_Char *str);
int CRB_print_wcs_ln(FILE *fp, CRB_Char *str);
CRB_Boolean CRB_iswdigit(CRB_Char ch);

/* error.c */
void CRB_error(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
               CRB_NativeLibInfo *info, int line_number,
               int error_code, ...);
void
CRB_check_argument_count_func(CRB_Interpreter *inter,
                              CRB_LocalEnvironment *env,
                              int line_number, int arg_count,
                              int expected_count);
#define CRB_check_argument_count(inter, env, arg_count, expected_count)\
  (CRB_check_argument_count_func(inter, env, __LINE__, \
  arg_count, expected_count))

void CRB_check_one_argument_type_func(CRB_Interpreter *inter,
                                      CRB_LocalEnvironment *env,
                                      int line_number,
                                      CRB_Value *arg,
                                      CRB_ValueType expected_type,
                                      char *func_name, int nth_arg);
#define CRB_check_one_argument_type(inter, env, arg, expected_type,\
                                    func_name, nth_arg)\
  (CRB_check_one_argument_type_func(inter, env, __LINE__, \
   arg, expected_type, func_name, nth_arg))

void CRB_check_argument_type_func(CRB_Interpreter *inter,
                                  CRB_LocalEnvironment *env,
                                  int line_number,
                                  int arg_count, int expected_count,
                                  CRB_Value *args,
                                  CRB_ValueType *expected_type,
                                  char *func_name);
#define CRB_check_argument_type(inter, env, arg_count, expected_count, \
                                args, expected_type, func_name)\
  (CRB_check_argument_type_func(inter, env, __LINE__, \
   arg_count, expected_count, args, expected_type, func_name))

void
CRB_check_argument_func(CRB_Interpreter *inter,
                        CRB_LocalEnvironment *env,
                        int line_number,
                        int arg_count, int expected_count,
                        CRB_Value *args, CRB_ValueType *expected_type,
                        char *func_name);
#define CRB_check_argument(inter, env, arg_count, expected_count,\
                            args, expected_type, func_name)\
  (CRB_check_argument_func(inter, env, __LINE__, \
  arg_count, expected_count, args, expected_type, func_name))


#endif /* PUBLIC_CRB_DEV_H_INCLUDED */
