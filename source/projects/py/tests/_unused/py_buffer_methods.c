
/**
 * @brief Resize a named buffer
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector (buf_name: sym, new_size: long)
 * @return t_max_err error code
 */
t_max_err py_bufresize(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    t_object* obj = NULL;
    char* buf_name = NULL;
    t_atom_long new_size = 0;
    t_max_err err = 0;

    if (argc < 2) {
        py_error(x, "need at least 2 args to resize buffer");
        goto error;
    }

    if ((argv + 0)->a_type != A_SYM) {
        py_error(
            x, "1st arg of bufresize needs to be a symbol name of the target buffer");
        goto error;
    }

    // argv+0 is the object name to send to
    buf_name = atom_getsym(argv)->s_name;
    py_log(x, "buf_name: %s", buf_name);
    py_log(x, "resize: %d", argv+1);
    if (buf_name == NULL) {
        goto error;
    }

    // if registry is empty, scan it
    if (hashtab_getsize(py_global_registry) == 0) {
        py_scan(x);
    }

    // lookup name in registry
    err = hashtab_lookup(py_global_registry, gensym(buf_name), &obj);
    if (err != MAX_ERR_NONE || obj == NULL) {
        py_error(x, "no object found in the registry");
        goto error;
    }

    // atom after the name of the receiver
    switch ((argv + 1)->a_type) {
    case A_FLOAT: {
        new_size = (t_atom_long)(argv + 1);
        break;
    }
    case A_LONG: {
        new_size = (argv + 1);
        break;
    }
    default:
        py_log(x, "cannot process size type must either float or int");
        break;
    }

    err = object_method_typed ((t_object *)obj, gensym("sizeinsamps"), 1, new_size, NULL);
    if (err) {
        py_error(x, "failed to resize buffer %s", buf_name);
        goto error;
    }

    // success
    return MAX_ERR_NONE;

error:
    py_error(x, "py_bufresize failed");
    return MAX_ERR_GENERIC;
}



/**
 * @brief Resize a named buffer ref method
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector (buf_name: sym, new_size: long)
 * @return t_max_err error code
 */
t_max_err py_bufresize2(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    t_buffer_obj* buf_obj = NULL;
    t_buffer_ref* buf_ref = NULL;
    t_symbol* buf_name = NULL;
    t_atom_long new_size = 0;
    t_max_err err = 0;

    if (argc < 2) {
        py_error(x, "need at least 2 args to resize buffer");
        goto error;
    }

    if ((argv + 0)->a_type != A_SYM) {
        py_error(
            x, "1st arg of bufresize needs to be a symbol name of the target buffer");
        goto error;
    }

    // argv+0 is the object name to send to
    buf_name = atom_getsym(argv);
    new_size = atom_getlong(argv+1);
    py_log(x, "buf_name: %s", buf_name->s_name);
    py_log(x, "resize: %d", new_size);
    if (buf_name == NULL) {
        goto error;
    }
    if (new_size == 0) {
        goto error;
    }


    buf_ref = buffer_ref_new(x, buf_name);
    if (!buffer_ref_exists(buf_ref)) {
        goto error;
    }

    buf_obj = buffer_ref_getobject(buf_ref);


    err = buffer_edit_begin(buf_obj);
    if (err) {
        py_error(x, "cannot start editing buffer '%s'",
                     buf_name->s_name);
        goto error;        
    }

    err = object_method_typed((t_object *)buf_obj, gensym("sizeinsamps"), 1, argv+1, NULL);
    if (err) {
        py_error(x, "failed to resize buffer %s", buf_name->s_name);
        goto error;
    }

    err = buffer_edit_end(buf_obj, 1); // second arg is 'valid' with positive=TRUE
    if (err) {
        py_error(x, "cannot end editing buffer %s",
                     buf_name->s_name);
        goto error;        
    }

    buffer_setdirty(buf_obj);

    goto cleanup;

error:
    py_error(x, "py_bufresize failed");

cleanup:
    if (buf_ref)
        object_free(buf_ref);
    buf_ref = NULL;

finally:
    if (err)
        return MAX_ERR_GENERIC;
    return MAX_ERR_NONE;
}