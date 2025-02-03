# mxpy: pdpython for max

This is an ongoing attempt to translate pdpython to maxmsp
from <https://github.com/garthz/pdpython> and
my fork <https://github.com/shakfu/pdpython>

```text
// pdpython.c : Pd external to bridge data in and out of Python
// Copyright (c) 2014, Garth Zeglin.  All rights reserved.  Provided under the
// terms of the BSD 3-clause license.
```

## TODO

- [ ] make ints, floats, and basic symbols work

- [ ] make `mxpy_eval` to handle bang (see `py_send`), ints, floats, list, etc..

- [ ] Fix the restriction that typed methods will fail, unless we `A_CANT` or `A_GIMMEBACK`?

See a simple `A_GIMMEBACK` example `simplejs.c` example in the max-sdk

see:

- <https://cycling74.com/forums/a_gimmeback-routine-appears-in-quickref-menu>

- <https://cycling74.com/forums/obex-how-to-get-return-values-from-object_method>

## Flow

1. `ext_main`: sets up one 'anything' method:

    ```c
    class_addmethod(c, (method)mxpy_eval, "anything", A_GIMME, 0)
    ```

2. `mxpy_new`: where object `[mxpy module Class arg1 arg2 arg3 .. argN]`

    ```c
    PyObject* args = t_atom_list_to_PyObject_list(argc - 2, argv + 2)
    x->py_object = PyObject_CallObject(func, args)
    ```

   note: if `Class` is actually a function which returns a non-callable value then `x->py_object` contains could be return with a bang

3. `mxpy_eval`: responds to max message and dispatches to the python-Class

4. `emit_outlet_message`: output returned values to outlet, tuples are output in sequence
