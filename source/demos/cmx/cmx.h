/**
 * @file cmx.h
 * @author shakfu (https://github.com/shakfu)
 * @brief libcmx: common max library
 * @version 0.1
 * @date 2025-03-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */

// ===========================================================================
// HEADER

#ifndef CMX_H
#define CMX_H

#include "ext.h"
#include "ext_obex.h"

#ifdef CMX_CPP_HEADER
#include <string>
#include <filesystem>
namespace fs = std::filesystem;
#endif

// ---------------------------------------------------------------------------
// constants

#define CMX_MAX_ELEMS 1024


// ---------------------------------------------------------------------------
// forward declarations

#ifdef CMX_CPP_HEADER
std::string get_path_to_external(t_class* c, char* subpath);
std::string get_path_to_package(t_class* c, char* subpath);
#else
t_string* get_path_to_external(t_class* c, char* subpath);
t_string* get_path_to_package(t_class* c, char* subpath);
#endif

t_object* create_object(t_object* x, const char* text);
void path_basename(const char* path, char* filename);
void path_dirname(const char* path, char* parent_directory);
void path_join(char* destination, const char* path1, const char* path2);

#endif // CMX_H


// ===========================================================================
// IMPLEMENTATION

#ifdef CMX_IMPLEMENTATION


#ifdef CMX_CPP_HEADER
/**
 * @brief      Return path to external with optional subpath
 *
 * @param      c        t_class instance
 * @param      subpath  The subpath or NULL (if not)
 *
 * @return     path to external + (optional subpath)
 */
std::string get_path_to_external(t_class* c, char* subpath)
{
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
    char conform_path[MAX_PATH_CHARS];
    short path_id = class_getpath(c);
    fs::path result;

#ifdef __APPLE__
    const char* ext_filename = "%s.mxo";
#else
    const char* ext_filename = "%s.mxe64";
#endif
    snprintf_zero(external_name, MAX_FILENAME_CHARS, ext_filename, c->c_sym->s_name);
    path_toabsolutesystempath(path_id, external_name, external_path);
    path_nameconform(external_path, conform_path, PATH_STYLE_NATIVE,
                     PATH_TYPE_TILDE);
    result = fs::path(external_path);
    if (subpath != NULL) {
        result /= subpath;
    }
    return result.string();
}


/**
 * @brief      Return path to package with optional subpath
 *
 * @param      c        t_class instance
 * @param      subpath  The subpath or NULL (if not)
 *
 * @return     path to package + (optional subpath)
 */
std::string get_path_to_package(t_class* c, char* subpath)
{
    fs::path external_path = fs::path(get_path_to_external(c, NULL));
    fs::path externals_folder = external_path.parent_path();
    fs::path package_folder = externals_folder.parent_path();

    if (subpath != NULL) {
        package_folder /= subpath;
    }

    return package_folder.string();
}

#else

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

#endif

t_symbol* locate_path_from_symbol(t_object* x, t_symbol* s)
{
    char filename[MAX_PATH_CHARS];
    char pathname[MAX_PATH_CHARS];
    short path;
    t_fourcc type = FOUR_CHAR_CODE('TEXT');
    t_max_err err;

    strncpy_zero(filename, s->s_name, MAX_PATH_CHARS);
    if (locatefile_extended(filename, &path, &type, &type, 1)) {
        // nozero: not found
        error("can't find file %s", s->s_name);
        return gensym("");
    }

    pathname[0] = 0;
    err = path_toabsolutesystempath(path, filename, pathname);
    if (err != MAX_ERR_NONE) {
        error("can't convert %s to absolutepath", s->s_name);
        return gensym("");
    }
    // post("full path is: %s", pathname->s_name);
    return gensym(pathname);
}


/**
 * @brief Create a new object from a box text
 *
 * @param x
 * @param text
 * @return t_object*
 */
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



/**
 * @brief Open a patch file located in the Max search path by name
 * 
 * @param name
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


#endif // CMX_IMPLEMENTATION