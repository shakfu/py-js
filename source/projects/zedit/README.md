# zedit: a web-based code-editor embedded in an max external

This subproject provides an example of a web-based code-editor / interactive console for the python3 externals in this project. It also embeds a python interpreter which is enabled by including mamba's single header python3 c library.

Note there is also a very nice alternative, light-weight and practical solution  to this requirement: the`py_external_editor.maxpat` abstraction in this project's `patchers` folder, which is also implemented as a bpatcher.

## Current Status

The current implementation uses [codemirror](https://codemirror.net), [xtermjs](https://github.com/xtermjs/xterm.js) and the [mongoose](https://github.com/cesanta/mongoose) embedded webserver library to interact with with them and the max external.

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
