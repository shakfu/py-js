# ZeroMQ / Jupyter Experiments

This folder contains some experiments in the use of [zeromq](https://zeromq.org) in Max externals, ultimately toward an attempt to develop somethink like a minimal jupyter client for Max.

## Building

If the Xcode project is not already generated, you need the marvelous utility [Xcodegen](https://github.com/yonaskolb/XcodeGen) to generate the xcode project from the `project.yml` spec. I personally generate the xcode project for every build (by running `./build.sh` in the root of the subproject), since its generally quicker than opening up the xcode gui. `Xcodegen` can be installed via Homebrew:

```bash
brew install xcodegen
```

## Common Dependencies

The dependencies can be obtained via Homebrew:

```bash
brew install zeromq czmq libsodium
```

For static compilation, the dependencies are `libzmq.a` (itself depending on `libsodium.a`) with associated headers, and also `libczmq.a`.

## Experiments

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
