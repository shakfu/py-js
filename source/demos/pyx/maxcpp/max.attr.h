
#ifndef MAXMSP_CPP_6_ATTR_H
#define MAXMSP_CPP_6_ATTR_H

// macros for attributes
// class attributes are almost universally attr_offset

/**
	Specify custom accessor methods for an attribute.  
	If you specify a non-NULL value for the setter or getter, 
	then the function you specify will be called to set or get the attribute's value
	rather than using the built-in accessor.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	getter			An appropriate getter method as discussed in @ref attribute_accessors, 
							or NULL to use the default getter.
	@param	setter			An appropriate setter method as discussed in @ref attribute_accessors, 
							or NULL to use the default setter.
*/
#define ATTR_ACCESSORS(objclass,attrname,getter,setter) \
	{ t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); \
		object_method(theattr,gensym("setmethod"),USESYM(get),TO_METHOD_GET(objclass,getter)); \
		object_method(theattr,gensym("setmethod"),USESYM(set),TO_METHOD_SET(objclass,setter)); }

#define ATTR_GET_ACCESSOR(objclass,attrname,getter) \
	{ t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); \
		object_method(theattr,gensym("setmethod"),USESYM(get),TO_METHOD_GET(objclass,getter)); }

#define ATTR_SET_ACCESSOR(objclass,attrname,setter) \
	{ t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); \
		object_method(theattr,gensym("setmethod"),USESYM(set),TO_METHOD_SET(objclass,setter)); }
		
/**
	Create a char attribute and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
*/
#define ATTR_CHAR(objclass,attrname,structmember) \
	class_addattr((objclass::m_class),attr_offset_new(attrname,USESYM(char),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,structmember)))


/**
	Create a t_atom_long integer attribute and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
*/

#define ATTR_LONG(objclass,attrname,structmember) \
		{		\
			C74_STATIC_ASSERT(structmembersize(objclass,structmember)==sizeof(long), "structmember must be long type"); \
			class_addattr((objclass::m_class),attr_offset_new(attrname,USESYM(long),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,structmember))); \
		}

/**
	Create a t_atom_long integer attribute and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
*/

#define ATTR_ATOM_LONG(objclass,attrname,structmember) \
		{		\
			C74_STATIC_ASSERT(structmembersize(objclass,structmember)==sizeof(t_atom_long), "structmember must be t_atom_long type"); \
			class_addattr((objclass::m_class),attr_offset_new(attrname,USESYM(atom_long),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,structmember))); \
		}

/**
	Create a t_int32 integer attribute and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
*/

#define ATTR_INT32(objclass,attrname,structmember) \
		{		\
			C74_STATIC_ASSERT(structmembersize(objclass,structmember)==sizeof(t_int32), "structmember must be t_int32 type"); \
			class_addattr((objclass::m_class),attr_offset_new(attrname,USESYM(int32),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,structmember))); \
		}

#define ATTR_FILEPATH(objclass,attrname,structmember) \
		{		\
			C74_STATIC_ASSERT(structmembersize(objclass,structmember)==sizeof(t_filepath), "structmember must be t_filepath type"); \
			class_addattr((objclass::m_class),attr_offset_new(attrname,USESYM(filepath),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,structmember))); \
		}

/**
	Create a 32-bit float attribute and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
*/
#define ATTR_FLOAT(objclass,attrname,structmember) \
	class_addattr((objclass::m_class),attr_offset_new(attrname,USESYM(float32),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,structmember)))


/**
	Create a 64-bit float attribute and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
*/
#define ATTR_DOUBLE(objclass,attrname,structmember) \
	class_addattr((objclass::m_class),attr_offset_new(attrname,USESYM(float64),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,structmember)))


/**
	Create a #t_symbol* attribute and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
*/
#define ATTR_SYM(objclass,attrname,structmember) \
	class_addattr((objclass::m_class),attr_offset_new(attrname,USESYM(symbol),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,structmember)))


/**
	Create a #t_atom attribute and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
*/
#define ATTR_ATOM(objclass,attrname,structmember) \
	class_addattr((objclass::m_class),attr_offset_new(attrname,USESYM(atom),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,structmember)))


/**
	Create a #t_object* attribute and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
*/
#define ATTR_OBJ(objclass,attrname,structmember) \
	class_addattr((objclass::m_class),attr_offset_new(attrname,USESYM(object),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,structmember)))


/**
	Create a color (#t_jrgba) attribute and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
*/
#define ATTR_RGBA(objclass,attrname,structmember) \
	{	CLASS_ATTR_DOUBLE_ARRAY(objclass::m_class,attrname,/*flags*/0,objclass,structmember,4); \
		CLASS_ATTR_ACCESSORS(objclass::m_class,attrname,NULL,jgraphics_attr_setrgba); \
		CLASS_ATTR_PAINT(objclass::m_class,attrname,0); }


/**
	Create an array-of-chars attribute of fixed length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	size			The number of chars in the array.
*/
#define ATTR_CHAR_ARRAY(objclass,attrname,structmember,size) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(char),(size),/*flags*/0,(method)0L,(method)0L,0/*fix*/,calcoffset(objclass,structmember)))


/**
	Create an array-of-long-integers attribute of fixed length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	size			The number of longs in the array.
*/
#define ATTR_LONG_ARRAY(objclass,attrname,structmember,size) \
		{		\
			C74_STATIC_ASSERT(structmembersize(objclass,structmember[0])==sizeof(long), "structmember must be long type"); \
			class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(long),(size),/*flags*/0,(method)0L,(method)0L,0/*fix*/,calcoffset(objclass,structmember))); \
		}

/**
	Create an array-of-t_atom_long-integers attribute of fixed length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	size			The number of longs in the array.
*/
#define ATTR_ATOM_LONG_ARRAY(objclass,attrname,structmember,size) \
		{		\
			C74_STATIC_ASSERT(structmembersize(objclass,structmember[0])==sizeof(t_atom_long), "structmember must be t_atom_long type"); \
			class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(atom_long),(size),/*flags*/0,(method)0L,(method)0L,0/*fix*/,calcoffset(objclass,structmember))); \
		}


/**
	Create an array-of-32bit-floats attribute of fixed length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	size			The number of floats in the array.
*/
#define ATTR_FLOAT_ARRAY(objclass,attrname,structmember,size) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(float32),(size),/*flags*/0,(method)0L,(method)0L,0/*fix*/,calcoffset(objclass,structmember)))


/**
	Create an array-of-64bit-floats attribute of fixed length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	size			The number of doubles in the array.
*/
#define ATTR_DOUBLE_ARRAY(objclass,attrname,structmember,size) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(float64),(size),/*flags*/0,(method)0L,(method)0L,0/*fix*/,calcoffset(objclass,structmember)))


/**
	Create an array-of-symbols attribute of fixed length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	size			The number of items in the #t_symbol* array.
*/
#define ATTR_SYM_ARRAY(objclass,attrname,structmember,size) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(symbol),(size),/*flags*/0,(method)0L,(method)0L,0/*fix*/,calcoffset(objclass,structmember)))


/**
	Create an array-of-atoms attribute of fixed length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	size			The number of items in the #t_atom array.
*/
#define ATTR_ATOM_ARRAY(objclass,attrname,structmember,size) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(atom),(size),/*flags*/0,(method)0L,(method)0L,0/*fix*/,calcoffset(objclass,structmember)))


/**
	Create an array-of-objects attribute of fixed length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	size			The number of items in the #t_object* array.
*/
#define ATTR_OBJ_ARRAY(objclass,attrname,structmember,size) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(object),(size),/*flags*/0,(method)0L,(method)0L,0/*fix*/,calcoffset(objclass,structmember)))




/**
	Create an array-of-chars attribute of variable length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	sizemember		The actual number of items in the char array at any given moment.
	@param	maxsize			The maximum number of items in the char array, i.e. the number of members allocated for the array in the struct.
*/
#define ATTR_CHAR_VARSIZE(objclass,attrname,structmember,sizemember,maxsize) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(char),(maxsize),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,sizemember),calcoffset(objclass,structmember)))


/**
	Create an array-of-long-integers attribute of variable length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	sizemember		The actual number of items in the long array at any given moment.
	@param	maxsize			The maximum number of items in the long array, i.e. the number of members allocated for the array in the struct.
*/
#define ATTR_LONG_VARSIZE(objclass,attrname,structmember,sizemember,maxsize) \
		{		\
			C74_STATIC_ASSERT(structmembersize(objclass,structmember[0])==sizeof(long), "structmember must be t_atom_long type"); \
			class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(long),(maxsize),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,sizemember),calcoffset(objclass,structmember))); \
		}

/**
	Create an array-of-32bit-floats attribute of variable length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	sizemember		The actual number of items in the float array at any given moment.
	@param	maxsize			The maximum number of items in the float array, i.e. the number of members allocated for the array in the struct.
*/
#define ATTR_FLOAT_VARSIZE(objclass,attrname,structmember,sizemember,maxsize) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(float32),(maxsize),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,sizemember),calcoffset(objclass,structmember)))


/**
	Create an array-of-64bit-floats attribute of variable length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	sizemember		The actual number of items in the double array at any given moment.
	@param	maxsize			The maximum number of items in the double array, i.e. the number of members allocated for the array in the struct.
*/
#define ATTR_DOUBLE_VARSIZE(objclass,attrname,structmember,sizemember,maxsize) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(float64),(maxsize),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,sizemember),calcoffset(objclass,structmember)))


/**
	Create an array-of-symbols attribute of variable length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	sizemember		The actual number of items in the #t_symbol* array at any given moment.
	@param	maxsize			The maximum number of items in the #t_symbol* array, i.e. the number of members allocated for the array in the struct.
*/
#define ATTR_SYM_VARSIZE(objclass,attrname,structmember,sizemember,maxsize) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(symbol),(maxsize),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,sizemember),calcoffset(objclass,structmember)))


/**
	Create an array-of-atoms attribute of variable length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	sizemember		The actual number of items in the #t_atom array at any given moment.
	@param	maxsize			The maximum number of items in the #t_atom array, i.e. the number of members allocated for the array in the struct.
*/
#define ATTR_ATOM_VARSIZE(objclass,attrname,structmember,sizemember,maxsize) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(atom),(maxsize),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,sizemember),calcoffset(objclass,structmember)))


/**
	Create an array-of-objects attribute of variable length, and add it to a Max class.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of this attribute as a C-string.
	@param	structmember	The C identifier of the member in the struct that holds the value of this attribute.
	@param	sizemember		The actual number of items in the #t_object* array at any given moment.
	@param	maxsize			The maximum number of items in the #t_object* array, i.e. the number of members allocated for the array in the struct.
*/
#define ATTR_OBJ_VARSIZE(objclass,attrname,structmember,sizemember,maxsize) \
	class_addattr((objclass::m_class),attr_offset_array_new(attrname,USESYM(object),(maxsize),/*flags*/0,(method)0L,(method)0L,calcoffset(objclass,sizemember),calcoffset(objclass,structmember)))


/**
	Add flags to an attribute.  

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	flags			Any flags you wish to add to this attribute, as defined in #e_max_attrflags.
*/
#define ATTR_ADD_FLAGS(objclass,attrname,flags) \
	{ t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); \
		long oldflags = object_method(theattr,gensym("getflags")); \
		object_method(theattr,gensym("setflags"),oldflags|flags); }


/**
	Remove flags from an attribute.  

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	flags			Any flags you wish to remove from this attribute, as defined in #e_max_attrflags.
*/
#define ATTR_REMOVE_FLAGS(objclass,attrname,flags) \
	{ t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); \
		long oldflags = object_method(theattr,gensym("getflags")); \
		object_method(theattr,gensym("setflags"),oldflags&(~flags)); }


/**
	Add a filter to the attribute to limit the lower bound of a value.
	The limiting will be performed by the default attribute accessor.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	minval			The minimum acceptable value to which the attribute will be limited.
	@see	CLASS_ATTR_FILTER_MAX
	@see	CLASS_ATTR_FILTER_CLIP
	@see	CLASS_ATTR_MIN
*/
#define ATTR_FILTER_MIN(objclass,attrname,minval) \
	{ t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); \
		attr_addfilter_clip(theattr,minval,0,1,0); }


/**
	Add a filter to the attribute to limit the upper bound of a value.
	The limiting will be performed by the default attribute accessor.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	maxval			The maximum acceptable value to which the attribute will be limited.
	@see	CLASS_ATTR_FILTER_MIN
	@see	CLASS_ATTR_FILTER_CLIP
	@see	CLASS_ATTR_MAX
*/
#define ATTR_FILTER_MAX(objclass,attrname,maxval) \
	{ t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); \
		attr_addfilter_clip(theattr,0,maxval,0,1); }


/**
	Add a filter to the attribute to limit both the lower and upper bounds of a value.
	The limiting will be performed by the default attribute accessor.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	minval			The maximum acceptable value to which the attribute will be limited.
	@param	maxval			The maximum acceptable value to which the attribute will be limited.
	@see 
*/
#define ATTR_FILTER_CLIP(objclass,attrname,minval,maxval) \
	{ t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); \
		attr_addfilter_clip(theattr,minval,maxval,1,1); }


/**
	Create a new attribute that is an alias of an existing attribute.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the actual attribute as a C-string.
	@param	aliasname		The name of the new alias attribute.
*/
#define ATTR_ALIAS(objclass,attrname,aliasname) \
	{	t_object *thealias; \
		t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); \
		thealias = object_clone(theattr); \
		object_method(thealias,USESYM(setname),gensym(aliasname)); \
		class_addattr(objclass::m_class,thealias); \
		CLASS_ATTR_ATTR_PARSE(objclass::m_class,aliasname,"alias",USESYM(symbol),0,attrname); }

/**
	Add a new attribute to the specified attribute to specify a default value.
	The default value will be automatically set when the object is created only if your object uses a dictionary constructor
	with the #CLASS_FLAG_NEWDICTIONARY flag.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
*/
#define ATTR_DEFAULT(objclass,attrname,parsestr) \
	{ t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"default",(t_symbol *)object_method(theattr,USESYM(gettype)),/*flags*/0,parsestr); }


/**
	Add a new attribute to the specified attribute to indicate that the specified attribute should be saved with the patcher.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
*/
#define ATTR_SAVE(objclass,attrname) \
	CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"save",USESYM(long),/*flags*/0,"1")


/**
	A convenience wrapper for both #CLASS_ATTR_DEFAULT and #CLASS_ATTR_SAVE.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	@see	CLASS_ATTR_DEFAULT
	@see	CLASS_ATTR_SAVE
*/
#define ATTR_DEFAULT_SAVE(objclass,attrname,parsestr) \
	{ CLASS_ATTR_DEFAULT(objclass::m_class,attrname,/*flags*/0,parsestr); CLASS_ATTR_SAVE(objclass::m_class,attrname,/*flags*/0); }


/**
	Add a new attribute to the specified attribute to specify a default value, based on Max's Object Defaults.
	If a value is present in Max's Object Defaults, then that value will be used as the default value.
	Otherwise, use the default value specified here.
	The default value will be automatically set when the object is created only if your object uses a dictionary constructor
	with the #CLASS_FLAG_NEWDICTIONARY flag.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
*/
#define ATTR_DEFAULTNAME(objclass,attrname,parsestr) \
	{ t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"defaultname",(t_symbol *)object_method(theattr,USESYM(gettype)),/*flags*/0,parsestr); }


/**
	A convenience wrapper for both #CLASS_ATTR_DEFAULTNAME and #CLASS_ATTR_SAVE.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	@see	CLASS_ATTR_DEFAULTNAME
	@see	CLASS_ATTR_SAVE
*/
#define ATTR_DEFAULTNAME_SAVE(objclass,attrname,parsestr) \
	{ CLASS_ATTR_DEFAULTNAME(objclass::m_class,attrname,/*flags*/0,parsestr); CLASS_ATTR_SAVE(objclass::m_class,attrname,/*flags*/0); }


/**
	Add a new attribute to the specified attribute to specify a lower range.
	The values will not be automatically limited.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	@see	CLASS_ATTR_MAX
	@see	CLASS_ATTR_FILTER_MAX
	@see	CLASS_ATTR_FILTER_CLIP
*/
#define ATTR_MIN(objclass,attrname,parsestr) \
  { t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"min",(t_symbol *)object_method(theattr,USESYM(gettype)),/*flags*/0,parsestr); } 


/**
	Add a new attribute to the specified attribute to specify an upper range.
	The values will not be automatically limited.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	@see	CLASS_ATTR_MIN
	@see	CLASS_ATTR_FILTER_MAX
	@see	CLASS_ATTR_FILTER_CLIP
*/
#define ATTR_MAX(objclass,attrname,parsestr) \
  { t_object *theattr=(t_object *)class_attr_get(objclass::m_class,gensym(attrname)); CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"max",(t_symbol *)object_method(theattr,USESYM(gettype)),/*flags*/0,parsestr); } 


// useful attr attr macros for UI objects

/**
	Add a new attribute indicating that any changes to the specified attribute will trigger a call
	to the object's paint method.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
*/
#define ATTR_PAINT(objclass,attrname) \
	CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"paint",USESYM(long),/*flags*/0,"1")


/**
	A convenience wrapper for both #CLASS_ATTR_DEFAULT and #CLASS_ATTR_PAINT.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	@see	CLASS_ATTR_DEFAULT
	@see	CLASS_ATTR_PAINT
*/
#define ATTR_DEFAULT_PAINT(objclass,attrname,parsestr) \
	{ CLASS_ATTR_DEFAULT(objclass::m_class,attrname,/*flags*/0,parsestr); CLASS_ATTR_PAINT(objclass::m_class,attrname,/*flags*/0); }


/**
	A convenience wrapper for #CLASS_ATTR_DEFAULT, #CLASS_ATTR_SAVE, and #CLASS_ATTR_PAINT.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	@see	CLASS_ATTR_DEFAULT
	@see	CLASS_ATTR_PAINT
	@see	CLASS_ATTR_SAVE
*/
#define ATTR_DEFAULT_SAVE_PAINT(objclass,attrname,parsestr) \
	{ CLASS_ATTR_DEFAULT(objclass::m_class,attrname,/*flags*/0,parsestr); CLASS_ATTR_SAVE(objclass::m_class,attrname,/*flags*/0); CLASS_ATTR_PAINT(objclass::m_class,attrname,/*flags*/0); }


/**
	A convenience wrapper for #CLASS_ATTR_DEFAULTNAME, #CLASS_ATTR_SAVE, and #CLASS_ATTR_PAINT.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	@see	CLASS_ATTR_DEFAULTNAME
	@see	CLASS_ATTR_PAINT
	@see	CLASS_ATTR_SAVE
*/	
#define ATTR_DEFAULTNAME_PAINT(objclass,attrname,parsestr) \
	{ CLASS_ATTR_DEFAULTNAME(objclass::m_class,attrname,/*flags*/0,parsestr); CLASS_ATTR_PAINT(objclass::m_class,attrname,/*flags*/0); }


/**
	A convenience wrapper for #CLASS_ATTR_DEFAULTNAME, #CLASS_ATTR_SAVE, and #CLASS_ATTR_PAINT.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	@see	CLASS_ATTR_DEFAULTNAME
	@see	CLASS_ATTR_PAINT
	@see	CLASS_ATTR_SAVE
*/
#define ATTR_DEFAULTNAME_SAVE_PAINT(objclass,attrname,parsestr) \
	{ CLASS_ATTR_DEFAULTNAME(objclass::m_class,attrname,/*flags*/0,parsestr); CLASS_ATTR_SAVE(objclass::m_class,attrname,/*flags*/0); CLASS_ATTR_PAINT(objclass::m_class,attrname,/*flags*/0); }


// useful attr attr macros for inpector properties


/**
	Add a new attribute to the specified attribute to specify an editor style for the Max inspector.
	Available styles include
	<ul>
	 	<li>"text"      : a text editor</li>
		<li>"onoff"     : a toggle switch</li>
		<li>"rgba"      : a color chooser</li>
		<li>"enum"      : a menu of available choices, whose symbol will be passed upon selection</li>
		<li>"enumindex" : a menu of available choices, whose index will be passed upon selection</li>
		<li>"rect"      : a style for displaying and editing #t_rect values</li>
		<li>"font"      : a font chooser</li>
		<li>"file"      : a file chooser dialog</li>
	</ul>

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
*/
#define ATTR_STYLE(objclass,attrname,parsestr) \
	CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"style",USESYM(symbol),/*flags*/0,parsestr)


/**
	Add a new attribute to the specified attribute to specify an a human-friendly label for the Max inspector.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	labelstr		A C-string, which will be parsed into an array of atoms to set the initial value.
*/
#define ATTR_LABEL(objclass,attrname,labelstr) \
	CLASS_ATTR_ATTR_FORMAT(objclass::m_class,attrname,"label",USESYM(symbol),/*flags*/0,"s",gensym_tr(labelstr))	


/**
	Add a new attribute to the specified attribute to specify a list of choices to display in a menu
	for the Max inspector.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	
	@remark This macro automatically calls
	@code
	CLASS_ATTR_STYLE(objclass::m_class,attrname,flags,"enum").
	@endcode
	
	@see	CLASS_ATTR_ENUMINDEX
*/
#define ATTR_ENUM(objclass,attrname,parsestr) \
	{ CLASS_ATTR_STYLE(objclass::m_class,attrname,/*flags*/0,"enum"); CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"enumvals",USESYM(atom),/*flags*/0,parsestr); }


/**
	Add a new attribute to the specified attribute to specify a list of choices to display in a menu
	for the Max inspector.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	
	@remark This macro automatically calls
	@code
	CLASS_ATTR_STYLE(objclass::m_class,attrname,flags,"enumindex").
	@endcode
	
	@see	CLASS_ATTR_ENUM
*/
#define ATTR_ENUMINDEX(objclass,attrname,parsestr) \
	{ CLASS_ATTR_STYLE(objclass::m_class,attrname,/*flags*/0,"enumindex"); CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"enumvals",USESYM(atom),/*flags*/0,parsestr); }

// localizable versions
#define ATTR_ENUMINDEX2(objclass,attrname,enum1,enum2) \
{ t_atom aaa[2]; CLASS_ATTR_STYLE(objclass::m_class,attrname,/*flags*/0,"enumindex"); atom_setsym(aaa,gensym_tr(enum1)); atom_setsym(aaa+1,gensym_tr(enum2)); \
CLASS_ATTR_ATTR_ATOMS(objclass::m_class,attrname,"enumvals",USESYM(atom),/*flags*/0,2,aaa); }
		
#define ATTR_ENUMINDEX3(objclass,attrname,enum1,enum2,enum3) \
{ t_atom aaa[3]; CLASS_ATTR_STYLE(objclass::m_class,attrname,/*flags*/0,"enumindex"); atom_setsym(aaa,gensym_tr(enum1)); atom_setsym(aaa+1,gensym_tr(enum2)); atom_setsym(aaa+2,gensym_tr(enum3));\
CLASS_ATTR_ATTR_ATOMS(objclass::m_class,attrname,"enumvals",USESYM(atom),/*flags*/0,3,aaa); }
		
#define ATTR_ENUMINDEX4(objclass,attrname,enum1,enum2,enum3,enum4) \
{ t_atom aaa[4]; CLASS_ATTR_STYLE(objclass::m_class,attrname,/*flags*/0,"enumindex"); atom_setsym(aaa,gensym_tr(enum1)); atom_setsym(aaa+1,gensym_tr(enum2)); atom_setsym(aaa+2,gensym_tr(enum3)); atom_setsym(aaa+3,gensym_tr(enum4));\
CLASS_ATTR_ATTR_ATOMS(objclass::m_class,attrname,"enumvals",USESYM(atom),/*flags*/0,4,aaa); }
		
#define ATTR_ENUMINDEX5(objclass,attrname,enum1,enum2,enum3,enum4,enum5) \
{ t_atom aaa[5]; CLASS_ATTR_STYLE(objclass::m_class,attrname,/*flags*/0,"enumindex"); atom_setsym(aaa,gensym_tr(enum1)); atom_setsym(aaa+1,gensym_tr(enum2)); atom_setsym(aaa+2,gensym_tr(enum3));\
atom_setsym(aaa+3,gensym_tr(enum4)); atom_setsym(aaa+4,gensym_tr(enum5));\
CLASS_ATTR_ATTR_ATOMS(objclass::m_class,attrname,"enumvals",USESYM(atom),/*flags*/0,5,aaa); }
		
#define ATTR_ENUMINDEX6(objclass,attrname,enum1,enum2,enum3,enum4,enum5,enum6) \
{ t_atom aaa[6]; CLASS_ATTR_STYLE(objclass::m_class,attrname,/*flags*/0,"enumindex"); atom_setsym(aaa,gensym_tr(enum1)); atom_setsym(aaa+1,gensym_tr(enum2)); atom_setsym(aaa+2,gensym_tr(enum3));\
atom_setsym(aaa+3,gensym_tr(enum4)); atom_setsym(aaa+4,gensym_tr(enum5)); atom_setsym(aaa+5,gensym_tr(enum6));\
CLASS_ATTR_ATTR_ATOMS(objclass::m_class,attrname,"enumvals",USESYM(atom),/*flags*/0,6,aaa); }
		
		
/**
	Add a new attribute to the specified attribute to specify a category to which the attribute is assigned
	in the Max inspector.  
	Categories are represented in the inspector as tabs.  
	If the specified category does not exist then it will be created.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
*/
#define ATTR_CATEGORY(objclass,attrname,parsestr) \
	CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"category",USESYM(symbol),/*flags*/0,str_tr(parsestr))


/**
	A convenience wrapper for #CLASS_ATTR_STYLE, and #CLASS_ATTR_LABEL.

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	stylestr		A C-string that names the style for the attribute. 
							See #CLASS_ATTR_STYLE for the available styles.
	@param	labelstr		A C-string that names the category to which the attribute is assigned in the inspector.

	@see	CLASS_ATTR_STYLE
	@see	CLASS_ATTR_LABEL
*/
#define ATTR_STYLE_LABEL(objclass,attrname,stylestr,labelstr) \
	{ CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"style",USESYM(symbol),/*flags*/0,stylestr); CLASS_ATTR_ATTR_FORMAT(objclass::m_class,attrname,"label",USESYM(symbol),/*flags*/0,"s",gensym_tr(labelstr)); }	


/**
	Add a new attribute to the specified attribute to flag an attribute as invisible to the Max inspector.  

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
*/
#define ATTR_INVISIBLE(objclass,attrname) \
	CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"invisible",USESYM(long),/*flags*/0,"1")


/**
	Add a new attribute to the specified attribute to specify a default order in which to list attributes.  

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
	@param	parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	
	@remark	A value of zero indicates that there is no ordering.  Ordering values begin at 1.  For example:
	@code
	CLASS_ATTR_ORDER(objclass::m_class, "firstattr",	0, "1");
	CLASS_ATTR_ORDER(objclass::m_class, "secondattr",	0, "2");
	CLASS_ATTR_ORDER(objclass::m_class, "thirdattr",	0, "3");
	@endcode
*/
#define ATTR_ORDER(objclass,attrname,parsestr) \
	CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"order",USESYM(long),/*flags*/0,parsestr)
		
/**
	Add a new attribute to the specified attribute to specify that it should appear in the inspector's Basic tab.  

	@ingroup	attr
	@param	objclass		The class pointer.
	@param	attrname		The name of the attribute as a C-string.
 
 */
#define ATTR_BASIC(objclass,attrname) \
CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"basic",USESYM(long),/*flags*/0,"1")



	
// useful attr attr macro for objects that embed binary data as base64

#define ATTR_ATOMARRAY(objclass,attrname) \
	CLASS_ATTR_ATTR_PARSE(objclass::m_class,attrname,"atomarray",USESYM(long),/*flags*/0,"1")


/**	Define and add attributes to class methods.
	@ingroup attr
	@param	objclass		The class pointer.
	@param	methodname		The name of the existing method as a C-string.
	@param	attrname		The name of the attribute to add as a C-string.
	@param	type			The datatype of the attribute to be added.
	@param	parsestring		A C-string, which will be parsed into an array of atoms to set the initial value.
	
	@remark An example which makes a method invisible to users:
	@code
	class_addmethod(objclass::m_class, (method)my_foo, "foo", 0);
	CLASS_METHOD_ATTR_PARSE(objclass::m_class, "foo", "undocumented", gensym("long"), 0, "1");
	@endcode
*/
#define METHOD_ATTR_PARSE(objclass,methodname,attrname,type,parsestring) \
	{	t_hashtab *methods=NULL; \
		t_object *m=NULL; \
		methods = (t_hashtab *)class_extra_lookup(objclass::m_class,gensym("methods")); \
		if (methods) { \
			hashtab_lookup(methods,gensym((methodname)),&m); \
			if (m) \
				object_addattr_parse(m,attrname,type,/*flags*/0,parsestring); \
		} \
	} 


// sticky macros for attribute attributes, and method attributes. Useful for defining attribute groups

/**
	Create an attribute, and add it to all following attribute declarations.
	The block is closed by a call to #CLASS_STICKY_ATTR_CLEAR.

	@ingroup	attr
	@param		objclass		The class pointer.
	@param		name			The name of the new attribute to create as a C-string.
	@param		flags			Any flags you wish to declare for this new attribute, as defined in #e_max_attrflags.
	@param		parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	
	@remark		The most common use of CLASS_STICKY_ATTR is for creating multiple attributes with the same category,
				as in this example:
	@code
	CLASS_STICKY_ATTR(objclass::m_class, "category", 0, "Foo");
	
	CLASS_ATTR_DOUBLE(objclass::m_class, "bar", 0, t_myobject, x_bar);
	CLASS_ATTR_LABEL(objclass::m_class, "bar", 0, "A Bar");

	CLASS_ATTR_CHAR(objclass::m_class, "switch", 0, t_myobject, x_switch);
	CLASS_ATTR_STYLE_LABEL(objclass::m_class, "switch", 0, "onoff", "Bar Switch");

	CLASS_ATTR_DOUBLE(objclass::m_class, "flow", 0, t_myobject, x_flow);
	CLASS_ATTR_LABEL(objclass::m_class, "flow",	0, "Flow Amount");
	
	CLASS_STICKY_ATTR_CLEAR(objclass::m_class, "category");
	@endcode
	
	@see		CLASS_STICKY_ATTR_CLEAR
*/
#define STICKY_ATTR(objclass,name,parsestr) \
	{ t_object *attr = attribute_new_parse(name,NULL,/*flags*/0,parsestr); class_sticky(objclass::m_class,gensym("sticky_attr"),gensym(name),attr); }


/**
	Close a #CLASS_STICKY_ATTR block.

	@ingroup	attr
	@param		objclass		The class pointer.
	@param		name			The name of the sticky attribute as a C-string.
	@see		CLASS_STICKY_ATTR
*/
#define STICKY_ATTR_CLEAR(objclass,name) class_sticky_clear(objclass::m_class,gensym("sticky_attr"),name?gensym(name):NULL)

#define STICKY_CATEGORY(objclass,name) \
{ t_object *attr = attribute_new_format("category",NULL,/*flags*/0,"s",gensym_tr(name)); class_sticky(objclass::m_class,gensym("sticky_attr"),gensym("category"),attr); }

#define STICKY_CATEGORY_CLEAR(objclass) class_sticky_clear(objclass::m_class,gensym("sticky_attr"),gensym("category"))

/**
	Create an attribute, and add it to all following method declarations.
	The block is closed by a call to #CLASS_STICKY_METHOD_CLEAR.

	@ingroup	attr
	@param		objclass		The class pointer.
	@param		name			The name of the new attribute to create as a C-string.
	@param		flags			Any flags you wish to declare for this new attribute, as defined in #e_max_attrflags.
	@param		parsestr		A C-string, which will be parsed into an array of atoms to set the initial value.
	
	@remark		The most common use of CLASS_STICKY_ATTR is for creating multiple attributes with the same category,
				as in this example:
	@code
	CLASS_STICKY_METHOD(objclass::m_class, "undocumented", 0, "1");
	
	// add some methods here with class_add_method()
	// the undocumented attribute for methods means that the ref-page 
	// generator will ignore these methods.
	
	CLASS_STICKY_METHOD_CLEAR(objclass::m_class, "undocumented");
	@endcode
	
	@see		CLASS_STICKY_METHOD_CLEAR
*/
#define STICKY_METHOD(objclass,name,parsestr) \
	{ t_object *attr = attribute_new_parse(name,NULL,/*flags*/0,parsestr); class_sticky(objclass::m_class,gensym("sticky_method"),gensym(name),attr); }


/**
	Close a #CLASS_STICKY_METHOD block.

	@ingroup	attr
	@param		objclass		The class pointer.
	@param		name			The name of the sticky attribute as a C-string.
	@see		CLASS_STICKY_METHOD
*/
#define STICKY_METHOD_CLEAR(objclass,name) class_sticky_clear(objclass::m_class,gensym("sticky_method"),name?gensym(name):NULL)


#endif // MAXMSP_CPP_6_ATTR_H
