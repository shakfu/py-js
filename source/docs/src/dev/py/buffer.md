# Wrapping the `buffer~` object




## Articles on buffer c-api

- Buffer creation: to create a buffer as per a [post](https://cycling74.com/forums/how-to-create-a-buffer-and-change-its-size) by Emanual Jourdan:

```c
t_atom a;
atom_setsym(&a, gensym("toto"));
t_object *b = object_new_typed(CLASS_BOX, gensym("buffer~"), 1, &a);
atom_setsym(&a, gensym("anton.aif"));
typedmess(b, gensym("replace"), 1, &a);
```

or 

```c
t_object * create_empty_buffer(t_py* x, t_symbol* name, long size_ms)
{
	t_atom argv[2];
	atom_setsym(argv + 0, name);
	atom_setlong(argv + 1, size_ms);
	t_object *buf = object_new_typed(CLASS_BOX, gensym("buffer~"), 2, argv);
	return buf;
}

t_object * create_buffer(t_py* x, t_symbol* name, t_symbol* sample_file)
{
	t_atom argv[2];
	atom_setsym(argv + 0, name);
	atom_setsym(argv + 1, sample_file);
	t_object *buf = object_new_typed(CLASS_BOX, gensym("buffer~"), 2, argv);
	return buf;
}

``` 

in cython

```python
@staticmethod
cdef Buffer empty(mx.t_object *x, str name, int size_ms):
    """create a buffer from scratch given name and file to load"""
    # create a buffer
    cdef mx.t_atom argv[2];
    mx.atom_setsym(argv+0, str_to_sym(name))
 	mx.atom_setlong(argv+1, size_ms);  
    cdef mx.t_object *b = <mx.t_object*>mx.object_new_typed(
        mx.CLASS_BOX, mx.gensym("buffer~"), 2, argv)

    # now retrieve buffer by name
    return Buffer.from_name(x, name)
```