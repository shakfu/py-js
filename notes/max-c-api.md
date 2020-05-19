
# Max C API Notes



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

Avoising crashes when sending:

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
