#import <stdio.h>
#import <xpc/xpc.h>
#import <Python.h>

long evaluate_python(const char* code);

long evaluate_python(const char* code)
{
    wchar_t* python_home = NULL;
    PyStatus status;

    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config.parse_argv = 0; // Disable parsing command line arguments
    config.isolated = 0;   // default is disabled
    config.home = python_home;

    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        printf("could not initialize python\n");
    }

    PyConfig_Clear(&config);

    PyObject* main_mod = PyImport_AddModule("__main__"); // borrowed
    PyObject* globals = PyModule_GetDict(main_mod); // borrowed reference
    PyObject* builtins = PyEval_GetBuiltins(); // borrowed
    PyDict_SetItemString(globals, "__builtins__", builtins);

    PyObject* pval = PyRun_String(code, Py_eval_input, globals, globals);

    long long_result = PyLong_AsLong(pval);

    printf("result: %ld\n", long_result);

    Py_XDECREF(pval);
    Py_XDECREF(globals);
    Py_Finalize();
    return long_result;
}

int main(int argc, const char *argv[])
{
    xpc_rich_error_t error;
    
    // Set up the xpc_listener for this service. It will handle all incoming connections.
    xpc_listener_t listener = xpc_listener_create("xpyc.PythonService", NULL, 0, ^(xpc_session_t  _Nonnull peer) {
        
        // Set the incoming message handler to a block that receives the message and performs the service's task.
        xpc_session_set_incoming_message_handler(peer, ^(xpc_object_t  _Nonnull message) {
            const char* code = xpc_dictionary_get_string(message, "code");
//            int64_t firstNumber = xpc_dictionary_get_int64(message, "firstNumber");
//            int64_t secondNumber = xpc_dictionary_get_int64(message, "secondNumber");
            
            // Create a reply and send it back to the client.
            xpc_object_t reply = xpc_dictionary_create_reply(message);
//            xpc_dictionary_set_int64(reply, "result", firstNumber + secondNumber);
            xpc_dictionary_set_int64(reply, "result", evaluate_python(code));
            xpc_rich_error_t replyError = xpc_session_send_message(peer, reply);
            if (replyError) {
                printf("Reply failed, error: %s", xpc_rich_error_copy_description(replyError));
            }
        });
    }, &error);
    
    printf("Created listener: %s", xpc_listener_copy_description(listener));
    
    // Resuming the serviceListener starts this service. This method does not return.
    dispatch_main();

    return 0;
}
