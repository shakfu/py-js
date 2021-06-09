# ZeroMQ / Jupyter Experiments

This folder contains some experiments in the use of [zeromq](https://zeromq.org) in Max externals, ultimately toward an attempt to develop a minimal jupyter client for Max.

If the Xcode project is not already generated, you need the marvelous utility [Xcodegen](https://github.com/yonaskolb/XcodeGen) to generate the xcode project from the `project.yml` spec. I personally generate the xcode project for every build (by running `./build.sh` in the root of the subproject), since its generally quicker than opening up the xcode gui. `Xcodegen` can be installed via Homebrew:

```bash
brew install xcodegen
```

## Common Dependencies

The dependencies are `libzmq.a` (itself depending on `libsodium.a`) with associated headers.

The other libraries are not required and are just for experimentation.

## Experiments

### zmqc

`zmqc` is short for zmq client. This zeromq hello-world-level client is embedded in a Max external. To test it, the corresponding `zmqc_server` should be run from the terminal in a separate process in a  REQUEST-REPLY pattern. The `zmqc_server.c` can be compiled as follows:

```bash

clang -o zmqc_server -lzmq zmqc_server.c

```


This is example blocks the ui thread when it is run. The next example, `zthread`, solves this issue.

### zthread

`zthread` is `zmqc` + Max threads to make it non-blocking. This works: one thread per zmq socket as per the zmq rules.

### jmx

An attempt to build a [jupyter_client](https://jupyter-client.readthedocs.io/en/stable/messaging.html) as an external.

So far implementing parts of the message protocol:

- [x] generate uuid
- [x] iso861 timestamp is generated
- [x] build Max dictionary and convert to json
- [ ] convert max messages to python syntax
- [ ] ...
