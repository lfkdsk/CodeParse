#include "DBG.h"
#include "crowbar.h"

CRB_Char *
CRB_object_get_string(CRB_Object *obj)
{
    DBG_assert(obj->type == STRING_OBJECT,
               ("obj->type..%d\n", obj->type));
    return obj->u.string.string;
}

void *
CRB_object_get_native_pointer(CRB_Object *obj)
{
    DBG_assert(obj->type == NATIVE_POINTER_OBJECT,
               ("obj->type..%d\n", obj->type));
    return obj->u.native_pointer.pointer;
}

void
CRB_object_set_native_pointer(CRB_Object *obj, void *p)
{
    DBG_assert(obj->type == NATIVE_POINTER_OBJECT,
               ("obj->type..%d\n", obj->type));
    obj->u.native_pointer.pointer = p;
}

CRB_Value *
CRB_array_get(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
              CRB_Object *obj, int index)
{
    return &obj->u.array.array[index];
}

void
CRB_array_set(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
              CRB_Object *obj, int index, CRB_Value *value)
{
    DBG_assert(obj->type == ARRAY_OBJECT,
               ("obj->type..%d\n", obj->type));
    obj->u.array.array[index] = *value;
}

CRB_Value
CRB_create_closure(CRB_LocalEnvironment *env, CRB_FunctionDefinition *fd)
{
    CRB_Value ret;

    ret.type = CRB_CLOSURE_VALUE;
    ret.u.closure.function = fd;
    ret.u.closure.environment = env->variable;

    return ret;
}

void
CRB_set_function_definition(char *name, CRB_NativeFunctionProc *proc,
                            CRB_FunctionDefinition *fd)
{
    fd->name = name;
    fd->type = CRB_NATIVE_FUNCTION_DEFINITION;
    fd->is_closure = CRB_TRUE;
    fd->u.native_f.proc = proc;
}

