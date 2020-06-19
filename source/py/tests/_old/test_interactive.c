#include <stdio.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <compile.h>
#include <eval.h>
#include <object.h>

#include "readline.h"

int add_history(char*);

int main(int argc, char* argv[])
{
    int i, j, done = 0; /* lengths of line, code */
    char ps1[] = ">>> ";
    char ps2[] = "... ";
    char* prompt = ps1;
    char *msg, *line, *code = NULL;
    PyObject *src, *glb, *loc;
    PyObject *exc, *val, *trb, *obj, *dum;

    Py_Initialize();
    loc = PyDict_New();
    glb = PyDict_New();
    PyDict_SetItemString(glb, "__builtins__", PyEval_GetBuiltins());

    while (!done) {
        line = readline(prompt);

        if (NULL == line) /* Ctrl-D pressed */
        {
            done = 1;
        } else {
            i = strlen(line);

            if (i > 0)
                add_history(line); /* save non-empty lines */

            if (NULL == code) /* nothing in code yet */
                j = 0;
            else
                j = strlen(code);

            code = realloc(code, i + j + 2);
            if (NULL == code) /* out of memory */
                exit(1);

            if (0 == j)         /* code was empty, so */
                code[0] = '\0'; /* keep strncat happy */

            strncat(code, line, i); /* append line to code */
            code[i + j] = '\n';     /* append '\n' to code */
            code[i + j + 1] = '\0';

            src = Py_CompileString(code, "<stdin>", Py_single_input);

            if (NULL != src) /* compiled just fine - */
            {
                if (ps1 == prompt ||         /* ">>> " or */
                    '\n' == code[i + j - 1]) /* "... " and double '\n' */
                {                            /* so execute it */
                    dum = PyEval_EvalCode(src, glb, loc);
                    Py_XDECREF(dum);
                    Py_XDECREF(src);
                    free(code);
                    code = NULL;
                    if (PyErr_Occurred())
                        PyErr_Print();
                    prompt = ps1;
                }
            } /* syntax error or E_EOF? */
            else if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
                PyErr_Fetch(&exc, &val, &trb); /* clears exception! */

                if (PyArg_ParseTuple(val, "sO", &msg, &obj)
                    && !strcmp(msg,
                               "unexpected EOF while parsing")) /* E_EOF */
                {
                    Py_XDECREF(exc);
                    Py_XDECREF(val);
                    Py_XDECREF(trb);
                    prompt = ps2;
                } else /* some other syntax error */
                {
                    PyErr_Restore(exc, val, trb);
                    PyErr_Print();
                    free(code);
                    code = NULL;
                    prompt = ps1;
                }
            } else /* some non-syntax error */
            {
                PyErr_Print();
                free(code);
                code = NULL;
                prompt = ps1;
            }

            free(line);
        }
    }

    Py_XDECREF(glb);
    Py_XDECREF(loc);
    Py_Finalize();
    exit(0);
}
