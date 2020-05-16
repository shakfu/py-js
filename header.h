#ifndef __EXT_GLOBALSYMBOL_H__
#define __EXT_GLOBALSYMBOL_H__
BEGIN_USING_C_LINKAGE
void *globalsymbol_reference(t_object *x, C74_CONST char *name, C74_CONST char *classname)
void globalsymbol_dereference(t_object *x, C74_CONST char *name, C74_CONST char *classname)
t_max_err globalsymbol_bind(t_object *x, C74_CONST char *name, long flags)
void globalsymbol_unbind(t_object *x, C74_CONST char *name, long flags)
void globalsymbol_notify(t_object *x, C74_CONST char *name, t_symbol *msg, void *data)
END_USING_C_LINKAGE
#endif 
