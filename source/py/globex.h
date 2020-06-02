
// global identifiers which can be read and written to from c
const char* NAME_INT;
const char* NAME_STR;
const char* NAME_LST;
const char* NAME_TUP;
const char* NAME_MAP;

// use as (PyImport_AppendInittab("globex", PyInit_globex)
PyMODINIT_FUNC PyInit_globex(void);
