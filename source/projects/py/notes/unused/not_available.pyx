# in header but not public

t_symbol *symbol_stripquotes(t_symbol *s);


def name_from_pathname(str pathname) -> str:
    """get name from pathname"""
    cdef char name[2048]
    mx.path_namefrompathname(pathname.encode(), name)
    return name.encode()


cdef class AtomArray:

    cdef mx.t_max_err setindex(self, long index, mx.t_atom* av):
        return mx.atomarray_setindex(self.ptr, index, av)


cdef class Dictionary:

    cdef mx.t_max_err set_bglocked(self, char c):
        """Set whether a patcher's background layer is locked."""
        return mx.jpatcher_set_bglocked(self.ptr, c)

    cdef char get_bghidden(self):
        """Determine whether a patcher's background layer is hidden."""
        return mx.jpatcher_get_bghidden(self.ptr)

    cdef mx.t_max_err set_bghidden(self, char c):
        """Set whether a patcher's background layer is hidden."""
        return mx.jpatcher_set_bghidden(self.ptr, c)

    cdef char get_fghidden(self):
        """Determine whether a patcher's foreground layer is hidden."""
        return mx.jpatcher_get_fghidden(self.ptr)

    cdef mx.t_max_err set_fghidden(self, char c):
        """Set whether a patcher's foreground layer is hidden."""
        return mx.jpatcher_set_fghidden(self.ptr, c)

    cdef mx.t_max_err get_editing_bgcolor(self, mx.t_jrgba *prgba):
        """Retrieve a patcher's editing background color."""
        return mx.jpatcher_get_editing_bgcolor(self.ptr, prgba)

    cdef mx.t_max_err set_editing_bgcolor(self, mx.t_jrgba *prgba):
        """Set a patcher's editing background color."""
        return mx.jpatcher_set_editing_bgcolor(self.ptr, prgba)

    cdef mx.t_max_err get_locked_bgcolor(self, mx.t_jrgba *prgba):
        """Retrieve a patcher's locked background color."""
        return mx.jpatcher_get_locked_bgcolor(self.ptr, prgba)

    cdef mx.t_max_err set_locked_bgcolor(self, mx.t_jrgba *prgba):
        """Set a patcher's locked background color."""
        return mx.jpatcher_set_locked_bgcolor(self.ptr, prgba)

    cdef mx.t_symbol *get_default_fontname(self):
        """Return the name of the default font used for new objects in a patcher."""
        return mx.jpatcher_get_default_fontname(self.ptr)

    cdef mx.t_max_err get_defrect(self, mx.t_rect *pr):
        """Query a patcher to determine the location and dimensions of its window when initially opened."""
        return mx.jpatcher_get_defrect(self.ptr, pr)

    cdef mx.t_max_err set_defrect(self, mx.t_rect *pr):
        """Set a patcher's default location and size."""
        return mx.jpatcher_set_defrect(self.ptr, pr)

    cdef float get_default_fontsize(self):
        """Return the size of the default font used for new objects in a patcher."""
        return mx.jpatcher_get_default_fontsize(self.ptr)

    cdef long get_default_fontface(self):
        """Return the index of the default font face used for new objects in a patcher."""
        return mx.jpatcher_get_default_fontface(self.ptr)


cdef class Dictionary:
    cdef mx.t_max_err read_yaml(self, const char* filename, const short path, mx.t_dictionary** d):
        """Read the specified JSON file and return a t_dictionary object."""
        return mx.dictionary_read_yaml(filename, path, d)

    cdef mx.t_max_err dictionary_write_yaml(self, const char *filename, const short path):
        """Serialize the specified t_dictionary object to a YAML file."""
        return mx.dictionary_write_yaml(self.d, filename, path)

cdef class Linklist:

    cdef mx.t_llelem* index2ptr(self, long index):
        return mx.linklist_index2ptr(self.lst, index)

    cdef long ptr2index(self, mx.t_llelem* p):
        return mx.linklist_ptr2index(self.lst, p)

    cdef mx.t_llelem* insertptr(self, void* o, mx.t_llelem* p):
        return mx.linklist_insertptr(self.lst, o, p)

    cdef long deleteptr(self, mx.t_llelem* p):
        return mx.linklist_deleteptr(self.lst, p)

    cdef long insertnodeindex(self, mx.t_llelem* p, long index):
        return mx.linklist_insertnodeindex(self.lst, p, index)

    cdef mx.t_llelem* insertnodeptr(self, mx.t_llelem* p1, mx.t_llelem* p2):
        return mx.linklist_insertnodeptr(self.lst, p1, p2)

    cdef long appendnode(self, mx.t_llelem* p):
        return mx.linklist_appendnode(self.lst, p)

    cdef void free(self, mx.t_llelem* elem):
        mx.linklistelem_free(self.lst, elem)

    # ERRORS

    cdef t_llelem *linklistelem_new()
    cdef long linklist_insert_sorted(t_linklist *x, void *o, long cmpfn(void *, void *))
    cdef t_atom_long linklist_findfirst(t_linklist *x, void **o, long cmpfn(void *, void *), void *cmpdata)
    cdef void linklist_findall(t_linklist *x, t_linklist **out, long cmpfn(void *, void *), void *cmpdata)
    cdef void linklist_methodall(t_linklist *x, t_symbol *s, ...)
    cdef void *linklist_methodindex(t_linklist *x, t_atom_long i, t_symbol *s, ...)
    cdef void linklist_sort(t_linklist *x, long cmpfn(void *, void *))


cdef class Box:
    def get_hint(self) -> str:
        """Retrieve a box's 'hint' attribute."""
        cdef mx.t_symbol* hint = mx.jbox_get_hint(self.ptr)
        return sym_to_str(hint)

    def set_hint(self, hint: str):
        """Set a box's 'hint' attribute."""
        cdef mx.t_max_err err = mx.jbox_set_hint(self.ptr, str_to_sym(hint))
        if err != mx.MAX_ERR_NONE:
           return error("could not set box's hint attribute")

    def get_annotation(self):
        """Retrieve a box's annotation string, if the user has given it an annotation."""
        cdef const char * annotation = mx.jbox_get_annotation(self.ptr)
        return annotation.encode('utf8')

    def set_annotation(self, annotation: str):
        """set a box's scripting name."""
        mx.jbox_set_annotation(self.ptr, annotation.decode())

    def get_presentation(self) -> bool:
        """Determine if a box is included in the presentation view."""
        return bool(mx.jbox_get_presentation(self.ptr))

    def set_presentation(self, on: bool):
        """Set box in the presentation view."""
        return mx.jbox_set_presentation(self.ptr, on)

cdef class Hashtab:

    cdef mx.t_hashtab_entry* entry_new(self, mx.t_symbol* key, mx.t_object* val):
         return mx.hashtab_entry_new(key, val)

    cdef void entry_free(self, mx.t_hashtab_entry *x):
         mx.hashtab_entry_free(x)

    cdef t_max_err hashtab_findfirst(t_hashtab *x, void **o, long cmpfn(void *, void *), void *cmpdata)

    cdef t_max_err hashtab_methodall(t_hashtab *x, t_symbol *s, ...)
