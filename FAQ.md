
# FAQ

## Compatibility

### Is this macOS only?

This project is currently only macOS x86_64 (Intel) or arm64 (Apple silicon) compatible.

### What about compatibility with Windows?

There's no particular reason why this project shouldn't work in windows except that I don't develop in windows any longer. Feel free to send pull requests to help make this happen.

### Does it only work with Homebrew python?

It used to work, by default, with [Homebrew](https://brew.sh) installed python but current versions don't require Homebrew python. Python3 installed from python.org works as expected, and even Apple installed system python3 has been tested to work on the default build and some but not all of the other build variants.

Basically, there is no intrinsic reason why it shouldn't work with python3 on your system if the python3 version >= 3.7

## Implementation

### Is the python interpreter embedded in the external or does the external basically use the host's python interpreter?

The default build creates a lightweight external dynamically linked to your local python3 interpreter; another variant embeds python3 into an external that is dynamically linked to a python3 interpreter which is part of the containing Max package; and another embeds python into the external itself without any dependencies. There are other ways as well. The project [README](https://github.com/shakfu/py-js) gives an overview of differences between approaches.

## Installation

### Can I use two different python3 externals in the same patch?

Python3 external types which are not 'isolated' (see the [build variations](README.md#build-variations) section) cannot be loaded at the same time. So for example, if you build a `framework-ext` variation, `py.mxo` and `pyjs.mxo` will not work together in the same patch, but if you built a `framework-pkg` variation of the same two externals they should work fine without issues.

Of course, it would be considered redundant to install two different python3 externals in your project.

## Logging

### Every time I open a patch there is a some debug information in the console. Should I be concerned?

It looks like someone left @debug=on in this patch and it further may have cached some paths to related on the build system in the patch. You should be able to switch it off by setting @debug=off. If they still remain, open the `.maxpat` in question in an editor (it's a JSON file) and remove the cached paths.

You can also go to the external c file itself and hardcode DEBUG=0 if you want to switch logging off completely.

## Extensibility

### How to extend python with your own scripts and 3rd party libraries?

The easiest solution is not to use a self-contained external and use an external that's linked to your system python3 installation. This is what gets built if you run ./build.sh in the root of the [py-js](https://github.com/shakfu/py-js) project. If you do it this way, you automatically get access to all of your python libraries, but you give up portability.

This release contains relocatable python3 externals which are useful for distribution in packages and standalones so it's a little bit more involved.

First note that there several ways to add code to the external:

1. The external's site-packages: `py-js/externals/py.mxo/Contents/Resources/lib/python3.X/site-packages`

2. The package script folder: `py-js/examples/scripts`

3. Whichever path you set the patcher PYTHONPATH property to (during object creation).

For (1), I have tested pure python scripts which should work without re-codesigning the externals, but if you add compiled extensions, then I think you have to re-codesign the external. Check out my [maxutils](https://github.com/shakfu/maxutils) project for help with that.

For (2), this is just a location that's searched automatically with `load`, `read`, and `execfile` messages so it can contain dependent files.

For (3), this is just a setting that is done at the patch level so it should be straightforward. As mentioned, the extra pythonpath is currently only set at object creation. It should be updated when it's changed but this is something on the todo list.

## Specific Python package Compatibility

### How to get numpy to work in py-js

The easiest way is to just create an adhoc python external linked to your system python3 setup. If you have numpy installed there, then you should be good to go with the following caveat: the type translation system does not currently automatically cover native numpy dtypes so they would have to be converted to normal lists before they become translated to to Max lists. This is not a hard constraint, just not implemented yet.

You can also add your system `site-packages` to the externals pythonpath attribute.

If you need numpy embedded in a portable variation of py-js, then you have a couple of options. A py-js package build which has 'thin' externals referencing a python distribution in the `support` folder of the package is the way to go and is provided by the `bin-homebrew-pkg` build option for example.

It is also possible to package numpy in a full relocatable external, it's quite involved, and can currently only be done with non-statically built relocatable externals.

The build used to auto-detect numpy in a python used to build the external, but now it is a manual process and the limited numpy support has to be enabled at the Makefile and cmake level.

