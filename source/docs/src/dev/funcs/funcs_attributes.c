
/** Common attr struct. This struct is provided for debugging convenience, 
	but should be considered opaque and is subject to change without notice. 

	@ingroup attr
*/
typedef struct _attr
{
	t_object		ob;
	t_symbol		*name;
	t_symbol		*type;	
	long			flags;  		//public/private get/set methods
	method			get;    		//override default get method
	method			set;    		//override default set method
	void			*filterget;		//filterobject for get method
	void			*filterset; 	//filterobject for set method
	void			*reserved;		//for future use
} t_attr;		

/** macros for settings and getting attribute dirty flag */

#define ATTR_SET_DIRTY(x) (((t_attr *)(x))->flags |= ATTR_DIRTY)
#define ATTR_UNSET_DIRTY(x) (((t_attr *)(x))->flags &= ~ATTR_DIRTY)
#define ATTR_GET_DIRTY(x) (((t_attr *)(x))->flags & ATTR_DIRTY)


//constructors


/**
	Create a new attribute. The attribute will allocate memory and store its own data. Attributes created using attribute_new() can be assigned either to classes (using the class_addattr() function) or to objects (using the object_addattr() function).

	@ingroup attr

	@param 	name	A name for the attribute, as a C-string
	@param 	type	A t_symbol * representing a valid attribute type. 
					At the time of this writing, the valid type-symbols are: 
					<tt>_sym_char</tt> (char), 
					<tt>_sym_long</tt> (long), 
					<tt>_sym_float32</tt> (32-bit float), 
					<tt>_sym_float64</tt> (64-bit float), 
					<tt>_sym_atom</tt> (Max #t_atom pointer), 
					<tt>_sym_symbol</tt> (Max #t_symbol pointer), 
					<tt>_sym_pointer</tt> (generic pointer) and 
					<tt>_sym_object</tt> (Max #t_object pointer).
	@param 	flags	Any attribute flags, expressed as a bitfield. 
					Attribute flags are used to determine if an attribute is accessible for setting or querying. 
					The available accessor flags are defined in #e_max_attrflags.					
	@param 	mget	The method to use for the attribute's <tt>get</tt> functionality. If <tt>mget</tt> is NULL, the default method is used. 
	@param 	mset	The method to use for the attribute's <tt>set</tt> functionality. If <tt>mset</tt> is NULL, the default method is used.

	@return 		This function returns the new attribute's object pointer if successful, or NULL if unsuccessful.

	@remark 		Developers wishing to define custom methods for <tt>get</tt> or <tt>set</tt> functionality need to prototype them as:
	@code
	t_max_err myobject_myattr_get(t_myobject *x, void *attr, long *ac, t_atom **av);
	@endcode
	@code
	t_max_err myobject_myattr_set(t_myobject *x, void *attr, long ac, t_atom *av);
	@endcode

	@remark 		Implementation will vary, of course, but need to follow the following basic models. 
					Note that, as with custom <tt>getvalueof</tt> and <tt>setvalueof</tt> methods for the object, 
					assumptions are made throughout Max that getbytes() has been used for memory allocation. 
					Developers are strongly urged to do the same:
	@code
	t_max_err myobject_myattr_get(t_myobject *x, void *attr, long *ac, t_atom **av)
	{
		if (*ac && *av)
			// memory passed in; use it
		else {
			*ac = 1; // size of attr data
			*av = (t_atom *)getbytes(sizeof(t_atom) * (*ac));
			if (!(*av)) {
				*ac = 0;
				return MAX_ERR_OUT_OF_MEM;
			}
		}
		atom_setlong(*av, x->some_value);
		return MAX_ERR_NONE;
	}

	t_max_err myobject_myattr_set(t_myobject *x, void *attr, long ac, t_atom *av)
	{
		if (ac && av) {
			x->some_value = atom_getlong(av);
		}
		return MAX_ERR_NONE;
	}
	@endcode
*/
t_object *attribute_new(C74_CONST char *name, t_symbol *type, long flags, method mget, method mset);


/**
	Create a new attribute. The attribute references memory stored outside of itself, in the object's data structure. Attributes created using attr_offset_new() can be assigned either to classes (using the class_addattr() function) or to objects (using the object_addattr() function).

	@ingroup attr

	@param 	name	A name for the attribute, as a C-string
	@param 	type	A t_symbol * representing a valid attribute type. 
					At the time of this writing, the valid type-symbols are: 
					<tt>_sym_char</tt> (char), 
					<tt>_sym_long</tt> (long), 
					<tt>_sym_float32</tt> (32-bit float), 
					<tt>_sym_float64</tt> (64-bit float), 
					<tt>_sym_atom</tt> (Max #t_atom pointer), 
					<tt>_sym_symbol</tt> (Max #t_symbol pointer), 
					<tt>_sym_pointer</tt> (generic pointer) and 
					<tt>_sym_object</tt> (Max #t_object pointer).
	@param 	flags	Any attribute flags, expressed as a bitfield. 
					Attribute flags are used to determine if an attribute is accessible for setting or querying. 
					The available accessor flags are defined in #e_max_attrflags.					
	@param 	mget	The method to use for the attribute's <tt>get</tt> functionality. 
					If <tt>mget</tt> is NULL, the default method is used. See the discussion under attribute_new(), for more information.
	@param 	mset	The method to use for the attribute's <tt>set</tt> functionality. 
					If <tt>mset</tt> is NULL, the default method is used. See the discussion under attribute_new(), for more information.
	@param 	offset	Byte offset into the class data structure of the object which will "own" the attribute. 
					The offset should point to the data to be referenced by the attribute. 
					Typically, the #calcoffset macro (described above) is used to calculate this offset.

	@return 		This function returns the new attribute's object pointer if successful, or NULL if unsuccessful.

	@remark 		For instance, to create a new attribute which references the value of a double variable (<tt>val</tt>) in an object class's data structure:
	@code
	t_object *attr = attr_offset_new("myattr", _sym_float64 / * matches data size * /, 0 / * no flags * /, (method)0L, (method)0L, calcoffset(t_myobject, val));
	@endcode
*/
t_object *attr_offset_new(C74_CONST char *name, C74_CONST t_symbol *type, long flags, C74_CONST method mget, C74_CONST method mset, long offset);


/**
	Create a new attribute. The attribute references an array of memory stored outside of itself, in the object's data structure. Attributes created using attr_offset_array_new() can be assigned either to classes (using the class_addattr() function) or to objects (using the object_addattr() function).

	@ingroup attr

	@param 	name		A name for the attribute, as a C-string
	@param 	type		A t_symbol * representing a valid attribute type. 
						At the time of this writing, the valid type-symbols are: 
						<tt>_sym_char</tt> (char), 
						<tt>_sym_long</tt> (long), 
						<tt>_sym_float32</tt> (32-bit float), 
						<tt>_sym_float64</tt> (64-bit float), 
						<tt>_sym_atom</tt> (Max #t_atom pointer), 
						<tt>_sym_symbol</tt> (Max #t_symbol pointer), 
						<tt>_sym_pointer</tt> (generic pointer) and 
						<tt>_sym_object</tt> (Max #t_object pointer).
	@param	size		Maximum number of items that may be in the array.
	@param 	flags		Any attribute flags, expressed as a bitfield. 
						Attribute flags are used to determine if an attribute is accessible for setting or querying. 
						The available accessor flags are defined in #e_max_attrflags.					
	@param 	mget		The method to use for the attribute's <tt>get</tt> functionality. 
						If <tt>mget</tt> is NULL, the default method is used. See the discussion under attribute_new(), for more information.
	@param 	mset		The method to use for the attribute's <tt>set</tt> functionality. 
						If <tt>mset</tt> is NULL, the default method is used. See the discussion under attribute_new(), for more information.
	@param 	offsetcount	Byte offset into the object class's data structure of a long variable describing how many array elements 
						(up to <tt>size</tt>) comprise the data to be referenced by the attribute. 
						Typically, the #calcoffset macro is used to calculate this offset.
	@param 	offset		Byte offset into the class data structure of the object which will "own" the attribute. 
						The offset should point to the data to be referenced by the attribute. 
						Typically, the #calcoffset macro is used to calculate this offset.

	@return 			This function returns the new attribute's object pointer if successful, or NULL if unsuccessful.

	@remark 			For instance, to create a new attribute which references an array of 10 t_atoms (<tt>atm</tt>; 
						the current number of "active" elements in the array is held in the variable <tt>atmcount</tt>) in an object class's data structure:
	@code
	t_object *attr = attr_offset_array_new("myattrarray", _sym_atom / * matches data size * /, 10 / * max * /, 0 / * no flags * /, (method)0L, (method)0L, calcoffset(t_myobject, atmcount) / * count * /, calcoffset(t_myobject, atm) / * data * /);
	@endcode
*/
t_object *attr_offset_array_new(C74_CONST char *name, t_symbol *type, long size, long flags, method mget, method mset, long offsetcount, long offset);


t_object *attr_filter_clip_new(void);


t_object *attr_filter_proc_new(method proc);


//for easy access of simple attributes

/**
	Retrieves the value of an attribute, given its parent object and name. 

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name

	@return 		This function returns the value of the specified attribute, if successful, or 0, if unsuccessful.

	@remark 		If the attribute is not of the type specified by the function, the 
	 				function will attempt to coerce a valid value from the attribute.
*/
t_atom_long object_attr_getlong(void *x, t_symbol *s);


/**
	Sets the value of an attribute, given its parent object and name. The function will call the attribute's <tt>set</tt> method, using the data provided.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	c		An integer value; the new value for the attribute

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_attr_setlong(void *x, t_symbol *s, t_atom_long c);


/**
	Retrieves the value of an attribute, given its parent object and name. 

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name

	@return 		This function returns the value of the specified attribute, if successful, or 0, if unsuccessful.

	@remark 		If the attribute is not of the type specified by the function, the 
	 				function will attempt to coerce a valid value from the attribute.
*/
t_atom_float object_attr_getfloat(void *x, t_symbol *s);


/**
	Sets the value of an attribute, given its parent object and name. The function will call the attribute's <tt>set</tt> method, using the data provided.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	c		An floating point value; the new value for the attribute

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_attr_setfloat(void *x, t_symbol *s, t_atom_float c);


/**
	Retrieves the value of an attribute, given its parent object and name. 

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name

	@return 		This function returns the value of the specified attribute, if successful, or the empty symbol (equivalent to <tt>gensym("")</tt> or <tt>_sym_nothing</tt>), if unsuccessful.
*/
t_symbol *object_attr_getsym(void *x, t_symbol *s);


/**
	Sets the value of an attribute, given its parent object and name. The function will call the attribute's <tt>set</tt> method, using the data provided.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	c		A t_symbol *; the new value for the attribute

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_attr_setsym(void *x, t_symbol *s, t_symbol *c);


char object_attr_getchar(void *x, t_symbol *s);
t_max_err object_attr_setchar(void *x, t_symbol *s, char c);
t_object* object_attr_getobj(void *x, t_symbol *s);
t_max_err object_attr_setobj(void *x, t_symbol *s, t_object *o); 


/**
	Retrieves the value of an attribute, given its parent object and name. 
	This function uses a developer-allocated array to copy data to. 
	Developers wishing to retrieve the value of an attribute without pre-allocating memory should refer to the object_attr_getvalueof() function.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	max		The number of array elements in <tt>vals</tt>. The function will take care not to overwrite the bounds of the array.
	@param 	vals	Pointer to the first element of a pre-allocated array of long data.

	@return 		This function returns the number of elements copied into <tt>vals</tt>.

	@remark 		If the attribute is not of the type specified by the function, the 
	 				function will attempt to coerce a valid value from the attribute.
*/
long object_attr_getlong_array(void *x, t_symbol *s, long max, t_atom_long *vals);


/**
	Sets the value of an attribute, given its parent object and name. The function will call the attribute's <tt>set</tt> method, using the data provided.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	count	The number of array elements in vals
	@param 	vals	Pointer to the first element of an array of long data

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_attr_setlong_array(void *x, t_symbol *s, long count, t_atom_long *vals);


/**
	Retrieves the value of an attribute, given its parent object and name. 
	This function uses a developer-allocated array to copy data to. 
	Developers wishing to retrieve the value of an attribute without pre-allocating memory should refer to the object_attr_getvalueof() function.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	max		The number of array elements in <tt>vals</tt>. The function will take care not to overwrite the bounds of the array.
	@param 	vals	Pointer to the first element of a pre-allocated array of unsigned char data.

	@return 		This function returns the number of elements copied into <tt>vals</tt>.

	@remark 		If the attribute is not of the type specified by the function, the 
	 				function will attempt to coerce a valid value from the attribute.
*/
long object_attr_getchar_array(void *x, t_symbol *s, long max, t_uint8 *vals);


/**
	Sets the value of an attribute, given its parent object and name. The function will call the attribute's <tt>set</tt> method, using the data provided.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	count	The number of array elements in vals
	@param 	vals	Pointer to the first element of an array of unsigned char data

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_attr_setchar_array(void *x, t_symbol *s, long count, C74_CONST t_uint8 *vals);


/**
	Retrieves the value of an attribute, given its parent object and name. 
	This function uses a developer-allocated array to copy data to. 
	Developers wishing to retrieve the value of an attribute without pre-allocating memory should refer to the object_attr_getvalueof() function.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	max		The number of array elements in <tt>vals</tt>. The function will take care not to overwrite the bounds of the array.
	@param 	vals	Pointer to the first element of a pre-allocated array of float data.

	@return 		This function returns the number of elements copied into <tt>vals</tt>.

	@remark 		If the attribute is not of the type specified by the function, the 
	 				function will attempt to coerce a valid value from the attribute.
*/
long object_attr_getfloat_array(void *x, t_symbol *s, long max, float *vals);


/**
	Sets the value of an attribute, given its parent object and name. The function will call the attribute's <tt>set</tt> method, using the data provided.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	count	The number of array elements in vals
	@param 	vals	Pointer to the first element of an array of float data

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_attr_setfloat_array(void *x, t_symbol *s, long count, float *vals);


/**
	Retrieves the value of an attribute, given its parent object and name. 
	This function uses a developer-allocated array to copy data to. 
	Developers wishing to retrieve the value of an attribute without pre-allocating memory should refer to the object_attr_getvalueof() function.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	max		The number of array elements in <tt>vals</tt>. The function will take care not to overwrite the bounds of the array.
	@param 	vals	Pointer to the first element of a pre-allocated array of double data.

	@return 		This function returns the number of elements copied into <tt>vals</tt>.

	@remark 		If the attribute is not of the type specified by the function, the 
	 				function will attempt to coerce a valid value from the attribute.
*/
long object_attr_getdouble_array(void *x, t_symbol *s, long max, double *vals);


/**
	Sets the value of an attribute, given its parent object and name. The function will call the attribute's <tt>set</tt> method, using the data provided.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	count	The number of array elements in vals
	@param 	vals	Pointer to the first element of an array of double data

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_attr_setdouble_array(void *x, t_symbol *s, long count, double *vals);


/**
	Retrieves the value of an attribute, given its parent object and name. 
	This function uses a developer-allocated array to copy data to. 
	Developers wishing to retrieve the value of an attribute without pre-allocating memory should refer to the object_attr_getvalueof() function.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	max		The number of array elements in <tt>vals</tt>. The function will take care not to overwrite the bounds of the array.
	@param 	vals	Pointer to the first element of a pre-allocated array of #t_symbol *s.

	@return 		This function returns the number of elements copied into <tt>vals</tt>.
*/
long object_attr_getsym_array(void *x, t_symbol *s, long max, t_symbol **vals);


/**
	Sets the value of an attribute, given its parent object and name. 
	The function will call the attribute's <tt>set</tt> method, using the data provided.

	@ingroup attr

	@param 	x		The attribute's parent object
	@param 	s		The attribute's name
	@param 	count	The number of array elements in vals
	@param 	vals	Pointer to the first element of an array of #t_symbol *s

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err object_attr_setsym_array(void *x, t_symbol *s, long count, t_symbol **vals);


//attr filters util

/**
	Attaches a clip filter to an attribute. 
	The filter will <em>only</em> clip values sent to the attribute using the attribute's <tt>set</tt> function.

	@ingroup attr

	@param 	x		Pointer to the attribute to receive the filter
	@param 	min		Minimum value for the clip filter
	@param 	max		Maximum value for the clip filter 
	@param 	usemin	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.
	@param 	usemax	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err attr_addfilterset_clip(void *x, double min, double max, long usemin, long usemax);


/**
	Attaches a clip/scale filter to an attribute. 
	The filter will <em>only</em> clip and scale values sent to the attribute using the attribute's <tt>set</tt> function.

	@ingroup attr

	@param 	x		Pointer to the attribute to receive the filter
	@param 	scale	Scale value. Data sent to the attribute will be scaled by this amount. <em>Scaling occurs previous to clipping</em>.
	@param 	min		Minimum value for the clip filter
	@param 	max		Maximum value for the clip filter 
	@param 	usemin	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.
	@param 	usemax	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err attr_addfilterset_clip_scale(void *x, double scale, double min, double max, long usemin, long usemax);


/**
	Attaches a clip filter to an attribute. 
	The filter will <em>only</em> clip values retrieved from the attribute using the attribute's <tt>get</tt> function.

	@ingroup attr

	@param 	x		Pointer to the attribute to receive the filter
	@param 	min		Minimum value for the clip filter
	@param 	max		Maximum value for the clip filter 
	@param 	usemin	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.
	@param 	usemax	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err attr_addfilterget_clip(void *x, double min, double max, long usemin, long usemax);


/**
	Attaches a clip/scale filter to an attribute. 
	The filter will <em>only</em> clip and scale values retrieved from the attribute using the attribute's <tt>get</tt> function.

	@ingroup attr

	@param 	x		Pointer to the attribute to receive the filter
	@param 	scale	Scale value. Data retrieved from the attribute will be scaled by this amount. <em>Scaling occurs previous to clipping</em>.
	@param 	min		Minimum value for the clip filter
	@param 	max		Maximum value for the clip filter 
	@param 	usemin	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.
	@param 	usemax	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err attr_addfilterget_clip_scale(void *x, double scale, double min, double max, long usemin, long usemax);


/**
	Attaches a clip filter to an attribute. 
	The filter will clip any values sent to or retrieved from the attribute using the attribute's <tt>get</tt> and <tt>set</tt> functions.

	@ingroup attr

	@param 	x		Pointer to the attribute to receive the filter
	@param 	min		Minimum value for the clip filter
	@param 	max		Maximum value for the clip filter 
	@param 	usemin	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.
	@param 	usemax	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err attr_addfilter_clip(void *x, double min, double max, long usemin, long usemax);


/**
	Attaches a clip/scale filter to an attribute. 
	The filter will clip and scale any values sent to or retrieved from the attribute using the attribute's <tt>get</tt> and <tt>set</tt> functions.

	@ingroup attr

	@param 	x		Pointer to the attribute to receive the filter
	@param 	scale	Scale value. Data sent to the attribute will be scaled by this amount. Data retrieved from the attribute will be scaled by its reciprocal. 
					<em>Scaling occurs previous to clipping</em>.
	@param 	min		Minimum value for the clip filter
	@param 	max		Maximum value for the clip filter 
	@param 	usemin	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.
	@param 	usemax	Sets this value to 0 if the minimum clip value should <em>not</em> be used. Otherwise, set the value to non-zero.

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err attr_addfilter_clip_scale(void *x, double scale, double min, double max, long usemin, long usemax);


/**
	Attaches a custom filter method to an attribute. 
	The filter will <em>only</em> be called for values retrieved from the attribute using the attribute's <tt>set</tt> function.

	@ingroup attr

	@param 	x		Pointer to the attribute to receive the filter
	@param 	proc	A filter method

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

	@remark 		The filter method should be prototyped and implemented as follows:
	@code
	t_max_err myfiltermethod(void *parent, void *attr, long ac, t_atom *av);

	t_max_err myfiltermethod(void *parent, void *attr, long ac, t_atom *av)
	{
		long i;
		float temp,

		// this filter rounds off all values
		// assumes that the data is float 
		for (i = 0; i < ac; i++) {
			temp = atom_getfloat(av + i);
			temp = (float)((long)(temp + 0.5));
			atom_setfloat(av + i, temp);
		}
		return MAX_ERR_NONE;
	}
	@endcode
*/
t_max_err attr_addfilterset_proc(void *x, method proc);


/**
	Attaches a custom filter method to an attribute. The filter will <em>only</em> be called for values retrieved from the attribute using the attribute's <tt>get</tt> function.

	@ingroup attr

	@param 	x		Pointer to the attribute to receive the filter
	@param 	proc	A filter method

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.

	@remark 		The filter method should be prototyped and implemented as described above for the attr_addfilterset_proc() function.
*/
t_max_err attr_addfilterget_proc(void *x, method proc);




/**
	Create a dictionary of attribute-name, attribute-value pairs 
	from an array of atoms containing an attribute definition list.
	
	@ingroup attr
	@param	x	A dictionary instance pointer.
	@param	ac	The number of atoms to parse in av.
	@param	av	A pointer to the first of the array of atoms containing the attribute values.

	@remark		The code example below shows the creation of a list of atoms using atom_setparse(),
				and then uses that list of atoms to fill the dictionary with attr_args_dictionary().
	@code
	long ac = 0;
	t_atom *av = NULL;
	char parsebuf[4096];
	t_dictionary *d = dictionary_new();
	t_atom a;
	
	sprintf(parsebuf,"@defrect %.6f %.6f %.6f %.6f @title Untitled @presentation 0 ", r->x, r->y, r->width, r->height);
	atom_setparse(&ac, &av, parsebuf);
	attr_args_dictionary(d, ac, av);
	atom_setobj(&a, d);
	@endcode
*/
void attr_args_dictionary(t_dictionary *x, short ac, t_atom *av);


/**
	Set attributes for an object that are defined in a dictionary.
	Objects with dictionary constructors, such as UI objects, 
	should call this method to set their attributes when an object is created.
	
	@ingroup attr
	@param	x	The object instance pointer.
	@param	d	The dictionary containing the attributes.
	@see	attr_args_process()
*/
void attr_dictionary_process(void *x, t_dictionary *d);

/**
	Check that a dictionary only contains values for existing attributes 
	of an object. If a key in the dictionary doesn't correspond an one of 
	the object's attributes, an error will be posted to the Max window.
	
	@ingroup attr
	@param	x	The object instance pointer.
	@param	d	The dictionary containing the attributes.
	@see	attr_dictionary_process()
*/
void attr_dictionary_check(void *x, t_dictionary *d);




/**
	Mark an attribute as being touched by some code not from the attribute setter.
	This will notify clients that the attribute has changed.

	@ingroup obj

	@param 	x			The object whose attribute has been changed 
	@param 	attrname	The attribute name

	@return				A Max error code 
 */				
t_max_err object_attr_touch(t_object *x, t_symbol *attrname);

/**
	Mark one or more attributes as being touched by some code not from the attribute setter.
	This will notify clients that the attributes have changed. Utility to call object_attr_touch()
	for several attributes

	@ingroup obj

	@param 	x			The object whose attribute has been changed 
	@param 	attrnames	The attribute names as a space separated string

	@return				A Max error code 
 */				
t_max_err object_attr_touch_parse(t_object *x, char *attrnames);


t_max_err object_attr_getvalueof_ext(void *x, t_symbol *s, long *argc, t_atom **argv);
t_max_err object_attr_setvalueof_ext(void *x, t_symbol *s, long argc, t_atom *argv);
long object_attr_getdirty(t_object *x, t_symbol *attrname);



//attr functions

/**
	Determines the point in an atom list where attribute arguments begin. 
	Developers can use this function to assist in the manual processing of attribute arguments, when attr_args_process() 
	doesn't provide the correct functionality for a particular purpose.

	@ingroup attr

	@param 	ac		The count of t_atoms in <tt>av</tt>
	@param 	av		An atom list

	@return 		This function returns an offset into the atom list, where the first attribute argument occurs. 
					For instance, the atom list <tt>foo bar 3.0 \@mode 6</tt> would cause <tt>attr_args_offset</tt> to return 3 
					(the attribute <tt>mode</tt> appears at position 3 in the atom list).
*/
long attr_args_offset(short ac, t_atom *av);


/**
	Takes an atom list and properly set any attributes described within. This function is typically used in an object's <tt>new</tt> method to conveniently process attribute arguments.

	@ingroup attr

	@param 	x		The object whose attributes will be processed
	@param 	ac		The count of t_atoms in <tt>av</tt>
	@param 	av		An atom list

	@remark 		Here is a typical example of usage:
	@code
	void *myobject_new(t_symbol *s, long ac, t_atom *av)
	{
		t_myobject *x = NULL;

		if (x=(t_myobject *)object_alloc(myobject_class))
		{
			// initialize any data before processing
			// attributes to avoid overwriting 
			// attribute argument-set values
			x->data = 0; 

			// process attr args, if any
			attr_args_process(x, ac, av);
		}
		return x;
	}
	@endcode
*/
void attr_args_process(void *x, short ac, t_atom *av);



