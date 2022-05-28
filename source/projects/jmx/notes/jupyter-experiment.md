# Jupyter, ZeroMQ, Kernels and Clients

## Basic Concept

 The c-extension unloading [problem](https://bugs.python.org/issue34309)  is [well expressed](https://pybind11.readthedocs.io/en/stable/advanced/embedding.html) by the pybind11 project:

  > *Modules created with pybind11 can be safely re-initialized after the interpreter has been restarted.* However, this may not apply to third-party extension modules. The issue is that Python itself cannot completely unload extension modules and there are several caveats with regard to interpreter restarting. In short, not all memory may be freed, either due to Python reference cycles or user-created global data.
  
One possible solution is to follow the Jupyter/IPython model which entails the following design elements:

- [Kernels](https://jupyter.readthedocs.io/en/latest/projects/kernels.html): "Kernels are programming language specific processes that run independently and interact with the Jupyter Applications and their user interfaces."

- [Frontends](https://speakerdeck.com/rgbkrk/jupyter-frontends-from-the-classic-jupyter-notebook-to-jupyterlab-nteract-and-beyond?slide=27): clients which communicate with the kernels by essentially sending code and getting sent back results.

- [Jupyter message specification](https://jupyter-client.readthedocs.io/en/latest/messaging.html#messaging-in-jupyter): :explains the basic communications design and messaging specification for how Jupyter frontends and kernels communicate. The [ZeroMQ](https://zguide.zeromq.org) library provides the low-level transport layer over which these messages are sent." The communication is typically serialized over JSON.

see also [here](https://ipython.org/ipython-doc/3/development/how_ipython_works.html)

## Demo of Running and Accessing the same Python Kernel

```bash
jupyter console
In [1] import os
```

and then in a separate terminal process to connect to the running kernel.

```bash
jupyter console --existing
In [1] os
Out[2]: <module 'os' from '/usr/local/Cellar/python@3.9/3.9.4/Frameworks/Python.framework/Versions/3.9/lib/python3.9/os.py'>
```

## Possible Solutions for py-js

### Core Concept

```text
frontend_1 (Max Ext) <----> Python Kernel (can be any other language)
                                ^
                                |
frontent_2 (Jupyter console) <--+
```

The idea is basically that the 'frontend' to a python (or otherwise) kernel is a Max/MSP external object which sends code and data (translated from MAX atoms) in JSON (according to the message spec) to the kernel and then receives in reply the result in JSON which is then translated to Max atoms.

### The benefits

- The primary benefit is that the kernel itself with all its c-extensions remains in a separate process. There can be multiple frontends accessing the kernel (let's say one from the python console or a jupyter notebook) and then the Max/MSP frontend, since they are already engaging witht the same namespace.

- All of this very well tested and there are many examples and a healthy community.

- It can then access the universe of language kernels out there.

### Other Implementation Possibilities

- to promote language agnosticism, (optionally) constrain the kernel to only return results in JSON.

- retain python interpreter in the jupyter client.

- embed a max `dict` object in the external for storing results.

### Implementation Requirements

- Implement in an external the [JUPYTER MSG FORMAT and WIRE PROTOCOL](https://github.com/jupyter/jupyter_client).

- Implement the translation layer: MAX MESSAGE or ATOMS <-> JSON data and python code

- Use of [ZeroMQ](https://zguide.zeromq.org)

- JSON lib in c:
  - [json-c](https://github.com/json-c/json-c)
  - [cJSON](https://github.com/DaveGamble/cJSON)

- Use of Max Threading

## Clients Examples

- [Jupyter Client in C-Sharp](https://www.codeproject.com/Articles/5250119/jupyter-net-Client-A-Csharp-Library-to-Interact-wi)

- [Jupyter Client in Haskell](https://hackage.haskell.org/package/jupyter-0.9.0/docs/Jupyter-Client.html)

- [nbterm](https://blog.jupyter.org/nbterm-jupyter-notebooks-in-the-terminal-6a2b55d08b70)

- [stackoverflow - executing code in ipython kernal](https://stackoverflow.com/questions/33731744/executing-code-in-ipython-kernel-with-the-kernelclient-api)

- also: [rpclib](http://rpclib.net)

## MSG FORMAT

### MSG HEADER

```python
{
    'msg_id' : str, # typically UUID, must be unique per message
    'session' : str, # typically UUID, should be unique per session
    'username' : str,
    # ISO 8601 timestamp for when the message is created
    'date': str,
    # All recognized message type strings are listed below.
    'msg_type' : str,
    # the message protocol version
    'version' : '5.0',
}
```

### FULL MSG

```python
{
    "header" : {
        "msg_id": "...",
        "msg_type": "...",
        ...
    },
    "parent_header": {},
    "metadata": {},
    "content": {},
    "buffers": [],
}
```

### THE WIRE PROTOCOL

[wire protocol spec](https://jupyter-client.readthedocs.io/en/latest/messaging.html#wire-protocol)

The above message format is only a logical representation of the contents of Jupyter messages, but does not describe the actual implementation at the wire level in zeromq. This section describes the protocol that must be implemented by Jupyter kernels and clients talking to each other over zeromq.

The reference implementation of the message spec is our Session class.

Note: This section should only be relevant to non-Python consumers of the protocol. Python consumers should import and the use implementation of the wire protocol in `jupyter_client.session.Session`

Every message is serialized to a sequence of at least six blobs of bytes:

```python
[
  b'u-u-i-d',         # zmq identity(ies)
  b'<IDS|MSG>',       # delimiter
  b'baddad42',        # HMAC signature
  b'{header}',        # serialized header dict
  b'{parent_header}', # serialized parent header dict
  b'{metadata}',      # serialized metadata dict
  b'{content}',       # serialized content dict
  b'\xf0\x9f\x90\xb1' # extra raw data buffer(s)
  ...
]
```

Note: beyond the scope to implement the HMAC / password part of the Wire protocol.
It can be left blank for no authentication required considering that a kernel is launched locally.

The front of the message is the ZeroMQ routing prefix, which can be zero or more socket identities. This is every piece of the message prior to the delimiter key <IDS|MSG>. In the case of IOPub, there should be just one prefix component, which is the topic for IOPub subscribers, e.g. execute_result, display_data.

## list of kernels

[List of Jupyter Kernels](https://github.com/jupyter/jupyter/wiki/Jupyter-kernels)

## blog posts

- [Creating Language Kernels for IPython](https://andrew.gibiansky.com/blog/ipython/ipython-kernels/)

- [Messaging in Jupyter](https://jupyter-client.readthedocs.io/en/latest/messaging.html#messaging)

## ipykernel

- [ipykernel repo](https://github.com/ipython/ipykernel/tree/master/ipykernel)

- [embed the ipykernel](https://github.com/ipython/ipykernel/blob/master/ipykernel/embed.py)
- [zmqshell](https://github.com/ipython/ipykernel/blob/master/ipykernel/zmqshell.py)

## Via a protocol layer (Zeromq to Atoms)

## Via a secondary Language (Javascript / Lua)

### Javascript

- [tslab](https://github.com/yunabe/tslab)
- [ijavascript](https://github.com/n-riesco/ijavascript)
- [itypescript](https://github.com/winnekes/itypescript) (in typescript)

### Lua

- [lua_ipython_kernel](https://github.com/neomantra/lua_ipython_kernel)
- [IPyLua](https://github.com/pakozm/IPyLua)
- [ilua](https://github.com/guysv/ilua)

### Haskell

- [Jupyter-Messages in Haskell](https://hackage.haskell.org/package/jupyter-0.9.0/docs/Jupyter-Messages.html)

## D Language

- [jupyter-wire](https://github.com/symmetryinvestments/jupyter-wire)

## kernel testing tools

- [testing kernels against message spec](https://github.com/ipython/ipython/wiki/Dev-Testing-kernels-against-message-specification)
- [jupyter kernel test](https://github.com/jupyter/jupyter_kernel_test)

## Binary Serialization

- Binary JSON
- [msgpack](https://github.com/msgpack/msgpack-c/tree/c_master)
