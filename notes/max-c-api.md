
# Max C API Notes


## Writing a text file

see: https://cycling74.com/forums/problem-with-sysfile_writetextfile

```c
 void buffTest_writefile(t_buffTest *x, char *filename, short path)
{
    char *buf  = "write me to a file";
    t_handle h = sysmem_newhandle(0);
    sysmem_ptrandhand(buf, h, strlen(buf));
    long err;
    t_filehandle fh;
    err = path_createsysfile(filename, path, 'TEXT', &fh);
    if (err)
        return;
    err = sysfile_writetextfile(fh, h, TEXT_LB_NATIVE);
    sysfile_close(fh);
    sysmem_freehandle(h);
}
```



## Getting Atoms from Arguments

### ext_obex.h

```c
/**
  Retrieves type from a #t_atom. 

  @ingroup atom
  @param  a   Pointer to a #t_atom whose type is of interest
  @return     This function returns the type of the specified t_atom as defined in #e_max_atomtypes
*/
long atom_gettype(const t_atom *a);
#endif


//the following are useful for setting the values _only_ if there is an arg
//rather than setting it to 0 or _sym_nothing

/**
  Retrieves the integer value of a particular t_atom from an atom list, if the atom exists.

  @ingroup atom

  @param  c   Pointer to a long variable to receive the atom's data if the function is successful.
  @param  idx   Offset into the atom list of the atom of interest, starting from 0. 
          For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
  @param  ac    Count of av.
  @param  av    Pointer to the first t_atom of an atom list.

  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

  @remark     The atom_arg_getlong() function only changes the value of <tt>c</tt> if the function is successful. 
          For instance, the following code snippet illustrates a simple, but typical use:
  @code
  void myobject_mymessage(t_myobject *x, t_symbol *s, long ac, t_atom *av)
  {
    t_atom_long var = -1;

    // here, we are expecting a value of 0 or greater
    atom_arg_getlong(&var, 0, ac, av);
    if (val == -1) // i.e. unchanged
      post("it is likely that the user did not provide a valid argument");
    else {
      ...
    }
  }
  @endcode
*/
t_max_err atom_arg_getlong(t_atom_long *c, long idx, long ac, const t_atom *av);


/**
  Retrieves the floating point value of a particular t_atom from an atom list, if the atom exists.

  @ingroup atom

  @param  c   Pointer to a float variable to receive the atom's data if the function is successful. Otherwise, the value is left unchanged.
  @param  idx   Offset into the atom list of the atom of interest, starting from 0. 
          For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
  @param  ac    Count of av.
  @param  av    Pointer to the first t_atom of an atom list.

  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
long atom_arg_getfloat(float *c, long idx, long ac, const t_atom *av);


/**
  Retrieves the floating point value, as a double, of a particular t_atom from an atom list, if the atom exists.

  @ingroup atom

  @param  c   Pointer to a double variable to receive the atom's data if the function is successful. Otherwise the value is left unchanged.
  @param  idx   Offset into the atom list of the atom of interest, starting from 0. 
          For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
  @param  ac    Count of av.
  @param  av    Pointer to the first t_atom of an atom list.

  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
long atom_arg_getdouble(double *c, long idx, long ac, const t_atom *av);


/**
  Retrieves the t_symbol * value of a particular t_atom from an atom list, if the atom exists.

  @ingroup atom

  @param  c   Pointer to a t_symbol * variable to receive the atom's data if the function is successful. Otherwise, the value is left unchanged.
  @param  idx   Offset into the atom list of the atom of interest, starting from 0. 
          For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
  @param  ac    Count of av.
  @param  av    Pointer to the first t_atom of an atom list.

  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

  @remark     The atom_arg_getsym() function only changes the value of <tt>c</tt> if the function is successful. 
          For instance, the following code snippet illustrates a simple, but typical use:
  @code
  void myobject_open(t_myobject *x, t_symbol *s, long ac, t_atom *av)
  {
    t_symbol *filename = _sym_nothing;

    // here, we are expecting a file name.
    // if we don't get it, open a dialog box 
    atom_arg_getsym(&filename, 0, ac, av);
    if (filename == _sym_nothing) { // i.e. unchanged
      // open the file dialog box,
      // get a value for filename
    }
    // do something with the filename
  }
  @endcode
*/
long atom_arg_getsym(t_symbol **c, long idx, long ac, const t_atom *av);
```



## Object creation / freeing

### ext_obex.h

```c
/**
  Allocates the memory for an instance of an object class and initialize its object header <em>internal to Max</em>. 
  It is used similarly to the traditional function newinstance(), but its use is required with obex-class objects. 
  The object_new_typed() function differs from object_new() by its use of an atom list for object arguments—in this way, 
  it more resembles the effect of typing something into an object box from the Max interface.

  @ingroup obj

  @param  name_space  The desired object's name space. Typically, either the 
            constant #CLASS_BOX, for obex classes which can 
            instantiate inside of a Max patcher (e.g. boxes, UI objects, 
            etc.), or the constant #CLASS_NOBOX, for classes 
            which will only be used internally. Developers can define 
            their own name spaces as well, but this functionality is 
            currently undocumented.
  @param  classname The name of the class of the object to be created
  @param  ac      Count of arguments in <tt>av</tt>
  @param  av      Array of t_atoms; arguments to the class's instance creation function.

  @return       This function returns a new instance of the object class if successful, or NULL if unsuccessful.
*/
void *object_new_typed(t_symbol *name_space, t_symbol *classname, long ac, t_atom *av);


/**
  Call the free function and release the memory for an instance of an internal object class previously instantiated using object_new(), object_new_typed() or other new-style object constructor functions (e.g. hashtab_new()). 
  It is, at the time of this writing, a wrapper for the traditional function freeobject(), but its use is suggested with obex-class objects.

  @ingroup obj
  @param  x   The pointer to the object to be freed. 
  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_free(void *x);
```



## Checking whether an object is an instance of a class


### ext_obex.h

```c
/**
  Determines if a particular object is an instance of a given class.

  @ingroup obj

  @param  x   The object to test
  @param  name  The name of the class to test this object against
  @return     This function returns 1 if the object is an instance of the named class. Otherwise, 0 is returned.
  @remark     For instance, to determine whether an unknown object pointer is a pointer to a print object, one would call:

  @code
  long isprint = object_classname_compare(x, gensym("print"));
  @endcode
*/
long object_classname_compare(void *x, t_symbol *name);
```


## Sending arbitrary messages to an object

### ext_obex.h

```c
/**
  Sends a type-checked message to an object.

  @ingroup obj

  @param  x   The object that will receive the message 
  @param  s   The message selector
  @param  ac    Count of message arguments in <tt>av</tt>
  @param  av    Array of t_atoms; the message arguments
  @param  rv    Return value of function, if available

  @return     This function returns the error code #MAX_ERR_NONE if successful, 
          or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

  @remark     If the receiver object can respond to the message, object_method_typed() returns the result in <tt>rv</tt>. Otherwise, <tt>rv</tt> will contain an #A_NOTHING atom.
*/
t_max_err object_method_typed(void *x, t_symbol *s, long ac, t_atom *av, t_atom *rv);
```


### ext_obex_util.h

```c
/**
  Convenience wrapper for object_method_typed() that uses atom_setparse() to define the arguments.

  @ingroup  obj
  @param    x     The object to which the message will be sent.
  @param    s     The name of the method to call on the object.
  @param    parsestr  A C-string to parse into an array of atoms to pass to the method.
  @param    rv      The address of an atom to hold a return value.
  @return   A Max error code.
  
  @see    object_method_typed()
  @see    atom_setparse()
*/
t_max_err object_method_parse(t_object *x, t_symbol *s, C74_CONST char *parsestr, t_atom *rv);
```






## Object Reference


It looks like `obex` is a type `hashtab` (Hash Table), which can be used for storing object references?


## Find named object

see: https://cycling74.com/forums/find-named-object-and-send-it-a-message

	I'm looking at some of the patcher scripting stuff in the api.  iterator.c is a good guide, but I do want to check if there's  a simple method similar to "getnamed" in javascript, such that I don't have to iterate through all the boxes in a patcher.
	Something along the lines of

```c
	t_object *desiredobject = jpatcher_get_namedobject("scriptedname");
```
	and you could then just pass desiredobject into the various jbox goodies.

and the answer

```c
t_max_err err;
t_object *patcher = NULL;
t_object *yourobject = NULL;

// get the patcher
err = object_obex_lookup(yourobjectpointer, gensym("#P"), &patcher);

// get the object that you're looking for.
yourobject = (t_object *)object_method(patcher, gensym("getnamedbox"), gensym("theobjectname"));
```

----

see: https://cycling74.com/forums/error-handling-with-object_method_typed

Avoiding crashes when sending:

	For messages which are internally defined as A_GIMME the correct call to use is object_method_typed(). But for other messages, say one with A_FLOAT as the argument, you will likely want to use object_method(). 


----

see: https://cycling74.com/forums/messnamed-equivalent-send-function-in-c-send-to-named-object


Question: 

	"want to send values to a named object, as there is in javascript with 'messnamed'... send values to receive objects."

Answer:

	all send's and receive's with the same name reference a single nobox object called "through", which you can simply retrieving by looking at the s_thing field of the name symbol — I mean, if you need to retrieve the through object associated to the "foo" symbol, just look for gensym("foo")->s_thing.
	Once you have the object, just send it a message with object_method().

```c
t_max_err object_send_method_typed(void *x, t_symbol *name, t_symbol *s, long ac, t_atom *av, t_atom *rv)
{
   t_object *thing = name->s_thing;

   if (!thing) {
       return MAX_ERR_INVALID_PTR;
   }
   if (NOGOOD(thing)) {
       return MAX_ERR_INVALID_PTR;
   }
   if (!object_classname_compare(thing, gensym("through"))) {
       return MAX_ERR_GENERIC;
   }
   return object_method_typed(thing, s, ac, av, rv);
}
```

## Dynamic Outlets?

see: https://cycling74.com/forums/dynamic-inlets-outlets

```c
// start the transaction with the box
t_object *b = NULL;
object_obex_lookup(x, _sym_pound_B, (t_object **)&b);
object_method(b, gensym("dynlet_begin"));

// update outlets with one or both of these calls
//outlet_delete(outlet_nth((t_object*)x, i));
//outlet_append((t_object*)x, NULL, gensym("signal"));

// end the transaction with the box
object_method(b, gensym("dynlet_end"));
```



## Outlets

- outlet creation order is important in `outlet_new(x, NULL)`?
