# apy.pyx

api.pyx is a cython 'builtin' module which wraps parts of the Max/MSP c-api
for the `py` external.

The `api` module consists of:
    - api.pyx: main cython (https://cython.org) wrapper
    - a number of cython declaration files (`.pxd`) which expose c headers
        - api_max.pxd: exposes Max api headers
        - api_msp.pxd: exposes MSP api headers
        - api_py.pxd: exposes the `py` external's headers

Cython classes, functions, and constants defined here are optionally
available for use by python code running in a `py` external instance.

## Usage

Either 

1. Send the `py` object an `import api` message or 

2. Import a module on the pythonpath which imports the `api` module or

3. Send a `(load <file.py)` message to load a file on the Max search path which contains python code that imports the `api` module.

Then

4. Call one of the functions or classes in this file 
   making sure to prefix it with `api.`


        1.
   ( import api )
        |
        |                      2.
      [ py ] ------ ( api.post('hello world') )


## Development

A lot of the painful laborious work of creating header mappings has been
done and can be reviewed (and corrected) in
the `api_max.pxd` and `api_msp.pxd` files.

This provides the benefit that we can import the max api via its header
declarations as follows:

    cimport api_max as mx

Then you can start using Max api symbols or functions in the cython code 
by prefixing with `mx.` For example

    gensym()    -> mx.gensym()
    post()      -> mx.post()
    ...

In addition the `py` external api is also mapped for use by cython
(see below) in api_py.pxd file:

    cimport api_py as px


Again please note any function exposed from the `py` external must
be prefixed as `px`:

    py_scan()   -> px.py_scan()


This separation of namespaces is clearly very useful when you are
wrapping code.


## Extension Types

We use cython extension types to wrap related C data structures and functions
in the Max api. This them accessible to python code and gives them a python-friendly interface.

So far the following extension types are planned or implemented (partial or otherwise)

- [x] MaxObject: general max t_object class
- [x] Atom
- [x] Atom Array: container for an array of atoms
- [ ] Atombuf: an alternative to Binbufs for temporary storage of atoms.
- [x] Binbuf
- [x] Buffer
- [x] Database: SQLite database access
- [x] Dictionary: structured/hierarchical data that is both sortable and fast
- [x] Hash Table: hash table for mapping symbols to data
- [ ] Index Map: managed array of pointers
- [x] Linked List: doubly-linked-list
- [ ] Quick Map: a double hash with keys mapped to values and vice-versa
- [x] String Object: wrapper for C-strings with an API for manipulating them
- [ ] Symbol Object: wrapper for symbols
- [x] Table
- [x] Patcher

- [x] PyExternal

Workarounds for max types which are not exposed in the c-api:

- coll: import and export the contents of a coll into a dict by
  sending a message to the dict object.

- jit.cellblock: link a coll to a cellblock and data sent to the
  coll will be sent to the cellblock.

- jit.matrix: can be populated via jit.fill from a coll


## Table of Contents

- imports
- compile time conditional imports
- compile time constants
- helper cdef functions (type-translation)
- extension types
- helper def functions