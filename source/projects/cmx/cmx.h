/**
 * @file cmx.h
 * @author shakfu (https://github.com/shakfu)
 * @brief libcmx: common max library
 * @version 0.1
 * @date 2022-03-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// ===========================================================================
// HEADER

#ifndef CMX_H
#define CMX_H

#include "ext.h"
#include "ext_obex.h"


// ---------------------------------------------------------------------------
// constants

#define CMX_MAX_ELEMS 1024


// ---------------------------------------------------------------------------
// forward declarations

// t_symbol* locate_path_to_external(t_class* c);
t_string* get_path_to_external(t_class* c, char* subpath);
t_string* get_path_to_package(t_class* c, char* subpath);
t_object* create_object(t_object* x, const char* text);
void path_basename(const char* path, char* filename);
void path_dirname(const char* path, char* parent_directory);
void path_join(char* destination, const char* path1, const char* path2);

#endif // CMX_H


// ===========================================================================
// IMPLEMENTATION

#ifdef CMX_IMPLEMENTATION


/**
 * @brief      Return path to external with optional subpath
 *
 * @param      c        t_class instance
 * @param      subpath  The subpath or NULL (if not)
 *
 * @return     path to external + (optional subpath)
 */
t_string* get_path_to_external(t_class* c, char* subpath)
{
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
    char conform_path[MAX_PATH_CHARS];
    short path_id = class_getpath(c);
    t_string* result;

#ifdef __APPLE__
    const char* ext_filename = "%s.mxo";
#else
    const char* ext_filename = "%s.mxe64";
#endif
    snprintf_zero(external_name, MAX_FILENAME_CHARS, ext_filename, c->c_sym->s_name);
    path_toabsolutesystempath(path_id, external_name, external_path);
    path_nameconform(external_path, conform_path, PATH_STYLE_NATIVE,
                     PATH_TYPE_TILDE);
    result = string_new(external_path);
    if (subpath != NULL) {
        string_append(result, subpath);
    }
    return result;
}


/**
 * @brief      Return path to package with optional subpath
 *
 * @param      c        t_class instance
 * @param      subpath  The subpath or NULL (if not)
 *
 * @return     path to package + (optional subpath)
 */
t_string* get_path_to_package(t_class* c, char* subpath)
{
    char _dummy[MAX_PATH_CHARS];
    char externals_folder[MAX_PATH_CHARS];
    char package_folder[MAX_PATH_CHARS];

    t_string* result;
    t_string* external_path = get_path_to_external(c, NULL);

    const char* ext_path_c = string_getptr(external_path);

    path_splitnames(ext_path_c, externals_folder, _dummy); // ignore filename
    path_splitnames(externals_folder, package_folder, _dummy); // ignore filename

    // post("externals_folder: %s", externals_folder);
    // post("package_folder: %s", package_folder);

    result = string_new((char*)package_folder);

    if (subpath != NULL) {
        string_append(result, subpath);
    }

    return result;
}



t_object* create_object(t_object* x, const char* text)
{
    t_object* patcher;

    if (object_obex_lookup(x, gensym("#P"), &patcher) == MAX_ERR_NONE) {
    	t_object* obj = newobject_fromboxtext(patcher, text);
    	if (obj != NULL)
    		return obj;
    }
    return NULL;
}



/** Load a patcher file located in the Max search path by name.
 * 
*/
void open_patch(const char *name)
{
	if (stringload(name) == 0) 
		error("could not load patch: '%s'", name);
}

/**
 * @brief      get parent directory 
 *
 * @param[in]  path                 full path
 * @param[out] parent_directory     output parent_directory
 */
void path_dirname(const char* path, char* parent_directory)
{
    char _filename[MAX_PATH_CHARS];
    path_splitnames(path, parent_directory, _filename); // discard filename
}

/**
 * @brief      get path basename
 *
 * @param[in]  path         full path
 * @param[out] filename     output filename
 */
void path_basename(const char* path, char* filename)
{
    char _folder[MAX_PATH_CHARS];
    path_splitnames(path, _folder, filename); // discard _folder
}



/**
 * @brief      join parent path to child subpath
 *
 * @param[out] destination  output destination path
 * @param[in]  path1        parent path
 * @param[in]  path2        child subpath
 */
void path_join(char* destination, const char* path1, const char* path2)
{
    //char filename[MAX_FILENAME_CHARS]; 
    //strncpy_zero(filename,str->s_name, MAX_FILENAME_CHARS); 

    if(path1 == NULL && path2 == NULL) {
        strcpy(destination, "");
    }
    else if(path2 == NULL || strlen(path2) == 0) {
        strcpy(destination, path1);
    }
    else if(path1 == NULL || strlen(path1) == 0) {
        strcpy(destination, path2);
    } 
    else {
        char directory_separator[] = "/";
#ifdef WIN32
        directory_separator[0] = '\\';
#endif
        const char *last_char = path1;
        while(*last_char != '\0')
            last_char++;        
        int append_directory_separator = 0;
        if(strcmp(last_char, directory_separator) != 0) {
            append_directory_separator = 1;
        }
        strcpy(destination, path1);
        if(append_directory_separator)
            strcat(destination, directory_separator);
        strcat(destination, path2);
    }
}


/**
 * see the following:
 * 
 * - https://cycling74.com/forums/error-handling-with-object_method_typed
 * - https://cycling74.com/forums/obex-how-to-get-return-values-from-object_method
 * - https://cycling74.com/forums/startingstopping-the-dac-by-program-in-max-7-xx
 * - https://cycling74.com/forums/sending-arbitrary-messages-to-other-objects-from-c
 * 
 * It can be confusing. For messages which are internally defined as A_GIMME the correct 
 * call to use is object_method_typed(). But for other messages, say one with A_FLOAT as 
 * the argument, you will likely want to use object_method(). 
 * 
 */

/**
	Retrieves an object's #method for a particular message selector.

	@ingroup obj
	@param 	x		The object whose method is being queried
	@param 	s		The message selector
	@return 		This function returns the #method if successful, or method_false() if unsuccessful.
*/
// method object_getmethod(void *x, t_symbol *s);
// also
// t_method_object *class_getmethod_object(t_class *x, t_symbol *methodname);



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
// t_max_err object_method_typed(void *x, t_symbol *s, long ac, t_atom *av, t_atom *rv);



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

// void *object_method(void *x, t_symbol *s, ...);


        
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


/**
	Parse a C-string into an array of atoms.
	This function allocates memory for the atoms if the ac and av parameters are NULL.
	Otherwise it will attempt to use any memory already allocated to av.
	Any allocated memory should be freed with sysmem_freeptr().
	
	@ingroup	atom
	@param		ac			The address of a variable to hold the number of returned atoms.
	@param		av			The address of a #t_atom pointer to which memory may be allocated and atoms copied.
	@param		parsestr	The C-string to parse.
	@return					A Max error code.
	
	@remark		The following example will parse the string "foo bar 1 2 3.0" into an array of 5 atoms.
				The atom types will be determined automatically as 2 #A_SYM atoms, 2 #A_LONG atoms, and 1 #A_FLOAT atom.
	@code
	t_atom *av =  NULL;
	long ac = 0;
	t_max_err err = MAX_ERR_NONE;
	
	err = atom_setparse(&ac, &av, "foo bar 1 2 3.0");
	@endcode
*/
// t_max_err atom_setparse(long *ac, t_atom **av, C74_CONST char *parsestr);

#endif // CMX_IMPLEMENTATION