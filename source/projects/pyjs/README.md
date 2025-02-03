# pyjs: a python3 jsextension for max

General purpose python3 jsextension, which means that it is a c-based Max external which can only be accessed via the javascript `js` interface.

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

Note that the source files in this projects are soft-linked from the `py` project. The reason for this is that the `py` and `pyjs` were originally developed together and there is still extensive non-cmake driven build infrastructure which assumes this.

Creating a separate folder for pyjs with its own CMakeLists.txt file means that the pyjs external will be built when `make projects` is called.

Ultimately all externals should have their own folders and documentation, but it will take some iterations to get there.

## Overview

The `pyjs` max external/jsextension provides a `PyJS` class and a minimal subset of the `py` external's features which work well with the max `js` object and javascript code (like returning json directly from evaluations of python expressions).

Th external has access to builtin python modules and the whole universe of 3rd party modules.

General purpose python3 jsextension, which means that it's a c-based Max external which can only be accessed via the javascript `js` interface.

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

### Key Features

The `pyjs` external implements the following c-level methods:

category | method       | param(s)      | in/out | can change ns
:------- | :----------- | :------------ | :----: | :------------:
core     | import       | module        | in     | yes
core     | eval         | expression    | out    | no
core     | exec         | statement     | in     | yes
core     | execfile     | file          | in     | yes
extra    | code         | expr or stmt  | out?   | yes
in-code  | eval_to_json | expression    | out    | no

Note that the `code` method allows for import/exec/eval of python code, which can be said to make those 'fit-for-purpose' methods redundant. However, it has been retained because it provides additional strictness and provides a helpful prefix in messages which indicates message intent.

#### Core

py/js's *core* features have a one-to-one correspondance to python's *very high layer* as specified [here](https://docs.python.org/3/c-api/veryhigh.html). In the following, when we refer to *object*, we refer to instances of the `pyjs` external.

- **Per-object namespaces**. Each object has a unique name (which is provided automatically or can be set by the user), and responds to an `import <module>` message which loads the specified python module in its namespace (essentially a `globals` dictionary). Notably, namespaces can be different for each instance.

- **Eval Messages**. Responds to an `eval <expression>` message in the left inlet which is evaluated in the context of the namespace.`pyjs` objects just return an `atomarray` of the results.

- **Exec Messages**. Responds to an `exec <statement>` message and an `execfile <filepath>` message which executes the statement or the file's code in the object's namespace. For `pyjs` objects no output is given.

#### Extra

The *extra* category of methods  makes the `pyjs` object play nice with the max/msp ecosystem:

- **Evaluate to JSON**. Can be used in javascript code only to automatically serialize the results of a python expression as a json string as follows: `evaluate_to_json <expression> -> JSON`.

#### Editing Support

For `pyjs` objects, code editing is already provided by the [js](https://docs.cycling74.com/max8/refpages/js) Max object.

#### Scripting

A subset of the Max c-api is wrapped by the cython-based `api` module (`api.pyx`). Prior to compilation it is converted to c and then compiled into the external. This exposes a Python *builtin* module called `api` to all python code running on `py` objects.

The `api` module includes functions and cython extension classes which make it relatively easy to call Max c-api methods from python. This is without doubt the most powerful feature of the `py` external.

As of this writing the following extension classes which wrap their corresponding Max datastructures are included in the `api` module: `Atom`, `AtomArray`, `Table`, `Buffer`, `Dictionary`, `Database`, `Linklist`, `Binbuf`, `Hashtab` and `Patcher`.

In addition, a cython extension class, `PyExternal`, gives python code access to the c-based `py` external's data and methods.

To give a sense of the level of integration which is possible as a result of this module, the following example demonstrates how `numpy` and `scipy.signal` can be used to read and write to and from a live Max `buffer~` object using the `api` module's `Buffer` extension class:

```python
import api

import numpy as np
from scipy import signal

def get_buffer_samples(name: str, sample_file: str) -> np.array:
    buf = api.create_buffer(name, sample_file)
    xs = np.array(buf.get_samples())
    assert len(xs) == buf.n_samples
    api.post(f"get {n_samples} samples from buffer {name}")
    return xs


def set_buffer_samples(name: str, duration_ms: int):
    buf = api.create_empty_buffer(name, duration_ms)
    t = np.linspace(0, 1, buf.n_samples, endpoint=False, dtype=np.float64)
    xs = signal.sawtooth(2 * np.pi * 5 * t)
    buf.set_samples(xs)
    api.post(f"set {buf.n_samples} samples to buffer {name}")
```

See the `examples/tests` folder and the `patchers/tests`  folder for more examples.

### Deployment Scenarios

There are 3 general deployment variations:

1. **Linked to system python**. Linking the externals to your system python (homebrew, built from source, etc.) This has the benefit of re-using your existing python modules and is the default option.

2. **Embedded in package**. Embedding the python interpreter in a Max package: in this variation, a dedicated python distribution (zipped or otherwise) is placed in the `support` folder of the `py/js` package (or any other package) and is linked to the `pyjs` extension (or both). This makes it size efficient and usable in standalones.

3. **Embedded in external**. The external itself as a container for the python interpreter: a custom python distribution (zipped or otherwise) is stored inside the external bundle itself, which can make it portable and usable in standalones.

As of this writing all three deployment scenarios are availabe, however it is worth looking more closely into the tradeoffs in each case, and a number of build variations exist.

Deployment Scenario    | `pyjs`
:--------------------- | :--------:
Linked to sys python   | yes
Embeddded in package   | yes
Embeddded in external  | yes

## Caveats

- Packaging and deployment of python3 externals has improved considerably but is still a work-in-progress: basically needing further documentation, consolidation and cleanup. For example, there are currently two build systems which overlap: a python3 based build system to handle complex packaging cases and cmake for handling the quick and efficient development builds and general cases.

- As of this writing, the `api` module, does not (like apparently all 3rd party python c-extensions) unload properly between patches and requires a restart of Max to work after you close the first patch which uses it. Unfortunately, this is a known [bug](https://bugs.python.org/issue34309) in python which is being worked on and may be [fixed](https://groups.google.com/forum/?utm_medium=email&utm_source=footer#!msg/cython-users/SnVpCE7Sq8M/hdT8S2iFBgAJ) in future versions (python 3.12 perhaps?).

- `Numpy`, the popular python numerical analysis package, falls in the above category. In newer versions of Python the situation is improving as above, but in python 3.9.x, it thankfully doesn't crash but gives the following error:

```bash
[py __main__] import numpy: SystemError('Objects/structseq.c:401: bad argument to internal function')
```

This just means that the user opened a patch with a `py-js` external that imports `numpy`, then closed the patch and (in the same Max session) re-opened it, or created a new patch importing `numpy` again.

To fix it, just restart Max and use it normally in your patch. Treat each patch as a session and restart Max after each session.

- `core` features relying on pure python code are supposed to be the most stable, and *should* not crash under most circumstances, `extra` features are less stable since they are more experimental, etc..

- The `api` module is the most experimental and evolving part of this project, and is completely optional. If you don't want to use it, don't import it.
