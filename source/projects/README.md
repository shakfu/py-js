# Python3 Max External Experiments

This folder contains some code experiments using Max with Python3. 

All of these projects can be built using cmake:

```bash
make cmake
```

The projects currently fall into two groups:

## A. Core Externals (py and pyjs)

Full featured and with most most developed packaging options.

Currently implemented in the `py` directory, both of these externals are built together.

## B. Implementation and Packaging Experiments

### cobra

Deferred and clocked evaluation of a python function via Max's ITM-based sequencing

### mamba

A Single-header python3 c library to nest python3 objects in other externals

### krait

A Single-header python3 c++ library to nest python3 objects in other externals

### mxpy

A translation of [pdpython](https://github.com/garthz/pdpython) to maxmsp

## C. ZeroMQ / Jupyter Experiments

These are experiments in the use of [zeromq](https://zeromq.org) in Max externals, ultimately toward an attempt to develop something like a minimal jupyter client for Max.

The dependencies can be obtained via Homebrew:

```bash
brew install zeromq czmq libsodium
```

For static compilation, the dependencies are `libzmq.a` (itself depending on `libsodium.a`) with associated headers, and also `libczmq.a`.

### zpy

The `zpy` consists of relatively thin zeromq (using the czmq wrapper) client embedded in a Max external. It talks to a corresponding server (written in python3) which is run (currently manually) in a separate process in a  REQUEST-REPLY pattern.

The `zpy` external is compiled as follows:

```bash

./build.sh

```

This prototype blocks the ui thread when it is run. The next example, `zthread`, solves this issue.

### zthread

`zthread` is a `zeromq` client + Max threads to make it non-blocking. This works: one thread per zmq socket as per the zmq rules.

### zpy3 (planned)

This planned project will apply the threading logic obtained in `zthread` to the `zpy` project to sidestep ui-thread blocking.

### jmx

An attempt to build a [jupyter_client](https://jupyter-client.readthedocs.io/en/stable/messaging.html) as an external.

So far implementing parts of the message protocol:

- [x] generate uuid
- [x] iso861 timestamp is generated
- [x] build Max dictionary and convert to json
- [ ] convert max messages to python syntax
- [ ] ...
