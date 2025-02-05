# apy.pyx

`api.pyx` is a cython 'builtin' module which wraps parts of the Max/MSP `c-api`
for the `py` external.

The `api` module consists of:
    - api.pyx: main [cython](https://cython.org) wrapper
    - a number of cython declaration files (`.pxd`) which expose c headers
        - `api_max.pxd`: exposes Max api headers
        - `api_msp.pxd`: exposes MSP api headers
        - `api_py.pxd`: exposes the `py` external's headers

Cython classes, functions, and constants defined here are optionally
available for use by python code running in a `py` external instance.

## Features

- Provides access to max c-api functionality in an interpreted language.

- Wraps a decent subset of the max c-api for python code running in a `py` external instance.

- Write max methods in cython which can be called in c

- All c is exposed via cython to python

- Wrap max types as cython extension classes, e.g. `api.Atom`, `api.Patcher`, ..

- Errors in python don't crash Max (or crash max less!)

## Design Notes

- [ ] to refactor the object_method_typed calls which are being duplicated in `Patcher`, `MaxObect`, `MaxApp`, etc..

- [ ] There should be sufficient number of wrapper classes (which wrap pointers)  to enable productive scripting. So instead of returning a `mx.t_object*` one returns a `MaxObject`

## Usage

Either

1. Send the `py` object an `import api` message or

2. Import a module on the pythonpath which imports the `api` module or

3. Send a `(load <file.py)` message to load a file on the Max search path which contains python code that imports the `api` module. Then;

4. Call one of the functions or classes in this file making sure to prefix it with `api.`

```text
        1.
   ( import api )
        |
        |                      2.
      [ py ] ------ ( api.post('hello world') )
```

## Development

A lot of the painful laborious work of creating header mappings has been
done and can be reviewed (and corrected) in
the `api_max.pxd` and `api_msp.pxd` files.

This provides the benefit that we can import the max api via its header
declarations as follows:

```python
    cimport api_max as mx
```

Then you can start using Max api symbols or functions in the cython code 
by prefixing with `mx.` For example

```text
    gensym()    -> mx.gensym()
    post()      -> mx.post()
    ...
```

In addition the `py` external api is also mapped for use by cython
(see below) in api_py.pxd file:

```python
    cimport api_py as px
```

Again please note any function exposed from the `py` external must
be prefixed as `px`:

```text
    py_scan()   -> px.py_scan()
```

This separation of namespaces is clearly very useful when you are
wrapping code.

## Extension Types

We use cython extension types to wrap related C data structures and functions
in the Max api. This them accessible to python code and gives them a python-friendly interface.

So far the following extension types are implemented (partial or otherwise)

- [x] MaxObject
- [x] Atom
- [x] Table
- [x] Buffer
- [x] Dictionary
- [x] Database
- [x] DatabaseResult
- [x] DatabaseView
- [x] Linklist
- [x] Binbuf
- [x] Atombuf
- [x] Hashtab
- [x] AtomArray
- [x] Patcher
- [x] Box
- [x] Buffer
- [x] PyExternal
- [x] MaxApp

Workarounds for max types which are not exposed in the c-api:

- `coll`: import and export the contents of a `coll` into a `dict` by
  sending a message to the `dict` object.

- `jit.cellblock`: link a `coll` to a `cellblock` and data sent to the
  `coll` will be sent to the `cellblock`.

- `jit.matrix`: can be populated via `jit.fill` from a `coll`
