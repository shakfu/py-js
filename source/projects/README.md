# Python3 Max External Experiments

This folder contains some code experiments using Max with Python3. 

All of these projects can be built using cmake:

```bash
make projects
```

The projects currently fall into two groups:

## A. Core Externals (py and pyjs)

Full featured and with most most developed packaging options.

Currently implemented in the `py` directory, both of these externals are built together.


## B. Implementation and Packaging Experiments

### krait

Deferred and clocked evaluation of a python function via Max's ITM-based sequencing

### mamba

A Single-header python3 c library to nest python3 objects in other externals

### cobra

A Single-header python3 c++ library to nest python3 objects in other externals

### mpy

A proof-of-concept [micropython](https://micropython.org) external.

### mxpy

A translation of [pdpython](https://github.com/garthz/pdpython) to maxmsp

### pktpy

An external based on the [pocketpy](https://github.com/blueloveTH/pocketpy) C++17 header-only Python interpreter for game engine/apps.

### pktpy2

An external based on the new `v2.0.x` version of [pocketpy](https://github.com/blueloveTH/pocketpy) using c11 instead of c++

## C. ZeroMQ / Jupyter Experiments

These are experiments in the use of [zeromq](https://zeromq.org) in Max externals, ultimately toward an attempt to develop something like a minimal jupyter client for Max.

The dependencies can be obtained via Homebrew:

```bash
brew install zeromq czmq libsodium
```

For static compilation, the dependencies are `libzmq.a` (itself depending on `libsodium.a`) with associated headers, and also `libczmq.a`.

### zpy

The `zpy` consists of relatively thin zeromq (using the czmq wrapper) client embedded in a Max external. It talks to a corresponding server (written in python3) which is run (currently manually) in a separate process in a REQUEST-REPLY pattern.

This prototype blocks the ui thread when it is run. The next example, `ztp`, solves this issue.

### ztp

The `ztp` external, the name refers to python, threaded and zeromq, is a `zeromq` client with Max threads to make it non-blocking and it spawns a zeromq python interpreter server. This works: one thread per zmq socket as per the zmq rules.

### jmx

An attempt to build a [jupyter_client](https://jupyter-client.readthedocs.io/en/stable/messaging.html) as an external.

