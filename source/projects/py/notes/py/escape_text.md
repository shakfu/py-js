
# How to handle text given Max's escaping rules

https://cycling74.com/forums/need-help-replicating-tosymbol-functionality-stuck-on-binbuf-string-conversion

See if it can be done without str_replace here.

## Current Solution

```c


/**
 * @brief      replace a string with a string
 *
 * @param      orig  The original
 * @param      rep   what to replace
 * @param      with  what to replace with
 *
 * @return     a new string with replaced strings
 * 
 * NOTE: You must free the result if result is non-NULL.
 * 
 */
char *str_replace(char *orig, char *rep, char *with) {
    // from: https://stackoverflow.com/questions/779875
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}



/**
 * @brief A helper function to evaluate Max text as a Python expression.
 *
 * @param x pointer to object structure
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param offset offset of atom vector from which to evaluate
 * @return t_max_err error code
 * 
 */
t_max_err py_eval_text(t_py* x, long argc, t_atom* argv, int offset)
{
    PyGILState_STATE gstate = PyGILState_Ensure();

    long textsize = 0;
    char* text = NULL;
    int is_eval = 1;
    PyObject* co = NULL;
    PyObject* pval = NULL;

    t_max_err err = atom_gettext(argc + offset, argv, &textsize, &text,
                                 OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err == MAX_ERR_NONE && textsize && text) {
        py_log(x, ">>> %s", text);
    } else {
        goto error;
    }

    char* new_text = str_replace(text, "\\", "");

    co = Py_CompileString(new_text, x->p_name->s_name, Py_eval_input);


    if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        PyErr_Clear();
        co = Py_CompileString(new_text, x->p_name->s_name, Py_single_input);
        is_eval = 0;
    }

    free(new_text);

    if (co == NULL) { // can be eval-co or exec-co or NULL here
        goto error;
    }
    sysmem_freeptr(text);

    pval = PyEval_EvalCode(co, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }
    Py_DECREF(co);

    if (!is_eval) {
        // bang for exec-type op
        PyGILState_Release(gstate);
        py_bang_success(x);
    } else {
        py_handle_output(x, pval);
        PyGILState_Release(gstate);
    }
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "python code evaluation failed");
    // fail bang
    PyGILState_Release(gstate);
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}

```



## Forum Example (Iain Duncan)

```c
void eval_atoms_as_string(t_py* x, t_symbol* sym, long argc, t_atom* argv)
{
    post("eval_atoms_as_string");
    char* token_1 = sym->s_name;
    int token_1_size = strlen(token_1);
    long size = 0;
    char* atoms_as_text = NULL;
    t_max_err err = atom_gettext(argc, argv, &size, &atoms_as_text,
                                 OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err == MAX_ERR_NONE && size && atoms_as_text) {
        int code_str_size = token_1_size + size + 1;
        char* code_str = (char*)sysmem_newptr(sizeof(char) * code_str_size);
        sprintf(code_str, "%s %s", token_1, atoms_as_text);
        post("code_str: %s", code_str);
        // now we have code, but we need to clean up Max escape chars
        char* code_str_clean = (char*)sysmem_newptr(sizeof(char)
                                                    * code_str_size);
        for (int i = 0, j = 0; i < code_str_size; i++, j++) {
            if (code_str[j] == '\\')
                code_str_clean[i] = code_str[++j];
            else
                code_str_clean[i] = code_str[j];
        }
        post("code_str cleaned: %s", code_str_clean);
        // call s4m
        s4m_s7_eval_c_string(x, code_str_clean);
    } else {
        // TODO abort error here
    }
    if (atoms_as_text) {
        sysmem_freeptr(atoms_as_text);
    }
}
```

