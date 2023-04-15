# zedit

This subproject is all about building a web-based code-editor for the python3 externals in this project.

While this is very interesting and fun to experiment with, there is also a very nice light-weight and practical approach to this problem, the `py_external_editor.maxpat` abstraction in this project's `patchers` folder, which is also implemented as a bpatcher.

## Current Status

There are 3 implementations in descending order of maturity:

1. codemirror-mongoose web-editor (c + js)

This implementation uses [codemirror](https://codemirror.net) and the [mongoose](https://github.com/cesanta/mongoose) embedded webserver library to embed a webserver in a max external and display on demand in an external browser.

Current features are:

- Embedded webserver running in a separate thread
- Python3 web-editor with dark theme, syntax highlighting, auto-complete, and search, ..
- Commands
	- Help: open a list of keboard commands
	- Open: open a file from the client side
	- Save: dump contents to external
	- Run: dump and run contents

See `zedit.maxhelp` for a demo of the external launching the embedded webserver and running the code-mirror web-editor.

2. monaco-mongoose web-editor (c + react js)

This uses the same embedded webserver as (1) but substitutes the code-editor component for the react-based [monaco-editor](https://github.com/suren-atoyan/monaco-react) (same editor as ms visual code).


3. node-for-max proof-of-concept (js)

This is a node-for-max variation on (1) using expressjs as the webserver.



## Future Direction


- use [jquery.terminal](https://github.com/jcubic/jquery.terminal)
