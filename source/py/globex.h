

const char* NAME_INT;
const char* NAME_STR;
const char* NAME_LST;
const char* NAME_TUP;
const char* NAME_MAP;

static PyObject* _print_global_INT(PyObject* pMod);

static PyObject* _print_global_INT_borrowed_ref(PyObject* pMod);

static PyObject* _print_globals(PyObject* pMod);

static PyMethodDef globex_methods[];

static PyModuleDef globex_module;

/* Add a dict of {str : int, ...}.
 * Returns 0 on success, 1 on failure.
 */
int _add_map_to_module(PyObject* module);

PyMODINIT_FUNC PyInit_globex(void);