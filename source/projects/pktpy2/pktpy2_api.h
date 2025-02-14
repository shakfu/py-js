// pktpy2_api.h

#ifndef PKTPY2_API_H
#define PKTPY2_API_H

// ----------------------------------------------------------------------------
// includes

// max api
#include "ext.h"
#include "ext_obex.h"

// pocketpy
#include "pocketpy.h"

// ----------------------------------------------------------------------------
// custom pktpy2 functions

bool int_add(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    py_i64 a = py_toint(py_arg(0));
    py_i64 b = py_toint(py_arg(1));
    py_newint(py_retval(), a + b);
    return true;
}

void print_to_console(const char * content)
{
    post(content);
}

t_max_err demo(void) {
    // Hello world!
    bool ok = py_exec("print('Hello world!')", "<string>", EXEC_MODE, NULL);
    if(!ok) goto __ERROR;

    // Create a list: [1, 2, 3]
    py_Ref r0 = py_getreg(0);
    py_newlistn(r0, 3);
    py_newint(py_list_getitem(r0, 0), 1);
    py_newint(py_list_getitem(r0, 1), 2);
    py_newint(py_list_getitem(r0, 2), 3);

    // Eval the sum of the list
    py_Ref f_sum = py_getbuiltin(py_name("sum"));
    py_push(f_sum);
    py_pushnil();
    py_push(r0);
    ok = py_vectorcall(1, 0);
    if(!ok) goto __ERROR;

    post("Sum of the list: %d\n", (int)py_toint(py_retval()));  // 6

    // Bind native `int_add` as a global variable
    py_newnativefunc(r0, int_add);
    py_setglobal(py_name("add"), r0);

    // Call `add` in python
    ok = py_exec("add(3, 7)", "<string>", EVAL_MODE, NULL);
    if(!ok) goto __ERROR;

    py_i64 res = py_toint(py_retval());
    post("Sum of 2 variables: %d\n", (int)res);  // 10

    return MAX_ERR_NONE;

__ERROR:
    py_printexc();
    return MAX_ERR_GENERIC;
}

// ============================================================================
// api module

// ----------------------------------------------------------------------------
// functions


static bool api_post(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    const char* cstr = py_tostr(py_arg(0));
    post(cstr);
    return true;
}

static bool api_error(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    const char* cstr = py_tostr(py_arg(0));
    error(cstr);
    return true;
}

// ----------------------------------------------------------------------------
// Atom (not-yet-working)

// typedef struct {
//     t_atom *ptr;
//     bool ptr_owner;
//     long size;
// } AtomObject;

// static void AtomObject__ctor(AtomObject* self, long size) {
//     self->ptr = (t_atom *)sysmem_newptr(size * sizeof(t_atom));
//     self->size = size;
//     self->ptr_owner = true;
// }

// static void AtomObject__dtor(AtomObject* self) {
//     if (self->ptr != NULL && self->ptr_owner) {
//         sysmem_freeptr(self->ptr);
//         self->ptr = NULL;
//     }
// }


// int py_arrayview(py_Ref self, py_TValue** p) {
//     if(py_typeof(self) == tp_list) {
//         *p = py_list_data(self);
//         return py_list_len(self);
//     }
//     if(py_typeof(self) == tp_tuple) {
//         *p = py_tuple_data(self);
//         return py_tuple_len(self);
//     }
//     return -1;
// }


// static bool Atom__new__(int argc, py_Ref argv) {
//     py_TValue* args = py_tuple_data(argv);
//     int size = py_tuple_len(argv);
//     AtomObject* obj = py_newobject(py_retval(), py_totype(argv), 0, sizeof(AtomObject));
//     AtomObject__ctor(obj, size);
//     for(int i = 0; i < size; i++) {
//         if(i > 0) {
//             if (py_isint(&args + i)) {
//                 long long_item = py_toint(&args + i);
//                 atom_setlong(obj->ptr + i, long_item);
//             }
//         }
//     }
//     if(argc == 1)
//         return true;
//     return false;
// }

// static bool Atom__init__(int argc, py_Ref argv) {
//     // Atom(self, *args)
//     if(argc == 0) {
//         // do nothing
//     } else if(argc == 2) {
//         AtomObject* obj = py_touserdata(py_arg(0));
//         PY_CHECK_ARG_TYPE(1, tp_tuple);
//     } else {
//         return TypeError("Atom(): expected 1 or 2 arguments, got %d");
//     }
//     py_newnone(py_retval());
//     return true;
// }

// ----------------------------------------------------------------------------
// Person

typedef struct t_person {
    int id;
    int age;
} t_person;

static void t_person__ctor(t_person* self, int id, int age) {
    self->id = id;
    self->age = age;
}

static void t_person__dtor(t_person* self) {
    self->id = 0;
    self->age = 0;
}

static bool Person__new__(int argc, py_Ref argv) {
    t_person* ptr = py_newobject(py_retval(), py_totype(argv), 0, sizeof(t_person));
    post("Person__new__ argc = %d", argc);
    if (argc == 1)
        return true;
    if (argc == 3) {
        PY_CHECK_ARG_TYPE(1, tp_int);
        py_i64 id = py_toint(py_arg(1));
        PY_CHECK_ARG_TYPE(2, tp_int);
        py_i64 age = py_toint(py_arg(2));
        t_person__ctor(ptr, id, age);
        return true;
    }
    return TypeError("Person__new__(): expected 0 or 2 arguments, got %d", argc - 1);
}

static bool Person__init__(int argc, py_Ref argv) {
    post("Person__init__ argc = %d", argc);
    if (argc == 1) {
        // do nothing
    }
    else if(argc == 3) {
        t_person* ptr = py_touserdata(py_arg(0));
        PY_CHECK_ARG_TYPE(1, tp_int);
        py_i64 id = py_toint(py_arg(1));
        PY_CHECK_ARG_TYPE(2, tp_int);
        py_i64 age = py_toint(py_arg(2));
        t_person__ctor(ptr, id, age);
    } else {
        return TypeError("Person__init__(): xpected 0 or 2 arguments, got %d", argc - 1);
    }
    py_newnone(py_retval());
    return true;
}


// ----------------------------------------------------------------------------
// initialize

bool api_module_initialize(void) {
    py_GlobalRef mod = py_newmodule("api");
    py_bindfunc(mod, "post", api_post);
    py_bindfunc(mod, "error", api_error);

    py_Type type = py_newtype("Person", tp_object, mod, NULL);
    py_bindmagic(type, __new__, Person__new__);
    py_bindmagic(type, __init__, Person__init__);

    // py_Type type = py_newtype("Atom", tp_object, mod, NULL);
    // py_bindmagic(type, __new__, Atom__new__);
    // py_bindmagic(type, __init__, Atom__init__);

    return true;
}




// ----------------------------------------------------------------------------

#endif // PKTPY2_API_H
