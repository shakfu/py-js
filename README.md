# py for max

An attempt to make a simple (and extensible) max external for python


## Features

### v0.1

Done

- [x] implementation of some high level python api functions in max
- [x] each py object as a virtual python which responds to follows msgs
	- [x] import <module>: adds module to the namespace
	- [x] eval <code> || eval @file <script>: eval <code> or <script> in namespace
	- [ ] exec <code> || exec @file <script>: inject into namespace (i.e. python exec)
- [x] exensible by embedded cython based python extension to include maxapi functions in python code so you could (hypothetically) e.g. create objects, send messages, etc..
- [ ] load default code
- [ ] edit default with text editor


## py/pyext docs (for inspiration!)

With the py object you can load python modules and execute the functions therein.
With the pyext you can use python classes to represent full-featured pd/Max message objects.
Multithreading (detached methods) is supported for both objects.
You can send messages to named objects or receive (with pyext) with Python methods.

## Notes

- outlet creation order is important in outlet_new(x, NULL)?

- Py_eval_input is equivalent to the built-in eval -- it evaluates an expression.
- Py_file_input is equivalent to exec -- It executes Python code, but does not return anything.
- Py_single_input evaluates an expression and prints its value -- used in the interpreter.


## TODO

- [ ] add right inlet bang after eval op ends
- [ ] add text edit object
- [ ] if attr has same name as method (the import saga), crash. fixed by making them different (should be another better way.)
- [x] add @run <script>
- [x] add cythonized access to max c-api..?
- [x] refactor eval code from py_eval into a function to allow for exec and execfile or PyRun_File scenarios


## Building

- use `make` in the `msx-sdk/sources/basics/py` directory

## Alternative access

- websockets: https://websockets.readthedocs.io/en/stable/intro.html


