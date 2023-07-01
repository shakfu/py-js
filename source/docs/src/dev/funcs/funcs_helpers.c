
long class_is_ui(t_class *c);
t_dictionary *object_dictionaryarg(long ac, t_atom *av);
t_max_err atom_alloc(long *ac, t_atom **av, char *alloc);
t_max_err atom_alloc_array(long minsize, long *ac, t_atom **av, char *alloc);
t_max_err class_subclass(t_class *superclass, t_class *subclass);
t_max_err symbolarray_sort(long ac, t_symbol **av);
t_object *class_super_construct(t_class *c, ...);
t_symbol *symbol_stripquotes(t_symbol *s);
t_symbol *symbol_unique(void);
void error_code(void *x, t_max_err v); //interrupt safe
void error_sym(void *x,t_symbol *s); //interrupt safe
void post_sym(void *x,t_symbol *s);  //interrupt safe


//more util functions

/**
	Generates a unique #t_symbol *. The symbol will be formatted somewhat like "u123456789". 

	@ingroup	misc
	@return 	This function returns a unique #t_symbol *.
*/
t_symbol *symbol_unique(void);


/**
	Strip quotes from the beginning and end of a symbol if they are present.
	@ingroup	misc
	@param	s	The symbol to be stipped.
	@return		Symbol with any leading/trailing quote pairs removed.
 */
t_symbol *symbol_stripquotes(t_symbol *s);


void error_code(void *x, t_max_err v); //interrupt safe


/**
	Posts an error message to the Max window. This function is interrupt safe. 

	@ingroup misc

	@param 	x		The object's pointer
	@param 	s		Symbol to be posted as an error in the Max window
*/
void error_sym(void *x,t_symbol *s); //interrupt safe


/**
	Posts a message to the Max window. This function is interrupt safe. 

	@ingroup misc
	@param 	x		The object's pointer
	@param 	s		Symbol to be posted in the Max window
*/
void post_sym(void *x,t_symbol *s);  //interrupt safe


/**
	Performs an ASCII sort on an array of #t_symbol *s.

	@ingroup misc

	@param 	ac		The count of #t_symbol *s in <tt>av</tt>
	@param 	av		An array of #t_symbol *s to be sorted

	@return 		This function returns the error code #MAX_ERR_NONE if successful, 
	 				or one of the other error codes defined in #e_max_errorcodes if unsuccessful.
*/
t_max_err symbolarray_sort(long ac, t_symbol **av);


/**
	Retrieve a pointer to a dictionary passed in as an atom argument.
	Use this function when working with classes that have dictionary constructors
	to fetch the dictionary.
	
	@ingroup obj
	@param	ac	The number of atoms.
	@param	av	A pointer to the first atom in the array.
	@return		The dictionary retrieved from the atoms.
	@see		attr_dictionary_process()
*/
t_dictionary *object_dictionaryarg(long ac, t_atom *av);


/**
	Allocate a single atom.
	If ac and av are both zero then memory is allocated.
	Otherwise it is presumed that memory is already allocated and nothing will happen.

	@ingroup		atom
	@param	ac		The address of a variable that will contain the number of atoms allocated (1).
	@param	av		The address of a pointer that will be set with the new allocated memory for the atom.
	@param	alloc	Address of a variable that will be set true is memory is allocated, otherwise false.
	@return			A Max error code.
*/
t_max_err atom_alloc(long *ac, t_atom **av, char *alloc);


/**
	Allocate an array of atoms.
	If ac and av are both zero then memory is allocated.
	Otherwise it is presumed that memory is already allocated and nothing will happen.

	@ingroup		atom
	@param	minsize	The minimum number of atoms that this array will need to contain.
					This determines the amount of memory allocated.
	@param	ac		The address of a variable that will contain the number of atoms allocated.
	@param	av		The address of a pointer that will be set with the new allocated memory for the atoms.
	@param	alloc	Address of a variable that will be set true is memory is allocated, otherwise false.
	@return			A Max error code.
*/
t_max_err atom_alloc_array(long minsize, long *ac, t_atom **av, char *alloc);


/**
	Determine if a class is a user interface object.
	
	@ingroup 	class
	@param	c	The class pointer.
	@return		True is the class defines a user interface object, otherwise false.
*/
long class_is_ui(t_class *c);


// new subclassing implementation		
/**
	Define a subclass of an existing class.
	First call class_new on the subclass, then pass in to class_subclass. 
	If constructor or destructor are NULL will use the superclass constructor.

	@ingroup			class
	@param	superclass	The superclass pointer.
	@param	subclass	The subclass pointer.
	@return			A Max error code
*/		
t_max_err class_subclass(t_class *superclass, t_class *subclass);
		
		
/**
	Call super class constructor.
	Use this instead of object_alloc if you want to call the super class constructor, 
	but allocating enough memory for subclass.

	@ingroup		class
	@param	c		The (sub)class pointer.
	@param	...		Args to super class constructor.
	@return			initialized object instance
*/		
t_object *class_super_construct(t_class *c, ...);
