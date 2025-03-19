//
//  server.c
//  Python interpreter server
//
//  Created by sa on 3/19/25.
//

#import <stdio.h>
#import <string.h>
#import <unistd.h>
#import <xpc/xpc.h>
#import <Python.h>


const char* py_handle_error()
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

        return pvalue_str;
    }
}

int main(int argc, const char *argv[])
{
    Py_Initialize();

    PyObject *globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());


    xpc_rich_error_t error;
    
    // Set up the xpc_listener for this service. It will handle all incoming connections.
    xpc_listener_t listener = xpc_listener_create("com.xpyc.PythonInterpreterService", NULL, 0, ^(xpc_session_t  _Nonnull peer) {
        
        // Set the incoming message handler to a block that receives the message and performs the service's task.
        xpc_session_set_incoming_message_handler(peer, ^(xpc_object_t  _Nonnull message) {
            const char* code = xpc_dictionary_get_string(message, "code");

            PyObject *pval = PyRun_String(code, Py_eval_input, globals, globals);
            if (pval == NULL) {
                const char * cstr = py_handle_error();
            } else {
                PyObject* repr = PyObject_Repr(pval);
                const char* cstr = PyUnicode_AsUTF8(repr);
            }
            Py_XDECREF(repr);
            Py_XDECREF(pval);

            // Create a reply and send it back to the client.
            xpc_object_t reply = xpc_dictionary_create_reply(message);
            xpc_dictionary_set_string(reply, "result", cstr);
            xpc_rich_error_t replyError = xpc_session_send_message(peer, reply);
            if (replyError) {
                printf("Reply failed, error: %s", xpc_rich_error_copy_description(replyError));
            }
        });
    }, &error);
    
    printf("Created listener: %s", xpc_listener_copy_description(listener));
    
    // Resuming the serviceListener starts this service. This method does not return.
    dispatch_main();
    Py_Finalize();
    PyMem_RawFree(program);
    return 0;
}

