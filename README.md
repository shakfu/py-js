# (minimal) py for max

An attempt to make a simple (and extensible) max external for python


## Features

### v0.1

- [x] Implementation of a few high level python api functions in max (eval, exec) to allow the evaluation of python code in a python `globals` namespace associated with the py object.
- [x] Each py object has its own python 'globals' namespace and responds to the following msgs
	- [x] `import <module>`: adds module to the namespace
	- [x] `eval <expression>`: evaluate expression within the context of the namespace (cannot modify ns)
	- [x] `exec <statement>`: executes statement into the namespace (can modify ns)
	- [x] `execfile <file.py>`: executes python file into the namespace (can modify ns)
	- [x] `run <file.py>`: executes python file into the namespace (can modify ns)

- [x] Extensible by embedded cython based python extensions which can call a library of wrapped max_api functions in python code. There is a proof of concept of the python code in the namsepace calling the max api `post` function successfully.
- [x] Exposing of good portion of the max api to cython scripting
- [ ] Autoload default code
- [ ] Edit default with text editor


## Building

The external is being developed using the max-sdk-8.0.3 package (which is installed where packages should be installed in Max 8).

In my case, the `py` external is developed as a project in the `msx-sdk/sources/basics` folder. Feel free to adjust the `Makefile` if your directory structure is different.

Only tested on OS X at present.


## TODO


- [ ] Global object/dict/ref mgmt (so two external can exist without Py_Finalize() causing a crash
- [ ] Implement section on two-way globals setting and reading (from python and c) in https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html
- [ ] Add right inlet bang after eval op ends
- [ ] Add text edit object
- [ ] If attr has same name as method (the import saga), crash. fixed by making them different.
- [x] Add `@run <script>`
- [x] Add cythonize access to max c-api..?
- [x] Refactor eval code from py_eval into a function to allow for exec and execfile or PyRun_File scenarios
- [x] 'import statement' in eval, exec or run causes a segmentation fault. see: https://docs.python.org/3/c-api/intro.html exception handling example
- [x] Add line repl
	- [x] Add up-arrow last line recall (great for 'random.random()')
- [x] Refactor into functions
- [x] make exec work! (needs globals in both slots: `PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals)`


