
# FAQ


## Compatibility

### Is this macOS only?

This project is macOS x86_64 (intel) compatible currently.

### What about compatibility with Apple Silicon?

There is an `M1` branch for the project under-development to provide this compatability and shift the build project's build system to cmake.

### What about compatibility with Windows?

There's no particular reason why this project doesn't work in windows except that I don't develop in windows any longer. Feel free to send pull requests to project though.

### Does it only work with Homebrew python?

It works by default with [Homebrew](https://brew.sh) installed python but it can also work with python compiled from source as well.

There is no instrinsic reason why it shouldn't work with other python3 installations on your system.


## Implementation

### Does it embed python into the external or is the external connecting to the local python installation?

The default build creates a lightweight external linked to your local homebrew python3; another variation embeds python3 into an external linked to python3 which resides in a Max package; and another variation embeds python into the external itself without an dependencies. There are other ways as well. The project [README](https://github.com/shakfu/py-js) gives an overview of the differences between the different approaches.


## Logging

### Every time I open a patch there is a some debug information in the console. Should I be concerned?

It looks like someone left @debug=on in this patch and it further may have cached some paths to related on the build system in the patch. You should be able to switch it off by setting @debug=off.


## Extensibility

### How to extend python with your own scripts and 3rd party libraries?

The easiest solution is not to use a self-contained external and use an external that's linked to your system python3 installation. This is what gets built if you run ./build.sh in the root of the [py-js](https://github.com/shakfu/py-js) project. If you do it this way, you automatically get access to all of your python libraries, but you give up portability.

This release contains relocatable python3 externals which are useful for distribution in packages and standalones so it's a little bit more involved.

First note that there several ways to add code to the external:

1. The external's site-packages: `py-js/externals/py.mxo/Contents/Resources/lib/python3.9/site-packages`

2. The package script folder: `py-js/examples/scripts`

3. Whichever path you set the patcher PYTHONPATH property to (during object creation).

For (1), I have tested pure python scripts which should work without re-codesigning the externals, but if you add compiled extensions, then I think you have to re-codesign the external. Check out my [maxutils](https://github.com/shakfu/maxutils) project for help with that.

For (2), this is just a location that's searched automatically with `load`, `read`, and `execfile` messages so it can contain dependent files.

For (3), this is just setting that is done at the patch level so it should be straightforward. As mentioned, the extra pythonpath is currently only set at object creation. It should be updated when changed but this is something on the todo list.


## Specific Python package Compatibility

### How to get numpy to work in py-js

The easiest way is to just create an adhoc python external linked to your system python3 setup. If you have numpy installed there, then you should be good to go with the following caveat: the type translation system does not currently automatically cover native numpy dtypes so they would have to be converted to normal lists before they become translated to to Max lists. This is not a hard constraint, just not implemented yet.

You can also add your system `site-packages` to the externals pythonpath attribute.

If you need numpy embedded in a portable variation of py-js, then you have a couple of options. A py-js package build which has 'thin' externals referencing a python distribution in the `support` folder of the package is the way to go and is provided by the `bin-homebrew-pkg` build option for example.

It is also possible to package numpy in a full relocatable external, it's quite involved, and cannot currently only be done with non-statically built relocatable externals. The releases section has an example of this.