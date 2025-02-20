/*
        Copyright 2001 - Cycling '74
        Joshua Kit Clayton jkc@cycling74.com
*/

#include "jit.common.h"

typedef struct _jit_foo
{
    t_jit_object ob;
    t_symbol* text;
} t_jit_foo;

void* _jit_foo_class;

t_jit_err jit_foo_init(void);
void jit_foo_free(t_jit_foo* x);
t_jit_foo* jit_foo_new(t_symbol* s);

t_jit_err jit_foo_init(void)
{
    long attrflags = 0;
    t_jit_object* attr;

    _jit_foo_class = jit_class_new("jit_foo", (method)jit_foo_new, (method)jit_foo_free,
                                     sizeof(t_jit_foo), 0L);

    // add attributes
    attrflags = JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_USURP_LOW;
    attr = jit_object_new(_jit_sym_jit_attr_offset, "text", _jit_sym_symbol, attrflags,
                          (method)0L, (method)0L, calcoffset(t_jit_foo, text));
    jit_class_addattr(_jit_foo_class, attr);

    jit_class_register(_jit_foo_class);

    return JIT_ERR_NONE;
}

void jit_foo_free(t_jit_foo* x)
{
    // nada
}

t_jit_foo* jit_foo_new(t_symbol* s)
{
    t_jit_foo* x = (t_jit_foo*)jit_object_alloc(_jit_foo_class);

    if (x) {
        x->text = s;
    }

    return x;
}
