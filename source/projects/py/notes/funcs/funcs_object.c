// methods_object.c

void *object_alloc(t_class *c)
void *object_new(t_symbol *name_space, t_symbol *classname, ...)
void *object_new_typed(t_symbol *name_space, t_symbol *classname, long ac, t_atom *av)
void *object_attach(t_symbol *name_space, t_symbol *s, void *x)
void *object_findregistered(t_symbol *name_space, t_symbol *s)
void *object_method(void *x, t_symbol *s, ...)
void *object_method_direct_getobject(t_object *x, t_symbol *sym)
void *object_register(t_symbol *name_space, t_symbol *s, void *x)
void *object_subscribe(t_symbol *name_space, t_symbol *s, t_symbol *classname, void *x)
void *object_super_method(t_object *x, t_symbol *s, ...);
void *object_this_method(t_object *x, t_symbol *s, ...);

t_max_err object_attach_byptr(void *x, void *registeredobject)
t_max_err object_attach_byptr_register(void *x, void *object_to_attach, t_symbol *reg_name_space)
t_max_err object_chuckattr(void *x, t_symbol *attrsym)
t_max_err object_deleteattr(void *x, t_symbol *attrsym)
t_max_err object_detach(t_symbol *name_space, t_symbol *s, void *x)
t_max_err object_detach_byptr(void *x, void *registeredobject)
t_max_err object_findregisteredbyptr(t_symbol **name_space, t_symbol **s, void *x)
t_max_err object_free(void *x)
t_max_err object_getvalueof(void *x, long *ac, t_atom **av)
t_max_err object_notify(void *x, t_symbol *s, void *data)
t_max_err object_register_getnames(t_symbol *name_space, long *namecount, t_symbol ***names)
t_max_err object_setvalueof(void *x, long ac, t_atom *av)
t_max_err object_unregister(void *x)
t_max_err object_unsubscribe(t_symbol *name_space, t_symbol *s, t_symbol *classname, void *x)
t_max_err object_method_typed(void *x, t_symbol *s, long ac, t_atom *av, t_atom *rv)
t_max_err object_method_typedfun(void *x, t_messlist *mp, t_symbol *s, long ac, t_atom *av, t_atom *rv)

method object_getmethod(void *x, t_symbol *s)
method object_method_direct_getmethod(t_object *x, t_symbol *sym)

t_class *object_class(void *x)

t_symbol *object_classname(void *x)
t_symbol *object_namespace(t_object *x)
t_symbol *object_register_unique(t_symbol *name_space, t_symbol *s, void *x)

long object_attr_usercanget(void *x,t_symbol *s)
long object_attr_usercanset(void *x,t_symbol *s)
method object_attr_method(void *x, t_symbol *methodname, void **attr, long *get)
t_max_err object_addattr(void *x, t_object *attr)
t_max_err object_attr_getvalueof(void *x, t_symbol *s, long *argc, t_atom **argv)
t_max_err object_attr_setvalueof(void *x, t_symbol *s, long argc, t_atom *argv)
void *object_attr_get(void *x, t_symbol *attrname)
void object_attr_getdump(void *x, t_symbol *s, long argc, t_atom *argv)

t_max_err object_obex_lookup(void *x, t_symbol *key, t_object **val)
t_max_err object_obex_lookuplong(void *x, t_symbol *key, t_atom_long *val)
t_max_err object_obex_lookupsym(void *x, t_symbol *key, t_symbol **val)
t_max_err object_obex_store(void *x,t_symbol *key, t_object *val)
t_max_err object_obex_storeflags(void *x,t_symbol *key, t_object *val, long flags)
t_max_err object_obex_storelong(void *x, t_symbol *key, t_atom_long val) 
t_max_err object_obex_storesym(void *x, t_symbol *key, t_symbol *val) 
void object_obex_dumpout(void *x, t_symbol *s, long argc, t_atom *argv)
long class_obexoffset_get(t_class *c)
void class_obexoffset_set(t_class *c, long offset)


/**
	Allocates the memory for an instance of an object class and initialize its object header. 
	It is used like the traditional function newobject, inside of an object's <tt>new</tt> method, but its use is required with obex-class objects.

	@ingroup obj
	@param 	c		The class pointer, returned by class_new()
	@return 		This function returns a new instance of an object class if successful, or NULL if unsuccessful.
*/
void *object_alloc(t_class *c);


/**
	Allocates the memory for an instance of an object class and initialize its object header <em>internal to Max</em>. 
	It is used similarly to the traditional function newinstance(), but its use is required with obex-class objects.

	@ingroup obj

	@param 	name_space	The desired object's name space. Typically, either the 
	 					constant #CLASS_BOX, for obex classes which can 
	 					instantiate inside of a Max patcher (e.g. boxes, UI objects, 
	 					etc.), or the constant #CLASS_NOBOX, for classes 
	 					which will only be used internally. Developers can define 
	 					their own name spaces as well, but this functionality is 
	 					currently undocumented.
	@param 	classname	The name of the class of the object to be created
	@param 	...			Any arguments expected by the object class being instantiated

	@return 			This function returns a new instance of the object class if successful, or NULL if unsuccessful.
*/
void *object_new(t_symbol *name_space, t_symbol *classname, ...);


/**
	Allocates the memory for an instance of an object class and initialize its object header <em>internal to Max</em>. 
	It is used similarly to the traditional function newinstance(), but its use is required with obex-class objects. 
	The object_new_typed() function differs from object_new() by its use of an atom list for object arguments—in this way, 
	it more resembles the effect of typing something into an object box from the Max interface.

	@ingroup obj

	@param 	name_space	The desired object's name space. Typically, either the 
	 					constant #CLASS_BOX, for obex classes which can 
	 					instantiate inside of a Max patcher (e.g. boxes, UI objects, 
	 					etc.), or the constant #CLASS_NOBOX, for classes 
	 					which will only be used internally. Developers can define 
	 					their own name spaces as well, but this functionality is 
	 					currently undocumented.
	@param 	classname	The name of the class of the object to be created
	@param 	ac			Count of arguments in <tt>av</tt>
	@param 	av			Array of t_atoms; arguments to the class's instance creation function.

	@return 			This function returns a new instance of the object class if successful, or NULL if unsuccessful.
*/
void *object_new_typed(t_symbol *name_space, t_symbol *classname, long ac, t_atom *av);


#ifndef object_free
/**
	Call the free function and release the memory for an instance of an internal object class previously instantiated using object_new(), 
	object_new_typed() or other new-style object constructor functions (e.g. hashtab_new()). 
	It is, at the time of this writing, a wrapper for the traditional function freeobject(), but its use is suggested with obex-class objects.

	@ingroup obj
	@param 	x		The pointer to the object to be freed. 
	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_free(void *x);
#endif


/**
	Sends an untyped message to an object. 
	There are some caveats to its use, however, particularly for 64-bit architectures.
	object_method_direct() should be used in cases where floating-point or other non-integer types are being passed on the stack or in return values.

	@ingroup obj

	@param 	x		The object that will receive the message 
	@param 	s		The message selector
	@param 	...		Any arguments to the message

	@return 		If the receiver object can respond to the message, object_method() returns the result. Otherwise, the function will return 0. 

	@remark 		Example: To send the message <tt>bang</tt> to the object <tt>bang_me</tt>:
	@code
	void *bang_result;
	bang_result = object_method(bang_me, gensym("bang"));
	@endcode
*/

void *object_method(void *x, t_symbol *s, ...);

  
/**
	do a strongly typed direct call to a method of an object

	@ingroup obj

	
	@param  rt		The type of the return value (double, void*, void...)
	@param	sig		the actual signature of the function in brackets ! 
					something like (t_object *, double, long)		
	@param 	x		The object where the method we want to call will be looked for,
					it will also always be the first argument to the function call
	@param 	s		The message selector
	@param 	...		Any arguments to the call, the first one will always be the object (x)

	@return 		will return anything that the called function returns, typed by (rt)
 
	@remark 		Example: To call the function identified by <tt>getcolorat</tt> on the object <tt>pwindow</tt>
					which is declared like:
					t_jrgba pwindow_getcolorat(t_object *window, double x, double y)
	@code
	double x = 44.73;
	double y = 79.21;
	t_object *pwindow;
	t_jrgba result = object_method_direct(t_jrgba, (t_object *, double, double), pwindow, gensym("getcolorat"), x, y);
	@endcode
*/
		
#define object_method_direct(rt, sig, x, s, ...) ((rt (*)sig)object_method_direct_getmethod((t_object *)x, s))((t_object *)object_method_direct_getobject((t_object *)x, s), __VA_ARGS__)

method object_method_direct_getmethod(t_object *x, t_symbol *sym);
void *object_method_direct_getobject(t_object *x, t_symbol *sym);

/**
	Sends a type-checked message to an object.

	@ingroup obj

	@param 	x		The object that will receive the message 
	@param 	s		The message selector
	@param 	ac		Count of message arguments in <tt>av</tt>
	@param 	av		Array of t_atoms; the message arguments
	@param 	rv		Return value of function, if available

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

	@remark 		If the receiver object can respond to the message, object_method_typed() returns the result in <tt>rv</tt>. Otherwise, <tt>rv</tt> will contain an #A_NOTHING atom.
*/
t_max_err object_method_typed(void *x, t_symbol *s, long ac, t_atom *av, t_atom *rv);


/**
	Currently undocumented. 

	@ingroup obj

	@param 	x		The object that will receive the message 
	@param 	mp		Undocumented
	@param 	s		The message selector
	@param 	ac		Count of message arguments in <tt>av</tt>
	@param 	av		Array of t_atoms; the message arguments
	@param 	rv		Return value of function, if available

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

	@remark 		If the receiver object can respond to the message, object_method_typedfun() returns the result in <tt>rv</tt>. Otherwise, <tt>rv</tt> will contain an #A_NOTHING atom.
*/
t_max_err object_method_typedfun(void *x, t_messlist *mp, t_symbol *s, long ac, t_atom *av, t_atom *rv);


/**
	Retrieves an object's #method for a particular message selector.

	@ingroup obj
	@param 	x		The object whose method is being queried
	@param 	s		The message selector
	@return 		This function returns the #method if successful, or method_false() if unsuccessful.
*/
method object_getmethod(void *x, t_symbol *s);


/**
	Retrieves an object instance's class name

	@ingroup obj
	@param 	x		The object instance whose class name is being queried
	@return 		The classname, or NULL if unsuccessful.
*/
t_symbol *object_classname(void *x);


t_symbol *object_namespace(t_object *x);	// return the namespace this object's class is part of


t_symbol *class_namespace(t_class *c);		// return the namespace the class is part of


/**
	Registers an object in a namespace.

	@ingroup obj

	@param 	name_space	The namespace in which to register the object. The namespace can be any symbol.
	 					If the namespace does not already exist, it is created automatically.
	@param 	s			The name of the object in the namespace. This name will be 
	 					used by other objects to attach and detach from the registered object.
	@param 	x			The object to register

	@return 	The function returns a pointer to the registered object. Under some 
				circumstances, object_register will <em>duplicate</em> the object, 
	 			and return a pointer to the duplicate—the developer should not assume 
	 			that the pointer passed in is the same pointer that has been registered. 
	 			To be safe, the returned pointer should be stored and used with the 
	 			bject_unregister() function.

	@remark		You should not register an object if the object is a UI object.
				UI objects automatically register and attach to themselves in jbox_new().
*/
void *object_register(t_symbol *name_space, t_symbol *s, void *x);


t_symbol *object_register_unique(t_symbol *name_space, t_symbol *s, void *x);


/**
	Determines a registered object's pointer, given its namespace and name.

	@ingroup obj
	
	@param 	name_space	The namespace of the registered object
	@param 	s			The name of the registered object in the namespace

	@return 	This function returns the pointer of the registered object, 
	 			if successful, or NULL, if unsuccessful.
*/
void *object_findregistered(t_symbol *name_space, t_symbol *s);


/**
	Determines the namespace and/or name of a registered object, given the object's pointer.

	@ingroup obj

	@param 	name_space	Pointer to a t_symbol *, to receive the namespace of the registered object
	@param 	s			Pointer to a t_symbol *, to receive the name of the registered object within the namespace
	@param 	x			Pointer to the registered object

	@return 	This function returns the error code #MAX_ERR_NONE if successful, 
	 			or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_findregisteredbyptr(t_symbol **name_space, t_symbol **s, void *x);

/**
	Returns all registered names in a namespace

	@ingroup obj

	@param 	name_space	Pointer to a t_symbol, the namespace to lookup names in
	@param 	namecount	Pointer to a long, to receive the count of the registered names within the namespace
	@param 	names		Pointer to a t_symbol **, to receive the allocated names. This pointer should be freed after use

	@return 	This function returns the error code <tt>MAX_ERR_NONE</tt> if successful, 
				or one of the other error codes defined in "ext_obex.h" if unsuccessful.
*/
t_max_err object_register_getnames(t_symbol *name_space, long *namecount, t_symbol ***names);
		
/**
	Attaches a client to a registered object. 
	Once attached, the object will receive notifications sent from the registered object (via the object_notify() function), 
	if it has a <tt>notify</tt> method defined and implemented.

	@ingroup obj

	@param 	name_space	The namespace of the registered object. 
						This should be the same value used in object_register() to register the object.
						If you don't know the registered object's namespace, the object_findregisteredbyptr() function can be used to determine it.
	@param 	s			The name of the registered object in the namespace. 
						If you don't know the name of the registered object, the object_findregisteredbyptr() function can be used to determine it.
	@param 	x			The client object to attach. Generally, this is the pointer to your Max object. 

	@return 	This function returns a pointer to the registered object (to the object 
	 			referred to by the combination of <tt>name_space</tt> and <tt>s</tt> 
	 			arguments) if successful, or NULL if unsuccessful.

	@remark		You should not attach an object to itself if the object is a UI object.
				UI objects automatically register and attach to themselves in jbox_new().

	@see		object_notify()
	@see		object_detach()
	@see		object_attach_byptr()
	@see		object_register()
*/
void *object_attach(t_symbol *name_space, t_symbol *s, void *x);


/**
	Detach a client from a registered object.

	@ingroup obj

	@param 	name_space	The namespace of the registered object. 
						This should be the same value used in object_register() to register the object.
						If you don't know the registered object's namespace, the object_findregisteredbyptr() function can be used to determine it.
	@param 	s			The name of the registered object in the namespace. 
						If you don't know the name of the registered object, the object_findregisteredbyptr() function can be used to determine it.
	@param 	x			The client object to attach. Generally, this is the pointer to your Max object. 

	@return				This function returns the error code #MAX_ERR_NONE if successful, 
						or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_detach(t_symbol *name_space, t_symbol *s, void *x);


/**
	Attaches a client to a registered object.  
	Unlike object_attach(), the client is specified by providing a pointer to that object 
	rather than the registered name of that object.
 
	Once attached, the object will receive notifications sent from the registered object (via the object_notify() function), 
	if it has a <tt>notify</tt> method defined and implemented.

	@ingroup obj
	@param	x					The attaching client object. Generally, this is the pointer to your Max object.
	@param	registeredobject	A pointer to the registered object to which you wish to attach.
	@return						A Max error code.
	 
	@remark						You should not attach an object to itself if the object is a UI object.
								UI objects automatically register and attach to themselves in jbox_new().
 
	@see		object_notify()
	@see		object_detach()
	@see		object_attach()
	@see		object_register() 
	@see		object_attach_byptr_register()
*/
t_max_err object_attach_byptr(void *x, void *registeredobject);


/**
	A convenience function wrapping object_register() and object_attach_byptr().

	@ingroup obj

	@param	x					The attaching client object. Generally, this is the pointer to your Max object.
	@param	object_to_attach	A pointer to the object to which you wish to registered and then to which to attach.
	@param	reg_name_space		The namespace in which to register the object_to_attach.
	@return						A Max error code.

	@see		object_register() 
	@see		object_attach_byptr()
*/		
t_max_err object_attach_byptr_register(void *x, void *object_to_attach, t_symbol *reg_name_space);


/**
	Detach a client from a registered object.
 
	@ingroup	obj
	@param		x					The attaching client object. Generally, this is the pointer to your Max object.
	@param		registeredobject	The object from which to detach.
	@return							A Max error code.

	@see		object_detach()
	@see		object_attach_byptr()
*/
t_max_err object_detach_byptr(void *x, void *registeredobject);

// function: object_subscribe
/**
 * Subscribes a client to wait for an object to register. Upon registration, the object will attach. Once attached, the object will receive notifications sent from the registered object (via the <tt>object_notify</tt> function), if it has a <tt>notify</tt> method defined and implemented. See below for more information, in the reference for <tt>object_notify</tt>.
 *
 * @ingroup obj
 *
 * @param 	name_space	The namespace of the registered object. This should be the 
 *						same value used in <tt>object_register</tt> to register the 
 *						object. If you don't know the registered object's namespace, 
 *						the <tt>object_findregisteredbyptr</tt> function can be 
 *						used to determine it.
 * @param 	s			The name of the registered object in the namespace. If you 
 *						don't know the name of the registered object, the 
 *						<tt>object_findregisteredbyptr</tt> function can be used to 
 *						determine it.
 * @param 	classname	The classname of the registered object in the namespace to
 *						use as a filter. If NULL, then it will attach to any class
 *						of object.
 * @param 	x			The client object to attach. Generally, this is the pointer to your Max object. 
 *
 * @return 	This function returns a pointer to the object if registered (to the object 
 *			referred to by the combination of <tt>name_space</tt> and <tt>s</tt> 
 *			arguments) if successful, or NULL if the object is not yet registered.
 *
 */
void *object_subscribe(t_symbol *name_space, t_symbol *s, t_symbol *classname, void *x);

// function: object_unsubscribe
/**
 * Unsubscribe a client from a registered object, detaching if the object is registered.
 *
 * @ingroup obj
 *
 * @param 	name_space	The namespace of the registered object. This should be the 
 *						same value used in <tt>object_register</tt> to register the 
 *						object. If you don't know the registered object's namespace, 
 *						the <tt>object_findregisteredbyptr</tt> function can be 
 *						used to determine it.
 * @param 	s			The name of the registered object in the namespace. If you 
 *						don't know the name of the registered object, the 
 *						<tt>object_findregisteredbyptr</tt> function can be used to 
 *						determine it.
 * @param 	classname	The classname of the registered object in the namespace to
 *						use as a filter. Currently unused for unsubscribe.
 * @param 	x			The client object to detach. Generally, this is the pointer to your Max object. 
 *
 * @return 	This function returns the error code <tt>MAX_ERR_NONE</tt> if successful, 
 *			or one of the other error codes defined in "ext_obex.h" if unsuccessful.
 *
 */
t_max_err object_unsubscribe(t_symbol *name_space, t_symbol *s, t_symbol *classname, void *x);


/**
	Removes a registered object from a namespace.

	@ingroup obj
	@param 	x		The object to unregister. This should be the pointer returned from the object_register() function.
	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_unregister(void *x);

/**
	Returns all registered names in a namespace
 
	@ingroup obj
 
	@param 	name_space	Pointer to a t_symbol, the namespace to lookup names in
	@param 	namecount	Pointer to a long, to receive the count of the registered names within the namespace
	@param 	names		Pointer to a t_symbol **, to receive the allocated names. This pointer should be freed after use
 
	@return				This function returns the error code <tt>MAX_ERR_NONE</tt> if successful, 
						or one of the other error codes defined in "ext_obex.h" if unsuccessful.
*/
t_max_err object_register_getnames(t_symbol *name_space, long *namecount, t_symbol ***names);

		
/**
	Broadcast a message (with an optional argument) from a registered object to any attached client objects.

	@ingroup obj

	@param 	x		Pointer to the registered object
	@param 	s		The message to send
	@param 	data	An optional argument which will be passed with the message. 
	 				Sets this argument to NULL if it will be unused. 

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

	@remark 		In order for client objects to receive notifications, they must define and implement a special method, <tt>notify</tt>, like so:
	@code
	class_addmethod(c, (method)myobject_notify, "notify", A_CANT, 0);
	@endcode

	@remark 		The <tt>notify</tt> method should be prototyped as:
	@code
	void myobject_notify(t_myobject *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
	@endcode
					where 
					<tt>x</tt> is the pointer to the receiving object,
					<tt>s</tt> is the name of the sending (registered) object in its namespace,
					<tt>msg</tt> is the sent message,
					<tt>sender</tt> is the pointer to the sending object, and
					<tt>data</tt> is an optional argument sent with the message. 
					This value corresponds to the data argument in the object_notify() method. 
*/
t_max_err object_notify(void *x, t_symbol *s, void *data);



/**
	Determines the class of a given object.

	@ingroup obj
	@param	x		The object to test
	@return 		This function returns the t_class * of the object's class, if successful, or NULL, if unsuccessful.
*/
t_class *object_class(void *x);


/**
	Retrieves the value of an object which supports the <tt>getvalueof/setvalueof</tt> interface. See part 2 of the pattr SDK for more information on this interface.

	@ingroup obj

	@param 	x		The object whose value is of interest
	@param 	ac		Pointer to a long variable to receive the count of arguments in <tt>av</tt>. The long variable itself should be set to 0 previous to calling this function.
	@param 	av		Pointer to a t_atom *, to receive object data. The t_atom * itself should be set to NULL previous to calling this function.

	@return			This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

	@remark 		Calling the object_getvalueof() function allocates memory for any data it returns. 
					It is the developer's responsibility to free it, using the freebytes() function.
					
	@remark 		Developers wishing to design objects which will support this function being called on them must define and implement a special method, <tt>getvalueof</tt>, like so:
	@code
	class_addmethod(c, (method)myobject_getvalueof, "getvalueof", A_CANT, 0);
	@endcode

	@remark 		The <tt>getvalueof</tt> method should be prototyped as:
	@code
	t_max_err myobject_getvalueof(t_myobject *x, long *ac, t_atom **av);
	@endcode

	@remark 		And implemented, generally, as:
	@code
	t_max_err myobj_getvalueof(t_myobj *x, long *ac, t_atom **av) 
	{
		if (ac && av) {
			if (*ac && *av) {
				// memory has been passed in; use it.
			} else {
				// allocate enough memory for your data
				*av = (t_atom *)getbytes(sizeof(t_atom));
			}
			*ac = 1; // our data is a single floating point value
			atom_setfloat(*av, x->objvalue);
		}
		return MAX_ERR_NONE;
	}

	@remark 		By convention, and to permit the interoperability of objects using the obex API, 
					developers should allocate memory in their <tt>getvalueof</tt> methods using the getbytes() function. 
	@endcode
*/
t_max_err object_getvalueof(void *x, long *ac, t_atom **av);


/**
	Sets the value of an object which supports the <tt>getvalueof/setvalueof</tt> interface.

	@ingroup obj

	@param 	x		The object whose value is of interest
	@param 	ac		The count of arguments in <tt>av</tt>
	@param 	av		Array of t_atoms; the new desired data for the object

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

	@remark 		Developers wishing to design objects which will support this function being called on them must define and implement a special method, <tt>setvalueof</tt>, like so:
	@code
	class_addmethod(c, (method)myobject_setvalueof, "setvalueof", A_CANT, 0);
	@endcode

	@remark 		The <tt>setvalueof</tt> method should be prototyped as:
	@code
	t_max_err myobject_setvalueof(t_myobject *x, long *ac, t_atom **av);
	@endcode

	@remark 		And implemented, generally, as:
	@code
	t_max_err myobject_setvalueof(t_myobject *x, long ac, t_atom *av)
	{
		if (ac && av) {
			// simulate receipt of a float value
			myobject_float(x, atom_getfloat(av));
		}
		return MAX_ERR_NONE;
	}
	@endcode
*/
t_max_err object_setvalueof(void *x, long ac, t_atom *av);

/**
	Returns the pointer to an attribute, given its name. 

	@ingroup attr

	@param 	x			Pointer to the object whose attribute is of interest
	@param 	attrname	The attribute's name

	@return 			This function returns a pointer to the attribute, if successful, or NULL, if unsuccessful.
*/
void *object_attr_get(void *x, t_symbol *attrname);


/**
	Returns the method of an attribute's <tt>get</tt> or <tt>set</tt> function, as well as a pointer to the attribute itself, from a message name. 

	@ingroup attr

	@param 	x			Pointer to the object whose attribute is of interest
	@param 	methodname	The Max message used to call the attribute's <tt>get</tt> or <tt>set</tt> function. For example, <tt>gensym("mode")</tt> or <tt>gensym("getthresh")</tt>.
	@param 	attr		A pointer to a void *, which will be set to the attribute pointer upon successful completion of the function
	@param 	get			A pointer to a long variable, which will be set to 1 upon successful completion of the function, 
						if the queried method corresponds to the <tt>get</tt> function of the attribute. 

	@return 			This function returns the requested method, if successful, or NULL, if unsuccessful.
*/
method object_attr_method(void *x, t_symbol *methodname, void **attr, long *get);


/**
	Determines if an object's attribute can be set from the Max interface (i.e. if its #ATTR_SET_OPAQUE_USER flag is set). 

	@ingroup attr

	@param 	x		Pointer to the object whose attribute is of interest
	@param 	s		The attribute's name

	@return 		This function returns 1 if the attribute can be set from the Max interface. Otherwise, it returns 0. 
*/
long object_attr_usercanset(void *x,t_symbol *s);


/**
	Determines if the value of an object's attribute can be queried from the Max interface (i.e. if its #ATTR_GET_OPAQUE_USER flag is set). 

	@ingroup attr

	@param 	x		Pointer to the object whose attribute is of interest
	@param 	s		The attribute's name

	@return 		This function returns 1 if the value of the attribute can be queried from the Max interface. Otherwise, it returns 0.
*/
long object_attr_usercanget(void *x,t_symbol *s);


/**
	Forces a specified object's attribute to send its value from the object's dumpout outlet in the Max interface. 

	@ingroup attr

	@param 	x		Pointer to the object whose attribute is of interest
	@param 	s		The attribute's name
	@param 	argc	Unused
	@param 	argv	Unused
*/
void object_attr_getdump(void *x, t_symbol *s, long argc, t_atom *argv);


t_max_err object_attr_getvalueof(void *x, t_symbol *s, long *argc, t_atom **argv);


/**
	Sets the value of an object's attribute. 

	@ingroup attr

	@param 	x		Pointer to the object whose attribute is of interest
	@param 	s		The attribute's name
	@param 	argc	The count of arguments in <tt>argv</tt>
	@param 	argv	Array of t_atoms; the new desired data for the attribute

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_attr_setvalueof(void *x, t_symbol *s, long argc, t_atom *argv);



//object specific attributes(dynamically add/delete)

/**
	Attaches an attribute directly to an object. 

	@ingroup attr

	@param 	x		An object to which the attribute should be attached
	@param 	attr	The attribute's pointer—this should be a pointer returned from attribute_new(), attr_offset_new() or attr_offset_array_new().

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_addattr(void *x, t_object *attr);


/**
	Detach an attribute from an object that was previously attached with object_addattr(). 
	The function will also free all memory associated with the attribute. 
	If you only wish to detach the attribute, without freeing it, see the object_chuckattr() function.

	@ingroup attr

	@param 	x			The object to which the attribute is attached
	@param 	attrsym		The attribute's name

	@return 			This function returns the error code #MAX_ERR_NONE if successful, 
	 					or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_deleteattr(void *x, t_symbol *attrsym);


/**
	Detach an attribute from an object that was previously attached with object_addattr(). 
	This function will <em>not</em> free the attribute (use object_free() to do this manually).

	@ingroup attr

	@param 	x			The object to which the attribute is attached
	@param 	attrsym		The attribute's name
	
	@return 			This function returns the error code #MAX_ERR_NONE if successful, 
	 					or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_chuckattr(void *x, t_symbol *attrsym);


// obex

/**
	Registers the byte-offset of the obex member of the class's data structure with the previously defined object class. 
	Use of this function is required for obex-class objects. It must be called from <tt>main()</tt>.

	@ingroup class

	@param 	c			The class pointer
	@param 	offset		The byte-offset to the obex member of the object's data structure. 
	 					Conventionally, the macro #calcoffset is used to calculate the offset.
*/
void class_obexoffset_set(t_class *c, long offset);


/**
	Retrieves the byte-offset of the obex member of the class's data structure.

	@ingroup	class
	@param	c	The class pointer
	@return 	This function returns the byte-offset of the obex member of the class's data structure.
*/
long class_obexoffset_get(t_class *c);


/**
	Retrieves the value of a data stored in the obex. 

	@ingroup obj

	@param 	x		The object pointer. This function should only be called on instantiated objects (i.e. in the <tt>new</tt> method or later), not directly on classes (i.e. in <tt>main()</tt>).
	@param 	key		The symbolic name for the data to be retrieved
	@param 	val		A pointer to a #t_object *, to be filled with the data retrieved from the obex.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

	@remark 		By default, pointers to the object's containing patcher and box objects are stored in the obex, under the keys '#P' and '#B', respectively. 
					To retrieve them, the developer could do something like the following:
	@code
	void post_containers(t_obexobj *x)
	{
		t_patcher *p;
		t_box *b;
		t_max_err err;

		err = object_obex_lookup(x, gensym("#P"), (t_object **)&p);
		err = object_obex_lookup(x, gensym("#B"), (t_object **)&b);

		post("my patcher is located at 0x%X", p);
		post("my box is located at 0x%X", b);
	}
	@endcode
*/
t_max_err object_obex_lookup(void *x, t_symbol *key, t_object **val);
t_max_err object_obex_lookuplong(void *x, t_symbol *key, t_atom_long *val);
t_max_err object_obex_lookupsym(void *x, t_symbol *key, t_symbol **val);

/**
	Stores data in the object's obex. 

	@ingroup obj

	@param 	x		The object pointer. This function should only be called on instantiated objects (i.e. in the <tt>new</tt> method or later), not directly on classes (i.e. in <tt>main()</tt>).
	@param 	key		A symbolic name for the data to be stored
	@param 	val		A #t_object *, to be stored in the obex, referenced under the <tt>key</tt>.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

	@remark 		Most developers will need to use this function for the specific purpose of storing the dumpout outlet in the obex 
					(the dumpout outlet is used by attributes to report data in response to 'get' queries). 
					For this, the developer should use something like the following in the object's <tt>new</tt> method:
	@code
	object_obex_store(x, _sym_dumpout, outlet_new(x, NULL));
	@endcode
*/
t_max_err object_obex_store(void *x,t_symbol *key, t_object *val);
t_max_err object_obex_storeflags(void *x,t_symbol *key, t_object *val, long flags);

t_max_err object_obex_storelong(void *x, t_symbol *key, t_atom_long val); 
t_max_err object_obex_storesym(void *x, t_symbol *key, t_symbol *val); 


/**
	Sends data from the object's dumpout outlet. 
	The dumpout outlet is stored in the obex using the object_obex_store() function (see above).
	It is used approximately like outlet_anything().

	@ingroup obj

	@param 	x		The object pointer. 
					This function should only be called on instantiated objects (i.e. in the <tt>new</tt> method or later), not directly on classes (i.e. in <tt>main()</tt>).
	@param 	s		The message selector #t_symbol *
	@param 	argc	Number of elements in the argument list in argv
	@param 	argv	t_atoms constituting the message arguments

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
void object_obex_dumpout(void *x, t_symbol *s, long argc, t_atom *argv);




/**
	Sends an untyped message to an object using superclass methods.
	Uses a thread specific stack to ensure traversal up the class hierarchy.

	@ingroup obj

	@param 	x		The object that will receive the message 
	@param 	s		The message selector
	@param 	...		Any arguments to the message

	@return 		If the receiver object can respond to the message, object_method() returns the result. Otherwise, the function will return 0. 
*/		
void *object_super_method(t_object *x, t_symbol *s, ...);


/**
	Sends an untyped message to an object, respects a thread specific class stack from object_super_method() calls

	@ingroup obj

	@param 	x		The object that will receive the message 
	@param 	s		The message selector
	@param 	...		Any arguments to the message

	@return 		If the receiver object can respond to the message, object_method() returns the result. Otherwise, the function will return 0. 
*/				
void *object_this_method(t_object *x, t_symbol *s, ...);


