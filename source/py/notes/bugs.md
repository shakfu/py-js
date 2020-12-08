# Challenging Bugs


## Gettext Dependency


## Methods Tests 2

Problem: the gettext dependency
    - try to build python without gettext installed first
    - need to find the local location of the `gettext/lib` automatically


Test results of py_test_standalone.maxpat with the following method

- [SUCCESS] bin-homebrew-sys (default)
    /usr/local/opt/python@3.8/Frameworks/Python.framework/Versions/3.8/lib/python3.8/os.py

- [SUCCESS] bin-homebrew-pkg
    "/Users/xx/Documents/Max 8/Packages/py/externals/pyjs.mxo/Contents/MacOS/../../../../support/python3.8/lib/python38.zip/os.py"


- [SUCCESS | FAIL] bin-homebrew-ext
    "/Users/sa/Documents/Max 8/Packages/py/externals/py.mxo/Contents/MacOS/../Frameworks/python3.8/lib/python38.zip/os.py"

    - pyjs: build succeeds but does not work

- [FAIL] bin-framework-pkg
    - py: no build error given but crashes with test
    - pyjs: no build error given but crashes with test

- [FAIL] bin-framework-ext
    - py: no build error given but does not load external
    - pyjs: no build error given but fails to produce expected value

- [FAIL] bin-resource-ext - build not found

- [FAIL] src-framework-pkg:
    - py (Failure to load) build-error: zipimport.ZipImportError: can't decompress data; zlib not available
    - pyjs (Failure to load)

- [FAIL] src-framework-ext
    build-error: zipimport.ZipImportError: can't decompress data; zlib not available
    - py: failure to load
    - pyjs: failure to load

- [FAIL] src-shared-pkg
    failed to load

- [FAIL] src-shared-ext

- [PARTIAL] src-static-pkg
    - incorrectly references build rather than target dir

- [FAIL] src-static-ext: fatal error: 'Python.h' file not found


SOLUTIONS:
    
    - [ ] check gettext dependency is not causing build failures (likely)
    src-static-ext build ok but then fails to load with the libintl dependency,
    so problem is pretty much to do with libintl/gettext issue.





## Numpy

Exclusively in the case for c-based 3-rd party python modules (such as the matrix library, Numpy), I have no issue importing them in the first patch that uses them, but if I close the patch (and cleanup my python objects), they are somehow not cleaned up properly and may cause unpredictable behavior: this could be returning nothing at import which is no biggie, but in the particular case of Numpy, it actually crashes Max (and defies any attempt to catch this any exceptions).

It turns out that this is actually a bug in python for embedded applications (which I wasn't aware of, and which is being worked on and may be fixed in future versions.

Therefore, the only thing that I can do right now is to stop users from 'reloading' c-based python extensions after a patch is closed which first uses them successfully, and ask them kindly to restart Max.
Therefore, is there any API method or function, which provides some meta information about whether Max has been freshly started or restarted or a count of patches which have been opened and closed, etc..

My basic idea is have some kind of flag_file which is available

```python

if not flag_file.exists:
	import normally
	touch flag_file

else if flag_file.exists:
	block imports
	error("cannot import c-extensions, restart max to use")
	remove flag_file


