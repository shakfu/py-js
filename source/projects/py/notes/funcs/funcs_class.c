// methods_class.c

t_class *class_new(C74_CONST char *name, C74_CONST method mnew, C74_CONST method mfree, long size, C74_CONST method mmenu, short type, ...);
t_max_err class_free(t_class *c);
t_max_err class_register(t_symbol *name_space, t_class *c);
t_max_err class_alias(t_class *c, t_symbol *aliasname);
t_max_err class_copy(t_symbol *src_name_space, t_symbol *src_classname, t_symbol *dst_name_space, t_symbol *dst_classname);	
t_max_err class_addmethod(t_class *c, C74_CONST method m, C74_CONST char *name, ...);
t_max_err class_addattr(t_class *c,t_object *attr);
t_symbol *class_nameget(t_class *c);
t_class *class_findbyname(t_symbol *name_space, t_symbol *classname);
t_class *class_findbyname_casefree(t_symbol *name_space, t_symbol *classname);
t_max_err class_dumpout_wrap(t_class *c);
t_class *class_getifloaded(t_symbol *name_space, t_symbol *classname);
t_class *class_getifloaded_casefree(t_symbol *name_space, t_symbol *classname);

long object_classname_compare(void *x, t_symbol *name);

t_hashtab *reg_object_namespace_lookup(t_symbol *name_space);
method class_method(t_class *x, t_symbol *methodname);
t_messlist *class_mess(t_class *x, t_symbol *methodname);
t_messlist *object_mess(t_object *x, t_symbol *methodname);
method class_attr_method(t_class *x, t_symbol *methodname, void **attr, long *get);
void *class_attr_get(t_class *x, t_symbol *attrname);
t_max_err class_extra_store(t_class *x,t_symbol *s,t_object *o);
t_max_err class_extra_storeflags(t_class *x,t_symbol *s,t_object *o,long flags);
void *class_extra_lookup(t_class *x,t_symbol *s);
t_max_err class_addtypedwrapper(t_class *x, method m, char *name, ...);
t_messlist *class_typedwrapper_get(t_class *x, t_symbol *s);
t_max_err object_addtypedwrapper(t_object *x, method m, char *name, ...);
t_messlist *object_typedwrapper_get(t_object *x, t_symbol *s);
t_hashtab *class_namespace_fromsym(t_symbol *name_space);
t_max_err class_namespace_getclassnames(t_symbol *name_space, long *kc, t_symbol ***kv);
t_max_err class_setpath(t_class *x, short vol);
short class_getpath(t_class *x);
 	
 

#ifndef class_new
/**
	Initializes a class by informing Max of its name, instance creation and free functions, size and argument types. 
	Developers wishing to use obex class features (attributes, etc.) <em>must</em> use class_new() 
	instead of the traditional setup() function.

	@ingroup class

	@param 	name	The class's name, as a C-string
	@param 	mnew	The instance creation function
	@param 	mfree	The instance free function
	@param 	size	The size of the object's data structure in bytes. 
					Usually you use the C sizeof operator here.
	@param 	mmenu	Obsolete - pass NULL.
	 				In Max 4 this was a function pointer for UI objects called when the user created a new object of the  
	 				class from the Patch window's palette. 
	@param 	type	A standard Max <em>type list</em> as explained in Chapter 3  
	 				of the Writing Externals in Max document (in the Max SDK).  
	 				The final argument of the type list should be a 0.  
	 				<em>Generally, obex objects have a single type argument</em>,  
	 				#A_GIMME, followed by a 0.

	@return 		This function returns the class pointer for the new object class. 
					<em>This pointer is used by numerous other functions and should be 
	 				stored in a global or static variable.</em>
*/
t_class *class_new(C74_CONST char *name, C74_CONST method mnew, C74_CONST method mfree, long size, C74_CONST method mmenu, short type, ...);
#endif


/**
	Frees a previously defined object class. <em>This function is not typically used by external developers.</em>

	@ingroup class
	@param 	c		The class pointer
	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
					or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err class_free(t_class *c);


#ifndef class_register
/**
	Registers a previously defined object class. This function is required, and should be called at the end of <tt>main()</tt>.

	@ingroup class

	@param 	name_space	The desired class's name space. Typically, either the 
	 					constant #CLASS_BOX, for obex classes which can 
	 					instantiate inside of a Max patcher (e.g. boxes, UI objects, 
	 					etc.), or the constant #CLASS_NOBOX, for classes 
	 					which will only be used internally. Developers can define 
	 					their own name spaces as well, but this functionality is 
	 					currently undocumented.
	@param 	c			The class pointer

	@return 			This function returns the error code #MAX_ERR_NONE if successful, 
						or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err class_register(t_symbol *name_space, t_class *c);
#endif


/**
	Registers an alias for a previously defined object class.

	@ingroup class
	@param 	c			The class pointer
	@param	aliasname	A symbol who's name will become an alias for the given class

	@return 			This function returns the error code #MAX_ERR_NONE if successful, 
						or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err class_alias(t_class *c, t_symbol *aliasname);

// function: class_copy
/**
 * Duplicates a previously registered object class, and registers a copy of this class.
 *
 * @ingroup classmod
 *
 * @param 	src_name_space	The source class's name space. 
 * @param 	src_classname	The source class's class name. 
 * @param 	dst_name_space	The copied class's name space. 
 * @param 	dst_classname	The copied class's class name. 
 *
 * @return 	This function returns the error code <tt>MAX_ERR_NONE</tt> if successful, 
 * 			or one of the other error codes defined in "ext_obex.h" if unsuccessful.
 *
 */
t_max_err class_copy(t_symbol *src_name_space, t_symbol *src_classname, t_symbol *dst_name_space, t_symbol *dst_classname);	


#ifndef class_addmethod
/**
	Adds a method to a previously defined object class. 
	
	@ingroup class
	
	@param 	c		The class pointer
	@param 	m		Function to be called when the method is invoked
	@param 	name	C-string defining the message (message selector)
	@param 	...		One or more integers specifying the arguments to the message, 
	 				in the standard Max type list format (see Chapter 3 of the 
	 				Writing Externals in Max document for more information).
	
	@return			This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
	
	@remark 		The class_addmethod() function works essentially like the 
	 				traditional addmess() function, adding the function pointed to  
	 				by <tt>m</tt>, to respond to the message string <tt>name</tt> in the   
	 				leftmost inlet of the object. 
*/
t_max_err class_addmethod(t_class *c, C74_CONST method m, C74_CONST char *name, ...);
#endif


/**
	Adds an attribute to a previously defined object class. 

	@ingroup class

	@param 	c		The class pointer
	@param 	attr	The attribute to add. The attribute will be a pointer returned 
	 				by attribute_new(), attr_offset_new() or 
	 				attr_offset_array_new().

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err class_addattr(t_class *c,t_object *attr);



/**
	Retrieves the name of a class, given the class's pointer.

	@ingroup class
	@param 	c		The class pointer
	@return 		If successful, this function returns the name of the class as a t_symbol *. 
*/
t_symbol *class_nameget(t_class *c);


/**
	Finds the class pointer for a class, given the class's namespace and name.

	@ingroup class

	@param 	name_space	The desired class's name space. Typically, either the 
	 					constant #CLASS_BOX, for obex classes which can 
	 					instantiate inside of a Max patcher (e.g. boxes, UI objects, 
	 					etc.), or the constant #CLASS_NOBOX, for classes 
	 					which will only be used internally. Developers can define 
	 					their own name spaces as well, but this functionality is 
	 					currently undocumented.
	@param 	classname	The name of the class to be looked up

	@return 			If successful, this function returns the class's data pointer. Otherwise, it returns NULL.
*/
t_class *class_findbyname(t_symbol *name_space, t_symbol *classname);


/**
	Finds the class pointer for a class, given the class's namespace and name.

	@ingroup class
	
	@param 	name_space	The desired class's name space. Typically, either the 
	 					constant #CLASS_BOX, for obex classes which can 
	 					instantiate inside of a Max patcher (e.g. boxes, UI objects, 
	 					etc.), or the constant #CLASS_NOBOX, for classes 
	 					which will only be used internally. Developers can define 
	 					their own name spaces as well, but this functionality is 
	 					currently undocumented.
	@param 	classname	The name of the class to be looked up (case free)

	@return 			If successful, this function returns the class's data pointer. Otherwise, it returns NULL.
*/
t_class *class_findbyname_casefree(t_symbol *name_space, t_symbol *classname);


/**
	Wraps user gettable attributes with a method that gets the values and sends out dumpout outlet.

	@ingroup class
	@param 	c		The class pointer
	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
					or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err class_dumpout_wrap(t_class *c);

t_class *class_getifloaded(t_symbol *name_space, t_symbol *classname);
t_class *class_getifloaded_casefree(t_symbol *name_space, t_symbol *classname);


/**
	Determines if a particular object is an instance of a given class.

	@ingroup obj

	@param 	x		The object to test
	@param 	name	The name of the class to test this object against
	@return 		This function returns 1 if the object is an instance of the named class. Otherwise, 0 is returned.
	@remark 		For instance, to determine whether an unknown object pointer is a pointer to a print object, one would call:

	@code
	long isprint = object_classname_compare(x, gensym("print"));
	@endcode
*/
long object_classname_compare(void *x, t_symbol *name);

t_hashtab *reg_object_namespace_lookup(t_symbol *name_space);
method class_method(t_class *x, t_symbol *methodname);
t_messlist *class_mess(t_class *x, t_symbol *methodname);
t_messlist *object_mess(t_object *x, t_symbol *methodname);
method class_attr_method(t_class *x, t_symbol *methodname, void **attr, long *get);
void *class_attr_get(t_class *x, t_symbol *attrname);
t_max_err class_extra_store(t_class *x,t_symbol *s,t_object *o);
t_max_err class_extra_storeflags(t_class *x,t_symbol *s,t_object *o,long flags);
void *class_extra_lookup(t_class *x,t_symbol *s);
t_max_err class_addtypedwrapper(t_class *x, method m, char *name, ...);
t_messlist *class_typedwrapper_get(t_class *x, t_symbol *s);
t_max_err object_addtypedwrapper(t_object *x, method m, char *name, ...);
t_messlist *object_typedwrapper_get(t_object *x, t_symbol *s);
t_hashtab *class_namespace_fromsym(t_symbol *name_space);
t_max_err class_namespace_getclassnames(t_symbol *name_space, long *kc, t_symbol ***kv);
t_max_err class_setpath(t_class *x, short vol);
short class_getpath(t_class *x);
 	
