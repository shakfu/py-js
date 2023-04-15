/**
    @file mpy - a micropython external

*/

#include "ext.h"
#include "ext_obex.h"

#include "port/micropython_embed.h"

// This is example 1 script, which will be compiled and executed.
static const char *example_1 =
    "print('hello world!', list(x + 1 for x in range(10)), end='eol\\n')";

// This is example 2 script, which will be compiled and executed.
static const char *example_2 =
    "for i in range(10):\n"
    "    print('iter {:08}'.format(i))\n"
    "\n"
    "try:\n"
    "    1//0\n"
    "except Exception as er:\n"
    "    print('caught exception', repr(er))\n"
    "\n"
    "import gc\n"
    "print('run GC collect')\n"
    "gc.collect()\n"
    "\n"
    "print('finish')\n"
    ;

// This array is the MicroPython GC heap.
static char HEAP[8 * 1024];


typedef struct mpy {
    t_object c_obj;
    void* c_outlet;

} t_mpy;


// prototypes
void* mpy_new(t_symbol* s, long argc, t_atom* argv);
void mpy_free(t_mpy* x);

// generic methods
void mpy_bang(t_mpy* x);

// core py methods
// t_max_err mpy_import(t_mpy* x, t_symbol* s);
// t_max_err mpy_eval(t_mpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err mpy_exec(t_mpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err mpy_execfile(t_mpy* x, t_symbol* s);

// extra py methods
// t_max_err mpy_call(t_mpy* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
// t_max_err mpy_assign(t_mpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err mpy_code(t_mpy* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
// t_max_err mpy_anything(t_mpy* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
// t_max_err mpy_pipe(t_mpy* x, t_symbol* s, long argc, t_atom* argv, void* outlet);


static t_class* s_mpy_class = NULL;

void ext_main(void* r)
{
    t_class* c = class_new("mpy", (method)mpy_new, (method)mpy_free,
                           sizeof(t_mpy), (method)0L, A_GIMME, 0);

    class_addmethod(c, (method)mpy_bang,      "bang", 0);

    // class_addmethod(c, (method)mpy_import,    "import",   A_SYM, 0);
    // class_addmethod(c, (method)mpy_eval,      "eval",     A_GIMME, 0);
    // class_addmethod(c, (method)mpy_exec,      "exec",     A_GIMME, 0);
    // class_addmethod(c, (method)mpy_execfile,  "execfile", A_DEFSYM, 0);

    // class_addmethod(c, (method)mpy_assign,    "assign",   A_GIMME, 0);
    // class_addmethod(c, (method)mpy_call,      "call",     A_GIMME, 0);
    // class_addmethod(c, (method)mpy_code,      "code",     A_GIMME, 0);
    // class_addmethod(c, (method)mpy_pipe,      "pipe",     A_GIMME, 0);
    // class_addmethod(c, (method)mpy_anything,  "anything", A_GIMME, 0);

    class_register(CLASS_BOX, c);

    s_mpy_class = c;

}


void* mpy_new(t_symbol* s, long argc, t_atom* argv)
{
    t_mpy* x = (t_mpy*)object_alloc(s_mpy_class);
    x->c_outlet = bangout(x);
    return x;
}


void mpy_free(t_mpy* x)
{
    // Deinitialise MicroPython.
}


void mpy_bang(t_mpy* x) {
    // test embedded MicroPython
    mp_embed_init(&HEAP[0], sizeof(HEAP));
    mp_embed_exec_str(example_1);
    mp_embed_exec_str(example_2);
    mp_embed_deinit();
    
    outlet_bang(x->c_outlet); 
}


// t_max_err mpy_import(t_mpy* x, t_symbol* s)
// {
//     return py_import(x->py, s); // returns t_max_err
// }


// t_max_err mpy_eval(t_mpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return py_eval(x->py, s, argc, argv, x->c_outlet);
// }


// t_max_err mpy_exec(t_mpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return py_exec(x->py, s, argc, argv);
// }


// t_max_err mpy_execfile(t_mpy* x, t_symbol* s)
// {
//     return py_execfile(x->py, s);
// }


// t_max_err mpy_call(t_mpy* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
// {
//     return py_call(x->py, s, argc, argv, x->c_outlet);
// }


// t_max_err mpy_assign(t_mpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return py_assign(x->py, s, argc, argv);
// }


// t_max_err mpy_code(t_mpy* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
// {
//     return py_code(x->py, s, argc, argv, x->c_outlet);
// }


// t_max_err mpy_anything(t_mpy* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
// {
//     return py_anything(x->py, s, argc, argv, x->c_outlet);
// }


// t_max_err mpy_pipe(t_mpy* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
// {
//     return py_pipe(x->py, s, argc, argv, x->c_outlet);
// }
