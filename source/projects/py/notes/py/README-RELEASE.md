---
title: py-js -- python3 objects for max v0.1
author: S. Alireza
output: 
    pdf_document:
        toc: true
        toc_depth: 3
        number_sections: false
---

---
Simple (and extensible) [python3](https://www.python.org) externals for [MaxMSP](https://cycling74.com)

repo - <https://github.com/shakfu/py-js>

Built, codesigned and notarized on 11 March, 2022 on MacOS Catalina 10.15.7 for x86_64 (Intel Mac) with Python 3.9.10

\newpage

## Intro

Thanks for downloading the first v0.1 release of python3 externals for Max/MSP from the [py-js](https://github.com/shakfu/py-js) project.

The common idea in these externals is to help you use and distribute python code and libraries in your Max applications.

In the first release, there are two different implementations of self-contained python3 externals which can be downloaded from the project site: a 'static' or statically linked distribution and a 'shared' or dynamically linked distribution. The former does not have any dependent binaries in its bundle and the latter has the typical `libpython3.9.dylib` as well as the `*.so` extensions embedded in its bundle.

Why have two implementations for the same thing? Simply, for the flexibility to handle different requirements and special cases such as:

- Certain licensing requires dynamic linking instead of static linking

- Special case: 'static' externals will not allow `numpy` to be embedded for rather obscure reasons, and will raise an error upon importation. This is still an issue (which may be resolved in the future), but as of this writing, use a 'shared' distribution if you want to embed numpy in the external.

- If you are sensitive to modest size differences:

    name     | type       | size (MB)
    :------- | :--------- | :----
    py       | static     | 9.0
    pyjs     | static     | 8.8
    py       | shared     | 16.7
    pyjs     | shared     | 16.5

- If you want to use both `py` and `pyjs` in the same patch. Although it is quite redundant to have two types of python interpreters, you may want to do so. In this case, the 'static' distribution makes this possible and the 'shared' distribution will not allow it.

In any case, we are probably getting ahead of ourselves.

What's the difference between `py` and `pyjs`? The next sections will provide more details on the differences between the two and why you should use one vs the other.

\newpage

![py-js test](./media/xkcd-python-environment.png)

\newpage

## **py** external

```text
globals
    obj_count                    : number of active py objects
    registry                     : global registry to lookup object names

patchers
    subpatchers
        py_repl                  : a basic single line repl for py
        py_repl_plus             : embeds a py object in a py_repl

py max external
    attributes
        name                     : unique object name
        file                     : file to load into editor
        autoload                 : load file at start
        pythonpath               : add path to python sys.path
        debug                    : switch debug logging on/off

    methods (messages) 
        core
            import <module>      : python import to object namespace
            eval <expression>    : python 'eval' semantics
            exec <statement>     : python 'exec' semantics
            execfile <path>      : python 'execfile' semantics
        
        extra
            assign <var> [arg]   : max-friendly msg assignments to py object namespace
            call <pyfunc> [arg]  : max-friendly python function calling
            pipe <arg> [pyfunc]  : process a py/max value via a pipe of py funcs
            code <expr|stmt>     : alternative way to eval or exec py code
            anything <expr|stmt> : anything version of the code method 

        time-based
            sched <t> <fn> [arg] : defer a python function call by t millisecs

        code editor
            read <path>          : read text file into editor
            load <path>          : combo of read <path> -> execfile <path>
            run                  : run the current code in the editor
     
        interobject
            scan                 : scan patcher and store names of child objects
            send <msg>           : send an arbitrary message to a named object

    inlets
        single inlet             : primary input (anything)

    outlets
        left outlet              : primary output (anything)
        middle outlet            : bang on failure
        right outlet             : bang on success 
```

## **pyjs** external

```text
pyjs max external (jsextension)
    attributes
        name                     : unique object name
        file                     : file to load in object namespace
        pythonpath               : add path to python sys.path
        debug                    : switch debug logging on/off
    
    methods 
        core (messages)
            import <module>      : python import to object namespace
            eval <expression>    : python 'eval' semantics
            exec <stmnt>         : python 'exec' semantics
            execfile <path>      : python 'execfile' semantics
        
        extra
            code <expr|stmt>     : eval/exec/import python code (see above)
            

        in-code (non-message)
            eval_to_json <expr>  : python 'eval' returns json


```

\newpage

## Key Features

### Core

py/js's *core* features have a one-to-one correspondance to python's *very high layer* as specified [here](https://docs.python.org/3/c-api/veryhigh.html). In the following, when we refer to *object*, we refer to instances of either the `py` or `pyjs` externals. A note of differences between the variations will be provided below.

- **Per-object namespaces**. Each object has a unique name (which is provided automatically or can be set by the user), and responds to an `import <module>` message which loads the specified python module in its namespace (essentially a `globals` dictionary). Notably, namespaces can be different for each instance.

- **Eval Messages**. Responds to an `eval <expression>` message in the left inlet which is evaluated in the context of the namespace. `py` objects output results to the left outlet, send a bang from the right outlet upon success or a bang from the middle outlet upon failure. `pyjs` objects just return an `atomarray` of the results.

- **Exec Messages**. Responds to an `exec <statement>` message and an `execfile <filepath>` message which executes the statement or the file's code in the object's namespace. For `py` objects, this produces no output from the left outlet, sends a bang from the right outlet upon success or a bang from the middle outlet upon failure. For `pyjs` objects no output is given.

### Extra

The *extra* category of methods  makes the `py` or `pyjs` object play nice with the max/msp ecosystem:

Implemented for `py` objects at present:

- **Assign Messages**. Responds to an `assign <varname> [x1, x2, ..., xN]` which is equivalent to `<varname> = [x1, x2, ..., xN]` in the python namespace. This is a way of creating variables in the object's python namespace using max message syntax. This produces no output from the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

- **Call Messages**. Responds to a `call <func> arg1 arg2 ... argN` kind of message where `func` is a python callable in the py object's namespace. This corresponds to the python `callable(*args)` syntax. This makes it easier to call python functions in a max-friendly way. If the callable does not have variable arguments, it will alternatively try to apply the arguments as a list i.e. `call func(args)`. Future work will try make `call` correspond to a python generic function call: `<callable> [arg1 arg2 ... arg_n] [key1=val1 key2=val2 ... keyN=valN]`. This outputs results to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

- **Pipe message**. Like a `call` in reverse, responds to a `pipe <arg> <f1> <f2> ... <fN>` message. In this sense, a value is *piped* through a chain of python functions in the objects namespace and returns the output to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

Implemented for both `py` and `pyjs` objects:

- **Code or Anything Messages**. Responds to a `code <expression || statement>` or (anything) `<expression || statement>` message. Arbitrary python code (expression or statement) can be used here, because the whole message body is converted to a string, the complexity of the code is only limited by Max's parsing and excaping rules. (EXPERIMENTAL and evolving).

Implemented for `pyjs` objects only:

- **Evaluate to JSON**. Can be used in javascript code only to automatically serialize the results of a python expression as a json string as follows: `evaluate_to_json <expression> -> JSON`.

### Interobject Communication

Implemented for `py` objects only:

- **Scan Message**. Responds to a `scan` message with arguments. This scans the parent patcher of the object and stores scripting names in the global registry.

- **Send Message**. Responds to a `send <object-name> <msg> <msg-body>` message. Used to send *typed* messages to any named object. Evokes a `scan` for the patcher's objects if a `registry` of names is empty.

### Editing Support

Implemented for `py` objects only.

- **Line REPL**. The `py`has two bpatcher line `repls`, one of which embeds a `py` object and another which has an outlet to connect to one. The repls include a convenient menu with all of the `py` object's methods and also feature coll-based history via arrow-up/arrow-down recall of entries in a session. Of course, a coll can made to save all commands if required.

- **Experimental Remote Console**. A new method (due to Ian Duncan) of sending code to the `py` node via `udp` has been implemented and allows for send-from-editor and send-from-interactive-console capabilities. The clients are still in their infancy, but this method looks promising since you get syntax highlighting, syntax checking, and other features. It assumes you want to treat your `py` nodes as remotely accessible `server/interpreters-in-max`.

- **Code Editor**. Double-clicking the `py` object opens a code-editor. This is populated by a `read` message which reads a file into the editor and saves the filepath to an attribute. A `load` message also `reads` the file followed by `execfile`. Saving the text in the editor uses the attribute filepath and execs the saved text to the object's namespace.

For `pyjs` objects, code editing is already built into the `js` objects.

\newpage

## FAQ

### Is this macOS only?

This project is macOS x86_64 (intel) compatible currently.

### What about compatibility with Apple Silicon?

There is an `M1` branch for the project under-development to provide this compatability and shift the build project's build system to cmake.

### What about compatibility with Windows?

There's no particular reason why this project doesn't work in windows except that I don't develop in windows any longer. Feel free to send pull requests to project though.

### Does it only work with Homebrew python?

It works by default with [Homebrew](https://brew.sh) installed python but it can also work with python compiled from source as well.

There is no instrinsic reason why it shouldn't work with other python3 installations on your system.

### Does it embed python into the external or is the external connecting to the local python installation?

The default build creates a lightweight external linked to your local homebrew python3; another variation embeds python3 into an external linked to python3 which resides in a Max package; and another variation embeds python into the external itself without an dependencies. There are other ways as well. The project [README](https://github.com/shakfu/py-js) gives an overview of the differences between the different approaches.

### Every time I open a patch there is a some debug information in the console. Should I be concerned?

It looks like someone left @debug=on in this patch and it further may have cached some paths to related on the build system in the patch. You should be able to switch it off by setting @debug=off.

### How to extend python with your own scripts and 3rd party libraries?

The easiest solution is not to use a self-contained external and use an external that's linked to your system python3 installation. This is what gets built if you run ./build.sh in the root of the [py-js](https://github.com/shakfu/py-js) project. If you do it this way, you automatically get access to all of your python libraries, but you give up portability.

This release contains relocatable python3 externals which are useful for distribution in packages and standalones so it's a little bit more involved.

First note that there several ways to add code to the external:

A. The external's site-packages: py-js/externals/py.mxo/Contents/Resources/lib/python3.9/site-packages

B. The package script folder: `py-js/examples/scripts`

C. Whichever path you set the patcher PYTHONPATH property to (during object creation).

For A, I have tested pure python scripts which should work without re-codesigning the externals, but if you add compiled extensions, then I think you have to re-codesign the external. Check out my [maxutils](https://github.com/shakfu/maxutils) project for help with that.

For B, this is just a location that's searched automatically with `load`, `read`, and `execfile` messages so it can contain dependent files.

For C, this is just setting that is done at the patch level so it should be straightforward. As mentioned, the extra pythonpath is currently only set at object creation. It should be updated when changed but this is something on the todo list.

### How to get numpy to work in py-js

The easiest way is to just create an adhoc python external linked to your system python3 setup. If you have numpy installed there, then you should be good to go with the following caveat: the type translation system does not currently automatically cover native numpy dtypes so they would have to be converted to normal lists before they become translated to to Max lists. This is not a hard constraint, just not implemented yet.

You can also add your system `site-packages` to the externals pythonpath attribute.

If you need numpy embedded in a portable variation of py-js, then you have a couple of options. A py-js package build which has 'thin' externals referencing a python distribution in the `support` folder of the package is the way to go and is provided by the `bin-homebrew-pkg` build option.

It is also possible to package numpy in a full relocatable external, it's quite involved, and cannot currently only be done with non-statically built relocatable externals. The releases section has an example of this just be aware that it is very large and has not been minimized.
