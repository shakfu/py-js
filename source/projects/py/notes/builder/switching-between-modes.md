# Switching between Modes


## Mode A: `py-js` symlinked to `$HOME/Documents/Max 8/Packages/py-js`


Ensure that py-js is not in a path with a space in it.

```bash
core.project.is_symlinked = True

py-js/source/py/targets/common.xcconfig
	PYJS_BUILD_ROOT = $(SRCROOT)/../build
	DSTROOT = $(SRCROOT)/../../../externals (default)
	
```

subdependencies are built in `py-js/source/py/targets/build/lib`



## Mode B: `py-js` copied to `$HOME/Documents/Max 8/Packages/py-js`


```bash
core.project.is_symlinked = False

py-js/source/py/targets/common.xcconfig
	PYJS_BUILD_ROOT = $(HOME)/.build_pyjs
	DSTROOT = $(PYJS_BUILD_ROOT)/externals
	
```

subdependencies are built in `$HOME/.build_pyjs/lib`

targets which are affected:

- `framework-ext` (location of `Python.framework`) changes between modes
- `shared-ext`: `lib` from `python-shared`
- `static-ext`: `lib` from `python-static`