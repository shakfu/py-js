// pktpy2.h

#ifndef PKTPY2_H
#define PKTPY2_H


// ----------------------------------------------------------------------------
// Includes

// max api
#include "ext.h"
#include "ext_obex.h"

// pocketpy
#include "pocketpy.h"

// ----------------------------------------------------------------------------
// Constants

#define PY_MAX_ELEMS 1024
#define ITER_SUCCESS 1
#define ITER_STOP 0
#define ITER_FAILURE (-1)

// ----------------------------------------------------------------------------
// Macros

#define py_checklist(self) py_checktype(self, tp_list)
#define py_checktuple(self) py_checktype(self, tp_tuple)
#define py_checkdict(self) py_checktype(self, tp_dict)

// ----------------------------------------------------------------------------
// Globals

t_class* pktpy2_class; // global pointer to object class

// ----------------------------------------------------------------------------
// Datastructures

typedef struct t_pktpy2 t_pktpy2;

// ----------------------------------------------------------------------------
// Object creation and destruction Methods

void* pktpy2_new(t_symbol* s, long argc, t_atom* argv);
void pktpy2_free(t_pktpy2* x);
void pktpy2_init(t_pktpy2* x);

// ----------------------------------------------------------------------------
// Attribute Getters / Setters and Helpers

t_max_err pktpy2_name_get(t_pktpy2 *x, t_object *attr, long *argc, t_atom **argv);
t_max_err pktpy2_name_set(t_pktpy2 *x, t_object *attr, long argc, t_atom *argv);

// ----------------------------------------------------------------------------
// Helpers

void pktpy2_float(t_pktpy2 *x, double f);

// ----------------------------------------------------------------------------
// Path helpers

t_max_err pktpy2_locate_path_from_symbol(t_pktpy2* x, t_symbol* s);

// ----------------------------------------------------------------------------
// Side-effect helpers

void pktpy2_bang(t_pktpy2* x);
void pktpy2_bang_success(t_pktpy2* x);
void pktpy2_bang_failure(t_pktpy2* x);

// ----------------------------------------------------------------------------
// Common handlers

t_max_err pktpy2_handle_float_output(t_pktpy2* x, py_GlobalRef pfloat);
t_max_err pktpy2_handle_long_output(t_pktpy2* x, py_GlobalRef plong);
t_max_err pktpy2_handle_string_output(t_pktpy2* x, py_GlobalRef pstring);
t_max_err pktpy2_handle_bool_output(t_pktpy2* x, py_GlobalRef pbool);
t_max_err pktpy2_handle_list_output(t_pktpy2* x, py_GlobalRef plist);
t_max_err pktpy2_handle_tuple_output(t_pktpy2* x, py_GlobalRef ptuple);
t_max_err pktpy2_handle_output(t_pktpy2* x, py_GlobalRef retval);

// ----------------------------------------------------------------------------
// Core Python Methods

t_max_err pktpy2_import(t_pktpy2* x, t_symbol* s);
t_max_err pktpy2_exec(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy2_eval(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy2_execfile(t_pktpy2* x, t_symbol* s);

// ----------------------------------------------------------------------------
// Extra Python Methods

// t_max_err pktpy2_anything(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv);

// ----------------------------------------------------------------------------
// Information Methods

// void pktpy2_count(t_pktpy2* x);
// void pktpy2_metadata(t_pktpy2* x);
// void pktpy2_assist(t_pktpy2* x, void* b, long m, long a, char* s);

// ----------------------------------------------------------------------------
// Time-based Methods

// t_max_err pktpy2_task(t_pktpy2* x);
// t_max_err pktpy2_sched(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv);

// ----------------------------------------------------------------------------
// Interobject Methods

// t_max_err pktpy2_send(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv);
// void pktpy2_scan(t_pktpy2* x);
// long pktpy2_scan_callback(t_pktpy2* x, t_object* obj);

// ----------------------------------------------------------------------------
// Code editor Methods

void pktpy2_read(t_pktpy2* x, t_symbol* s);
void pktpy2_doread(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv);
void pktpy2_load(t_pktpy2* x, t_symbol* s); // read(f) -> execfile(f)
void pktpy2_dblclick(t_pktpy2* x);
void pktpy2_run(t_pktpy2* x);
void pktpy2_edclose(t_pktpy2* x, char** text, long size);
t_max_err pktpy2_edsave(t_pktpy2* x, char** text, long size);
void pktpy2_okclose(t_pktpy2* x, char *s, short *result);

// ----------------------------------------------------------------------------
// max datastructure support methods

// ----------------------------------------------------------------------------



#endif // PKTPY2_H
