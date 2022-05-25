//  Python interpreter server
#include <czmq.h>
#include <Python.h>


void py_handle_error()
{
    if (PyErr_Occurred()) {

        // get error info
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
        Py_XDECREF(ptype);

        PyObject* pvalue_pstr = PyObject_Repr(pvalue);
        const char* pvalue_str = PyUnicode_AsUTF8(pvalue_pstr);
        Py_XDECREF(pvalue);
        Py_XDECREF(pvalue_pstr);
        Py_XDECREF(ptraceback);

        puts(pvalue_str);
    }
}


int main(int argc, char* argv[])
{
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    Py_SetProgramName(program);
    Py_Initialize();

    PyObject *globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

    //  Socket to talk to clients
    zsock_t *responder = zsock_new(ZMQ_REP);
    int rc = zsock_bind(responder, "tcp://*:5555");
    assert (rc == 0);

    int count = 0;
    while (1) {
        count += 1;
        if (count > 10) break;
        char *recv_str = zstr_recv(responder);
        printf("server received '%s'\n", recv_str);
        PyObject *pval = PyRun_String(recv_str, Py_eval_input, globals, globals);
        if (pval == NULL) {
            py_handle_error();
            goto finally;
        }
        PyObject* repr = PyObject_Repr(pval);
        const char* cstr = PyUnicode_AsUTF8(repr);
        Py_XDECREF(repr);
        Py_XDECREF(pval);
        sleep(1);          //  Do some 'work'
        printf("server response: %s\n", cstr);
        zstr_send(responder, cstr);
        zstr_free(&recv_str);
    }
    finally:
        zsock_destroy (&responder);
        Py_Finalize();
        PyMem_RawFree(program);
        return 0;
}
// 
