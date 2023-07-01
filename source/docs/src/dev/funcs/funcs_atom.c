// methods_atom.c

long atom_arg_getdouble(double *c, long idx, long ac, const t_atom *av);
long atom_arg_getfloat(float *c, long idx, long ac, const t_atom *av);
long atom_arg_getsym(t_symbol **c, long idx, long ac, const t_atom *av);
long atom_getcharfix(const t_atom *a);
long atom_gettype(const t_atom *a);
t_atom_float atom_getfloat(const t_atom *a);
t_atom_long atom_getlong(const t_atom *a);
t_max_err atom_arg_getlong(t_atom_long *c, long idx, long ac, const t_atom *av);
t_max_err atom_setfloat(t_atom *a, double b);
t_max_err atom_setlong(t_atom *a, t_atom_long b);
t_max_err atom_setobj(t_atom *a, void *b);
t_max_err atom_setsym(t_atom *a, t_symbol *b);
t_symbol *atom_getsym(const t_atom *a);
void *atom_getobj(const t_atom *a);


//atom functions 

#ifndef atom_setlong
/**
	Inserts an integer into a #t_atom and change the t_atom's type to #A_LONG. 

	@ingroup atom

	@param 	a		Pointer to a #t_atom whose value and type will be modified
	@param 	b		Integer value to copy into the #t_atom

	@return 	This function returns the error code #MAX_ERR_NONE if successful, 
	 			or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err atom_setlong(t_atom *a, t_atom_long b);
#endif


#ifndef atom_setfloat
/**
	Inserts a floating point number into a #t_atom and change the t_atom's type to #A_FLOAT. 

	@ingroup atom

	@param 	a		Pointer to a #t_atom whose value and type will be modified
	@param 	b		Floating point value to copy into the #t_atom

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err atom_setfloat(t_atom *a, double b);
#endif


#ifndef atom_setsym
/**
	Inserts a #t_symbol * into a #t_atom and change the t_atom's type to #A_SYM. 

	@ingroup atom

	@param 	a		Pointer to a #t_atom whose value and type will be modified
	@param 	b		Pointer to a #t_symbol to copy into the #t_atom

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err atom_setsym(t_atom *a, t_symbol *b);				
#endif


/**
	Inserts a generic pointer value into a #t_atom and change the t_atom's type to #A_OBJ. 

	@ingroup atom

	@param 	a		Pointer to a #t_atom whose value and type will be modified
	@param 	b		Pointer value to copy into the #t_atom
	
	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err atom_setobj(t_atom *a, void *b);


#ifndef atom_getlong
/**
	Retrieves a long integer value from a #t_atom. 

	@ingroup atom

	@param 	a		Pointer to a #t_atom whose value is of interest
	@return 		This function returns the value of the specified #t_atom as an integer, if possible. Otherwise, it returns 0. 
	@remark 		If the #t_atom is not of the type specified by the function, the function will attempt to coerce a valid value from the t_atom. 
					For instance, if the t_atom <tt>at</tt> is set to type #A_FLOAT with a value of <tt>3.7</tt>, 
					the atom_getlong() function will return the truncated integer value of <tt>at</tt>, or <tt>3</tt>. 
					An attempt is also made to coerce #t_symbol data.
*/
t_atom_long atom_getlong(const t_atom *a);
#endif


#ifndef atom_getfloat
/**
	Retrieves a floating point value from a #t_atom. 

	@ingroup atom
	@param 	a		Pointer to a #t_atom whose value is of interest
	@return 		This function returns the value of the specified #t_atom as a floating point number, if possible. Otherwise, it returns 0. 

	@remark 		If the #t_atom is not of the type specified by the function, the function will attempt to coerce a valid value from the t_atom. 
					For instance, if the t_atom <tt>at</tt> is set to type #A_LONG with a value of <tt>5</tt>, 
					the atom_getfloat() function will return the value of <tt>at</tt> as a float, or <tt>5.0</tt>. 
					An attempt is also made to coerce #t_symbol data.
*/
t_atom_float atom_getfloat(const t_atom *a);
#endif


#ifndef atom_getsym
/**
	Retrieves a t_symbol * value from a t_atom. 

	@ingroup atom
	@param 	a		Pointer to a t_atom whose value is of interest
	@return 		This function returns the value of the specified #A_SYM-typed #t_atom, if possible. 
					Otherwise, it returns an empty, but valid, #t_symbol *, equivalent to <tt>gensym("")</tt>, or <tt>_sym_nothing</tt>. 

	@remark 		No attempt is made to coerce non-matching data types. 
*/
t_symbol *atom_getsym(const t_atom *a);
#endif


/**
	Retrieves a generic pointer value from a #t_atom. 

	@ingroup atom
	@param 	a		Pointer to a #t_atom whose value is of interest
	@return 		This function returns the value of the specified #A_OBJ-typed t_atom, if possible. Otherwise, it returns NULL. 
*/
void *atom_getobj(const t_atom *a);


/**
	Retrieves an unsigned integer value between 0 and 255 from a t_atom. 

	@ingroup atom
	@param 	a		Pointer to a #t_atom whose value is of interest
	@return 		This function returns the value of the specified #t_atom as an integer between 0 and 255, if possible. Otherwise, it returns 0. 

	@remark 		If the #t_atom is typed #A_LONG, but the data falls outside of the range 0-255, the data is truncated to that range before output.

	@remark 		If the t_atom is typed #A_FLOAT, the floating point value is multiplied by 255. and truncated to the range 0-255 before output. 
					For example, the floating point value <tt>0.5</tt> would be output from atom_getcharfix as <tt>127</tt> (0.5 * 255. = 127.5). 

	@remark 		No attempt is also made to coerce #t_symbol data. 
*/
long atom_getcharfix(const t_atom *a);


#ifndef atom_gettype
/**
	Retrieves type from a #t_atom. 

	@ingroup atom
	@param 	a		Pointer to a #t_atom whose type is of interest
	@return 		This function returns the type of the specified t_atom as defined in #e_max_atomtypes
*/
long atom_gettype(const t_atom *a);
#endif


//the following are useful for setting the values _only_ if there is an arg
//rather than setting it to 0 or _sym_nothing

/**
	Retrieves the integer value of a particular t_atom from an atom list, if the atom exists.

	@ingroup atom

	@param 	c		Pointer to a long variable to receive the atom's data if the function is successful.
	@param 	idx		Offset into the atom list of the atom of interest, starting from 0. 
					For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
	@param 	ac		Count of av.
	@param 	av		Pointer to the first t_atom of an atom list.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

	@remark 		The atom_arg_getlong() function only changes the value of <tt>c</tt> if the function is successful. 
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

	@param 	c		Pointer to a float variable to receive the atom's data if the function is successful. Otherwise, the value is left unchanged.
	@param 	idx		Offset into the atom list of the atom of interest, starting from 0. 
					For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
	@param 	ac		Count of av.
	@param 	av		Pointer to the first t_atom of an atom list.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
long atom_arg_getfloat(float *c, long idx, long ac, const t_atom *av);


/**
	Retrieves the floating point value, as a double, of a particular t_atom from an atom list, if the atom exists.

	@ingroup atom

	@param 	c		Pointer to a double variable to receive the atom's data if the function is successful. Otherwise the value is left unchanged.
	@param 	idx		Offset into the atom list of the atom of interest, starting from 0. 
					For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
	@param 	ac		Count of av.
	@param 	av		Pointer to the first t_atom of an atom list.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
long atom_arg_getdouble(double *c, long idx, long ac, const t_atom *av);


/**
	Retrieves the t_symbol * value of a particular t_atom from an atom list, if the atom exists.

	@ingroup atom

	@param 	c		Pointer to a t_symbol * variable to receive the atom's data if the function is successful. Otherwise, the value is left unchanged.
	@param 	idx		Offset into the atom list of the atom of interest, starting from 0. 
					For instance, if you want data from the 3rd atom in the atom list, <tt>idx</tt> should be set to 2.
	@param 	ac		Count of av.
	@param 	av		Pointer to the first t_atom of an atom list.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

	@remark 		The atom_arg_getsym() function only changes the value of <tt>c</tt> if the function is successful. 
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

