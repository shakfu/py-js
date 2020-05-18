/*
    py.c - basic experiment in minimal max object for calling python code

    repo - github.com/shakfu/py

    This object has 1 inlet and 2 outlets

    Basic Features

    1.  Per-Object Namespace. It responds to an 'import <module>' message in
        the left inlet which loads a python module in its namespace. Each new import
        (like python) adds to the namespace.

    2.  Eval Messages. It responds to an 'eval <expression>' message in the left inlet
        which is evaluated in the namespace and outputs results to the left outlet
        and outputs a bang from the right outlet to signal end of evaluation.

    py interpreter object
        attributes
            imports
            code

        messages
            import <module> [adds to @imports]
            eval <code> or eval @file <path>
            exec <code> or exec @file <path>
            run  <code> or run  @file <path>

            (phase 2)
            load file <path> -> into code (for persistence) and texeditor edits

            (phase N)
            embed ipython kernel? (-;

    TODO

        - [ ] add right inlet bang after eval op ends
        - [ ] add @run <script>
        - [ ] add text edit object

*/

/*--------------------------------------------------------------------
 * Includes
 */

/* max api */
#include "ext.h"
#include "ext_obex.h"

/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

/* constants */
#define PY_MAX_ATOMS 128
#define PY_MAX_NAME  "PY_NAME"
#define PY_NAMESPACE "PY_SPACE"

/*--------------------------------------------------------------------
 * Object Types
 */

/* [py] external type */
typedef struct _py {
    /* object header */
    t_object p_ob;

    /* object attributes */
    t_symbol *p_name;        /* unique name */
    t_symbol *p_module;      /* python-related */
    t_symbol *p_code;        /* python-related */
    
    /* outlet creation */
    void *p_outlet;

    /* useful objects */
    // t_object *patcher;      /* to send msgs to objects */
    // t_object *box;          /* the ui box of the py instance? */
    // t_object *registry;     /* to keep a local registry of objects? */
    
    /* python-related */
    PyObject *p_globals;    /* global python namespace (new ref) */
} t_py;


/*--------------------------------------------------------------------
 * Enums
 */

/* python execution mode */
typedef enum {
    PY_EVAL,
    PY_EXEC,
    PY_RUN
} py_mode;


/*--------------------------------------------------------------------
 * Methods
 */


/* method prototypes */
void py_bang(t_py *x);
void py_import(t_py *x, t_symbol *s);
// void py_find(t_py *x, t_symbol *s);
void py_eval(t_py *x, t_symbol *s, long argc, t_atom *argv);
void py_run(t_py *x, t_symbol *s, long argc, t_atom *argv);
void py_dblclick(t_py *x);
void *py_new(t_symbol *s, long argc, t_atom *argv);
void py_free(t_py *x);

/*--------------------------------------------------------------------
 * Helper Functions
 */

void py_init(t_py *x);
