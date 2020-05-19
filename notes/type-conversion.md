# Convert to/from C, Python, and Atom types.


## Python <-> C 


Long (Int)

- Check: PyLong_Check
- Python -> C: PyLong_AsLong
- C -> Python: PyLong_FromLong (NR)


Double (Float)

- Check: PyFloat_Check
- Python -> C: PyFloat_AsDouble
- C -> Python: PyFloat_FromDouble


Bytes Strings

- Check: PyBytes_Check
- Python -> C: PyBytes_AsString
- C -> Python: PyBytes_FromString


Unicode Strings

- Check: PyUnicode_Check
- Python -> C: PyUnicode_AsUTF8
- C -> Python: PyUnicode_FromString


## Max API - ext_obex_utils.h:

### C -> Atoms

```c

// Assign an array of char values to an array of atoms.
t_max_err atom_setchar_array(long ac, t_atom *av, long count, unsigned char *vals);

// Assign an array of long integer values to an array of atoms.
t_max_err atom_setlong_array(long ac, t_atom *av, long count, t_atom_long *vals);

// Assign an array of 32bit float values to an array of atoms.
t_max_err atom_setfloat_array(long ac, t_atom *av, long count, float *vals);

// Assign an array of 64bit float values to an array of atoms.
t_max_err atom_setdouble_array(long ac, t_atom *av, long count, double *vals);


// -------- Create + Allocate

// Parse a C-string into an array of atoms.
t_max_err atom_setparse(long *ac, t_atom **av, C74_CONST char *parsestr);

// Create an array of atoms populated with values using sprintf-like syntax.
t_max_err atom_setformat(long *ac, t_atom **av, C74_CONST char *fmt, ...); 
// same as atom_setformat using va_list
t_max_err atom_setformat_va(long *ac, t_atom **av, C74_CONST char *fmt, va_list args);

```

### Atoms -> C

```c


// Retrieve values from an array of atoms using sscanf-like syntax.
t_max_err atom_getformat(long ac, t_atom *av, C74_CONST char *fmt, ...);
// same as atom_getformat using va_list
t_max_err atom_getformat_va(long ac, t_atom *av, C74_CONST char *fmt, va_list args);



typedef enum{
  OBEX_UTIL_ATOM_GETTEXT_DEFAULT          // default translation rules for getting text from atoms
  OBEX_UTIL_ATOM_GETTEXT_TRUNCATE_ZEROS   // eliminate redundant zeros for floating point numbers (default used)
  OBEX_UTIL_ATOM_GETTEXT_SYM_NO_QUOTE     // don't introduce quotes around symbols with spaces
  OBEX_UTIL_ATOM_GETTEXT_SYM_FORCE_QUOTE  // always introduce quotes around symbols (useful for JSON)
  OBEX_UTIL_ATOM_GETTEXT_COMMA_DELIM      // separate atoms with commas (useful for JSON)
  OBEX_UTIL_ATOM_GETTEXT_FORCE_ZEROS      // always print the zeros
  OBEX_UTIL_ATOM_GETTEXT_NUM_HI_RES       // print more decimal places
  OBEX_UTIL_ATOM_GETTEXT_NUM_LO_RES       // print fewer decimal places (HI_RES will win though)
} e_max_atom_gettext_flags;

// Convert an array of atoms into a C-string.
t_max_err atom_gettext(long ac, t_atom *av, long *textsize, char **text, long flags);

// Fetch an array of #t_symbol* values from an array of atoms.
t_max_err atom_getsym_array(long ac, t_atom *av, long count, t_symbol **vals);

// Fetch an array of #t_atom values from an array of atoms.
t_max_err atom_getatom_array(long ac, t_atom *av, long count, t_atom *vals);

// Fetch an array of #t_object* values from an array of atoms.
t_max_err atom_getobj_array(long ac, t_atom *av, long count, t_object **vals);
```


### Check Max Types

```c
// Determines whether or not an atom represents a #t_string object.
long atomisstring(const t_atom *a);

// Determines whether or not an atom represents a #t_atomarray object.
long atomisatomarray(t_atom *a);

// Determines whether or not an atom represents a #t_dictionary object.
long atomisdictionary(t_atom *a);

```

### Max Types -> Atoms 
```c
// Assign an array of #t_symbol* values to an array of atoms.
t_max_err atom_setsym_array(long ac, t_atom *av, long count, t_symbol **vals);

// Assign an array of #t_atom values to an array of atoms.
t_max_err atom_setatom_array(long ac, t_atom *av, long count, t_atom *vals);

// Assign an array of #t_object* values to an array of atoms.
t_max_err atom_setobj_array(long ac, t_atom *av, long count, t_object **vals);
```

## Max APU -- ext_obex.h

```c

// Inserts an integer into a #t_atom and change the t_atom's type to #A_LONG. 
t_max_err atom_setlong(t_atom *a, t_atom_long b);

// Inserts a floating point number into a #t_atom and change the t_atom's type to #A_FLOAT. 
t_max_err atom_setfloat(t_atom *a, double b);

// Inserts a #t_symbol * into a #t_atom and change the t_atom's type to #A_SYM. 
t_max_err atom_setsym(t_atom *a, const t_symbol *b);				

// Inserts a generic pointer value into a #t_atom and change the t_atom's type to #A_OBJ. 
t_max_err atom_setobj(t_atom *a, void *b);

// Retrieves a long integer value from a #t_atom. 
t_atom_long atom_getlong(const t_atom *a);

// Retrieves a floating point value from a #t_atom. 
t_atom_float atom_getfloat(const t_atom *a);

// Retrieves a t_symbol * value from a t_atom. 
t_symbol *atom_getsym(const t_atom *a);

// Retrieves a generic pointer value from a #t_atom. 
void *atom_getobj(const t_atom *a);

// Retrieves an unsigned integer value between 0 and 255 from a t_atom. 
long atom_getcharfix(const t_atom *a);

// Retrieves type from a #t_atom. 
long atom_gettype(const t_atom *a);

//the following are useful for setting the values _only_ if there is an arg
//rather than setting it to 0 or _sym_nothing

// Retrieves the integer value of a particular t_atom from an atom list, if the atom exists.
t_max_err atom_arg_getlong(t_atom_long *c, long idx, long ac, const t_atom *av);

// Retrieves the floating point value of a particular t_atom from an atom list, if the atom exists.
long atom_arg_getfloat(float *c, long idx, long ac, const t_atom *av);

// Retrieves the floating point value, as a double, of a particular t_atom from an atom list, if the atom exists.
long atom_arg_getdouble(double *c, long idx, long ac, const t_atom *av);

// Retrieves the t_symbol * value of a particular t_atom from an atom list, if the atom exists.
long atom_arg_getsym(t_symbol **c, long idx, long ac, const t_atom *av);

	
```




