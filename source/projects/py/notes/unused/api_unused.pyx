# place for 'archived' cython code


# ----------------------------------------------------------------------------
# Subclass of MaxObject type

# this works
cdef class SubClass(MaxObject):
    cdef int n

    def __init__(self, int n, classname: str, *args, namespace: str = "box"):
        self.n = n
        super().__init__(classname, namespace)


# ----------------------------------------------------------------------------
# String type

# This string type is totally useless in this context
# use python string instead!!

cdef class String:
    cdef mx.t_string* _str

    def __cinit__(self, bytes cstr):
        self._str = mx.string_new(cstr)

    def getptr(self) -> bytes:
        return mx.string_getptr(self._str)
            
    def reserve(self, long numbytes):
        mx.string_reserve(self._str, numbytes)

    def append(self, bytes s):
        mx.string_append(self._str, s)

    def chop(self, long numchars):
        mx.string_chop(self._str, numchars)


# ----------------------------------------------------------------------------
# List type


cdef mx.t_cmpfn __linklist_compare_func = None


cdef long compare_linklist_objects(void* o1, void* o2):
    """Methods that require a comparison function pointer to be passed in use this type.

        long (*t_cmpfn)(void *, void *)

    It should return true or false depending on the outcome of the comparison of the two
    linklist items passed in as arguments.
    """
    return <bint>__linklist_compare_func(o1, o2)

def insert_sorted(self, MaxObject obj, sort_func=None) -> long:
    """Insert an item into the list, keeping the list sorted according to a specified comparison function."""
    cdef long idx = -1
    if sort_func:
        __linklist_compare_func = sort_func
    idx = mx.linklist_insert_sorted(self.ptr, obj.ptr, compare_linklist_objects)
    if idx == -1:
        raise ValueError("could not insert sorted object into linklist")
    return idx

# ----------------------------------------------------------------------------
# TABLE type

# removed redundant methods in py.c and `api.PyExternal`, see `api.Table`
# for equivalent functionality.

cdef class PyExternal:
    # ...
    cdef bint table_exists(self, str table_name):
        """Return true if a table exists."""
        return px.py_table_exists(self.ptr, table_name.encode())

    cdef mx.t_max_err list_to_table(self, char* table_name, PyObject* plist):
        """Convert a Python list to a table."""
        return px.py_list_to_table(self.ptr, table_name, plist)

    cdef PyObject* table_to_list(self, char* table_name):
        """Convert a table to python list"""
        return px.py_table_to_list(self.ptr, table_name)


def table_exists(str name):
    """checks if a table exists."""

    cdef long **storage
    cdef long size

    result = mx.table_get(mx.gensym(name.encode('utf-8')), &storage, &size)

    if result == 0:
        success_bang()
    else:
        failure_bang()

    return result


def copy_list_to_table(list[int] xs, str name):
    """copies integers from a python list[int] to a max table"""
    
    cdef long **storage
    cdef long size

    length = len(xs)

    result = mx.table_get(str_to_sym(name), &storage, &size)

    if result == 0:
        if length <= size:
            for i, x in enumerate(xs):
                storage[0][i] = <long>x
        else:
            for i in range(size):
                storage[0][i] = <long>xs[i]


def get_table_as_list(str name):
    """gets integer content of a named max table as a python list"""

    cdef long **storage
    cdef long size
    cdef long value
    cdef list[int] xs = []

    result = mx.table_get(str_to_sym(name), &storage, &size)

    if result == 0:
        for i in range(size):
            value = storage[0][i]
            xs.append(<int>value)

    return xs


# ----------------------------------------------------------------------------
# api.List

# REMOVE or REPLACE
# This class is way too low-level and is a poor replacement for Python's list.
# Investigate how to subclass python list type and include Max friendly features


cdef object __linklist_compare_func = None


cdef long compare_linklist_objects(object o1, object o2):
    """Methods that require a comparison function pointer to be passed in use this type.

        long (*t_cmpfn)(void *, void *)

    It should return true or false depending on the outcome of the comparison of the two
    linklist items passed in as arguments.
    """
    return <bint>__linklist_compare_func(o1, o2)


cdef class ListElement:
    """Wraps the t_llelem linklist element."""

    cdef mx.t_llelem* ptr
    cdef bint ptr_owner

    def __cinit__(self):
        self.ptr = NULL
        self.ptr_owner = False


    @staticmethod
    cdef from_ptr(mx.t_llelem* ptr, bint ptr_owner=False):
        cdef ListElement elem = ListElement.__new__(ListElement)
        elem.ptr = ptr
        elem.ptr_owner = ptr_owner
        return elem


cdef class List:
    """Wraps the t_linklist object."""

    cdef mx.t_linklist* ptr


    def __cinit__(self):
        self.ptr = <mx.t_linklist*>mx.linklist_new()

    def __dealloc__(self):
        mx.linklist_chuck(self.ptr)  # will free list only and not contained objects
        #  or
        #  object_free(self.ptr)  # will free all in list

    def __repr__(self) -> str:
        return f"<List size:{self.get_size()}'>"

    def __getitem__(self, long index) -> MaxObject:
        cdef mx.t_object* obj = <mx.t_object*>mx.linklist_getindex(self.ptr, index)
        return MaxObject.from_ptr(obj)

    def __setitem__(self, long index, MaxObject obj):
        self.insert(index, obj)

    def __delitem__(self, long index):
        self.delete_index(index)

    @property
    def size(self) -> int:
        """Get the size of the linklist."""
        return mx.linklist_getsize(self.ptr)

    def get(self, long idx = 0) -> MaxObject:
        """Return a t_object stored in a linklist at a specified index."""
        cdef mx.t_object* obj = NULL
        if not (0 <= idx <= self.size -1):
            raise IndexError("index out of range")
        obj = <mx.t_object*>mx.linklist_getindex(self.ptr, idx)
        if obj is NULL:
            raise ValueError("value could not be retrieved from index {idx}")
        return MaxObject.from_ptr(obj)

    def chuck(self):
        """Free a linklist, but don't free the items it contains.

        The linklist can contain a variety of different types of data.
        By default, the linklist assumes that all items are max objects with a valid
        #t_object header.
        
        You can alter the linklist's notion of what it contains by using the 
        linklist_flags() method.
        
        When you free the linklist by calling object_free() it then tries to free all of the items it contains.  
        If the linklist is storing a custom type of data, or should otherwise not free the data it contains,
        then call linklist_chuck() to free the object instead of object_free().
        """
        mx.linklist_chuck(self.ptr)

    def get_size(self) -> int:
        """Get the size of the linklist."""
        return mx.linklist_getsize(self.ptr)

    def get_index_of_object(self, MaxObject obj) -> long:
        """Return an item's index, given the item itself."""
        cdef long idx = mx.linklist_objptr2index(self.ptr, <mx.t_object*>obj.ptr)
        if idx == mx.MAX_ERR_GENERIC:
            raise IndexError("could not get index from object")
        return idx

    def append(self, MaxObject obj):
        """Add an item to the end of the list."""
        cdef mx.t_atom_long err = mx.linklist_append(self.ptr, obj.ptr)
        if err == -1:
            raise ValueError("append object failed")

    def insert(self, long index, MaxObject obj):
        """Insert an object at a given index."""
        cdef mx.t_atom_long err = mx.linklist_insertindex(self.ptr, obj.ptr, index)
        if err == -1:
            raise ValueError("append object failed")

    def insert_sorted(self, MaxObject obj, sort_func=None) -> long:
        """Insert an item into the list, keeping the list sorted according to a specified comparison function."""
        cdef long idx = -1
        if sort_func:
            __linklist_compare_func = sort_func
        idx = mx.linklist_insert_sorted(self.ptr, obj.ptr, compare_linklist_objects)
        if idx == -1:
            raise ValueError("could not insert sorted object into linklist")
        return idx

    def insert_after_object(self, MaxObject obj, MaxObject after) -> ListElement:
        """Insert an item into the list after another specified item."""
        cdef mx.t_llelem* elem_ptr = <mx.t_llelem*>mx.linklist_insertafterobjptr(self.ptr,
            <mx.t_object*>obj.ptr, <mx.t_object*>after.ptr)
        return ListElement.from_ptr(elem_ptr)

    def insert_before_object(self, MaxObject obj, MaxObject before) -> ListElement:
        """Insert an item into the list before another specified item."""
        cdef mx.t_llelem* elem_ptr = <mx.t_llelem*>mx.linklist_insertbeforeobjptr(self.ptr,
            <mx.t_object*>obj.ptr, <mx.t_object*>before.ptr)
        return ListElement.from_ptr(elem_ptr)

    def move_after_object(self, MaxObject obj, MaxObject after) -> ListElement:
        """Move an existing item in the list to a position after another specified item in the list."""
        cdef mx.t_llelem* elem_ptr = <mx.t_llelem*>mx.linklist_moveafterobjptr(self.ptr,
            <mx.t_object*>obj.ptr, <mx.t_object*>after.ptr)
        return ListElement.from_ptr(elem_ptr)

    def move_before_object(self, MaxObject obj, MaxObject before) -> ListElement:
        """Move an existing item in the list to a position before another specified item in the list."""
        cdef mx.t_llelem* elem_ptr = <mx.t_llelem*>mx.linklist_movebeforeobjptr(self.ptr,
            <mx.t_object*>obj.ptr, <mx.t_object*>before.ptr)
        return ListElement.from_ptr(elem_ptr)

    def delete_index(self, long index):
        """Remove the item from the list at the specified index and free it.
    
        The linklist can contain a variety of different types of data.
        By default, the linklist assumes that all items are max objects with a valid
        #t_object header.  Thus by default, it frees items by calling object_free() on them.

        You can alter the linklist's notion of what it contains by using the 
        linklist_flags() method.

        If you wish to remove an item from the linklist and free it yourself, then you
        should use linklist_chuckptr().
        """
        cdef mx.t_max_err err = mx.linklist_deleteindex(self.ptr, index)
        if err:
            raise ValueError("could not delete object at index")

    def chuck_index(self, long index):
        """Remove the item from the list at the specified index.
    
        You are responsible for freeing any memory associated with the item that is
        removed from the linklist.
        """
        cdef mx.t_max_err err = mx.linklist_chuckindex(self.ptr, index)
        if err:
            raise ValueError("could not chuck object at index")

    def chuck_object(self, MaxObject obj) -> int:
        """Remove the specified item from the list.
    
        You are responsible for freeing any memory associated with the item that is
        removed from the linklist.
        """
        cdef long err = mx.linklist_chuckobject(self.ptr, obj.ptr)
        return err

    def delete_object(self, MaxObject obj) -> int:
        """Delete the specified item from the list.

        The object is removed from the list and deleted.
        The deletion is done with respect to any flags passed to linklist_flags.
        """
        cdef long err = mx.linklist_chuckobject(self.ptr, obj.ptr)
        return err

    def clear(self):
        """Remove and free all items in the list.
    
        Freeing items in the list is subject to the same rules as linklist_deleteindex().
        You can alter the linklist's notion of what it contains, and thus how items are freed,
        by using the linklist_flags() method.
        """
        mx.linklist_clear(self.ptr)

    cdef mx.t_atom_long makearray(self, void** a, long max):
        """Retrieve linklist items as an array of pointers."""
        return mx.linklist_makearray(self.ptr, a, max)

    def reverse(self):
        """Reverse the order of items in the linked-list."""
        mx.linklist_reverse(self.ptr)

    def rotate(self, long i):
        """Rotate items in the linked list in circular fashion."""
        mx.linklist_rotate(self.ptr, i)

    def shuffle(self):
        """Randomize the order of items in the linked-list."""
        mx.linklist_shuffle(self.ptr)

    def swap(self, long a, long b):
        """Swap the position of two items in the linked-list, specified by index."""
        mx.linklist_swap(self.ptr, a, b)

    cdef void methodall_imp(self, void* x, void* sym, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6, void* p7, void* p8):
        """Call a method on all objects in the linklist."""
        mx.linklist_methodall_imp(x, sym, p1, p2, p3, p4, p5, p6, p7, p8)

    cdef void* methodindex_imp(self, void* x, void* i, void* s, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6, void* p7):
        """Call a method on an object at a given index."""
        mx.linklist_methodindex_imp(x, i, s, p1, p2, p3, p4, p5, p6, p7)

    cdef mx.t_atom_long funall_break(self, mx.method fun, void* arg):
        """Call a function on all objects in the linklist."""
        return mx.linklist_funall_break(self.ptr, fun, arg)

    cdef void* funindex(self, long i, mx.method fun, void* arg):
        """Call a function on an object at a given index."""
        return mx.linklist_funindex(self.ptr, i, fun, arg)

    cdef void* substitute(self, void* p, void* newp):
        """Substitute an object with a new object."""
        return mx.linklist_substitute(self.ptr, p, newp)

    cdef void* next(self, void* p, void** next):
        """Get the next object."""
        return mx.linklist_next(self.ptr, p, next)

    cdef void* prev(self, void* p, void** prev):
        """Get the previous object."""
        return mx.linklist_prev(self.ptr, p, prev)

    cdef void* last(self, void** item):
        """Get the last object."""
        return mx.linklist_last(self.ptr, item)

    def readonly(self, long readonly=1):
        """Set the readonly flag."""
        mx.linklist_readonly(self.ptr, readonly)

    cdef void flags(self, long flags):
        """Set the flags."""
        mx.linklist_flags(self.ptr, flags)

    cdef mx.t_atom_long getflags(self):
        """Get the flags."""
        return mx.linklist_getflags(self.ptr)

    cdef long match(self, void* a, void* b):
        """Match two objects."""
        return mx.linklist_match(a, b)

    cdef void funall(self, mx.method fun, void* arg):
        """Call a function on all objects in the linklist."""
        mx.linklist_funall(self.ptr, fun, arg)


