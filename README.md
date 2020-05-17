# (minimal) py for max

An attempt to make a simple (and extensible) max external for python


## Features

### v0.1

- [x] implementation of a few high level python api functions in max (eval, exec) to 
	  allow the evaluation of python code in a python `globals` namespace associated with the py object.
- [x] each py object has its own python 'globals' namespace, that it behaves as virtual module or script, which responds to follows msgs
	- [x] `import <module>`: adds module to the namespace
	- [x] `eval <code> || eval @file <script>`: `eval <code> or <script>` in namespace
	- [ ] `exec <code> || exec @file <script>`: inject into namespace (i.e. python `exec`)

- [x] exensible by embedded cython based python extension to include maxapi functions in python code so you could (hypothetically) e.g. create objects, send messages, etc.. There is a proof of concept of the python code in the namsepace calling the max api `post` function successfully.
- [x] exposing of good portiong of the max api to cython scripting
- [ ] load default code
- [ ] edit default with text editor


## Building

The external is being developed  using the max-sdk-8.0.3 package (which is installed where packages should be installed in Max 8).

In my case, I just happened to be developing the `py` external as a folder in the  `msx-sdk/sources/basics` folder. Feel free to adjust the `Makefile` if your directory structure is different.

Only tested on OS X at present.


## TODO

- [ ] add right inlet bang after eval op ends
- [ ] add text edit object
- [ ] if attr has same name as method (the import saga), crash. fixed by making them different (should be another better way.)
- [x] add `@run <script>`
- [x] add cythonize access to max c-api..?
- [x] refactor eval code from py_eval into a function to allow for exec and execfile or PyRun_File scenarios




## Development Notes


### Repl possible?

**Is it possible to launch a python interactive loop / repl from the py external?**

Ideally it would be fantastic to launch a python repl per py external instance (and of course full access to the globals namespace and the embedded cython wrapping of the max api) which would allow one to script ma via an enteractive loop (real livecoding)

- What about embedding the jupyter kernel? (tried the main python-based kernel -> caused Max to crash). Perhaps need to launch with nogil? or in a different thread or process?

- What about integrating the new jupyter c++-based kernl [xeus](https://github.com/jupyter-xeus/xeus) or its python implementation [xeus-python](https://github.com/jupyter-xeus/xeus-python)?

- What about just using `osc` via `[udpreceive]`? see [python-osc](https://github.com/attwad/python-osc) 

- Iain Duncan's [Schema for Max](https://github.com/iainctduncan/scheme-for-max) has a novel repl which work nicely. Could be another way



### Outlets

- outlet creation order is important in `outlet_new(x, NULL)`?


### Evaluation

- `Py_eval_input` is equivalent to the built-in eval -- it evaluates an expression.
- `Py_file_input` is equivalent to exec -- It executes Python code, but does not return anything.
- `Py_single_input` evaluates an expression and prints its value -- used in the interpreter.

- lack of execfile in python 3 (see https://stackoverflow.com/questions/436198/what-is-an-alternative-to-execfile-in-python-3)

```python
execfile("somefile.py", global_vars, local_vars)

# as

with open("somefile.py") as f:
    code = compile(f.read(), "somefile.py", 'exec')
    exec(code, global_vars, local_vars)
```


### Alternative access

- websockets: https://websockets.readthedocs.io/en/stable/intro.html


