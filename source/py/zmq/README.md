# ZeroMQ Experiments

This folder contains some experiments in the use of [zeromq](https://zeromq.org) in Max externals.

Note that to build you need [Xcodegen](https://github.com/yonaskolb/XcodeGen) (which can be to generate the xcode project each time (not strictly necessary but fun anyway). It can be installed via Homebrew:

```bash
brew install xcodegen
```

## common dependencies

The dependencies are `libzmq.a` (itself depending on `libsodium.a`) with associated headers.

The other libraries are not required and are just for experimentation.

## The Experiments

### zmqc

`zmqc` is short for zmq client. The a zeromq hello-world level client is embedded in a Max external. A zeromq server is run from the terminal in separate process in a  REQUEST-REPLY pattern.

This is example blocks the ui thread when it is run. The next example, `zthread`, solves this issue.

### zthread

`zthread` is `zmqc` + Max threads to make it non-blocking. This works well: one thread per zmq socket as per the rules.

### jmx

An attempt to build a [jupyter_client](https://jupyter-client.readthedocs.io/en/stable/messaging.html) as an external.
