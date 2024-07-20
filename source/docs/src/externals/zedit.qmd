# zedit: a web-based code-editor embedded in an max external

This subproject provides an example of a python3 external whith the following features:

- Embeds a python interpreter via the mamba single header python3 c library
- Embeds the c-based mongoose webserver
- Provides a web-based code-editor 
- Provides a web-based interactive terminal

This ui in this project is very powerfull using typescript/javascript-based web technologies. This is probably overkill for a python3 external, but illustratrive nonetheless of the potential achieved by embedding an small webserver in an external.

For simpler exmaples of user interfaces which interact with python3 externals see the `patchers/bpatchers_ui` and the `patchers/bpatcher_py` folders.

## Usage 

You have to setup the project before being able to use it:

```bash
cd py-js/source/projects/zedit/web
make
```
then return to the root of the project

```bash
cd py-js
make dev
```

It is possible to build the external and embed everything into a bundle by building the project in Release mode.

This will build and deploy the javascript / typescript code to the `zedit/web/public` folder. There is folder called `zedit/webroot` which is a symlink to the above public folder.

Note that the `n4m` folder contains an alternative impplementation which can be ignored for the time being as it is a work-in-pgress


## Current Status

The current implementation uses [codemirror](https://codemirror.net), [xtermjs](https://github.com/xtermjs/xterm.js) and the [mongoose](https://github.com/cesanta/mongoose) embedded webserver library to interact with the user and the max external.

Current features are:

- Embedded webserver running in a separate thread
- Python3 web-editor with dark theme, syntax highlighting, auto-complete, and search, ..
- Basic web terminal 
- Commands
	- Help: open a list of keboard commands
	- Open: open a file from the client side
	- Save: dump contents to external
	- Run: dump and run contents

See `zedit.maxhelp` for a demo of the external launching the embedded webserver and running the code-mirror web-editor.

There is also a node-for-max variation on (1) using expressjs as the webserver.


## Future Direction

- use [jquery.terminal](https://github.com/jcubic/jquery.terminal)
