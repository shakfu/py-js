# FAQ

## Compatibility

### Does this project work with Windows?

Yes, although Windows support came relatively recently, all python externals and also the `pktpy` externals can be compiled using the default MSVC buildsystem.

There are now several build variants for Windows: local externals linked to system Python, and relocatable builds suitable for Max packages and standalones. Refer to the README quickstart for compilation instructions.

### Does this project work with macOS?

Yes, macOS support is the most mature for both x86_64 (Intel) and arm64 (Apple silicon). Note that externals are only built natively (no univeral binaries to keep the size of externals under control).

Python3 installed from [python.org](https://www.python.org) works as expected, and even Apple installed system python3 has been tested to work on the default build and some but not all of the other build variants.

Basically, there is no intrinsic reason why it shouldn't work with python3 on your system if the python3 version >= 3.7

Refer to the README quickstart for compilation instructions.

### Can I use Homebrew python on macOS?

Definitely, if python is installed using [Homebrew](https://brew.sh) you can create externals using `make`, `make projects`, `make homebrew-ext` and `homebrew-pkg` depending on your requirement. See `make help` for some guidance on which target to build.

## Implementation

### Is the python interpreter embedded in the external or does the external basically use the host's python interpreter?

The `make default` build creates a lightweight external dynamically linked to your local python3 interpreter; other build variants such as `framework-pkg` embeds python3 into an external that is dynamically linked to a python3 interpreter which is part of the containing Max package; and another such as `framework-ext` embeds python into the external itself without any dependencies. There are other ways as well. The section, [Building self-contained Python3 Externals for Packages and Standalones](https://github.com/shakfu/py-js/tree/main/source/projects/py#building-self-contained-python3-externals-for-packages-and-standalones), gives an overview of the different approaches.

Recent work on the `mamba` single-header c library project makes it possible build relocatable python3 externals along the lines of what is possible with `py` and `pyjs`

## Installation

### Can I use two different python3 externals in the same patch?

Python3 external types which are not 'isolated' (see the [build variations](https://github.com/shakfu/py-js/tree/main/source/projects/py#build-variations) section) cannot be loaded at the same time. So for example, if you build a `framework-ext` variation, `py.mxo` and `pyjs.mxo` will not work together in the same patch, but if you built a `framework-pkg` variation of the same two externals, they should work fine without issues.

Of course, it would be considered redundant to install two different python3 external types in your project at the same time.

## Logging

### Every time I open a patch there is a some debug information in the console. Should I be concerned?

It looks like someone left @debug=on in this patch and it further may have cached some paths to related on the build system in the patch. You should be able to switch it off by setting @debug=off. If they still remain, open the `.maxpat` in question in an editor (it's a JSON file) and remove the cached paths.

You can also go to the external c file itself and hardcode DEBUG=0 if you want to switch logging off completely.

## Extensibility

### How to extend python with your own scripts and 3rd party libraries?

The easiest solution is to use an external that's linked to your system python3 installation (and not a self-contained external). This is what gets built if you run `make` or `make projects` in the root of the [py-js](https://github.com/shakfu/py-js) project. If you do it this way, you automatically get access to all of your python libraries, but you give up portability.

This release contains relocatable python3 externals which are useful for distribution in packages and standalones so it's a little bit more involved.

First note that there several ways to add code to the external:

1. The external's site-packages: `py-js/externals/py.mxo/Contents/Resources/lib/python3.X/site-packages`

2. The package `examples` folder: such as `py-js/examples/scripts`

3. Whichever path you set the patcher PYTHONPATH property to (during object creation).

For (1), if you make changes to the external by adding modules or compiled extensions then you will have to re-codesign the external. This is straightforward in `py-js`, just make sure the externals are in the `externals` folder and `make sign`.

For (2), this is just a location that's searched automatically with `load`, `read`, and `execfile` messages so it can contain dependent files.

For (3), this is just a setting that is done at the patch level so it should be straightforward. As mentioned, the extra `pythonpath` is currently only set at object creation. It should be updated when it's changed but this is something on the todo list.

## Specific Python package Compatibility

### How to get numpy to work in py-js

The easiest way is to just create an adhoc python external linked to your system python3 setup. If you have `numpy` installed there, then you should be good to go with the following caveat: the type translation system does not currently automatically cover native `numpy` `dtypes` so they would have to be converted to normal lists before they become translated to to Max `lists`. This is not a hard constraint, just not implemented yet.

You can also add your system `site-packages` to the externals pythonpath attribute.

If you need numpy embedded in a portable variation of py-js, then you have a couple of options. A py-js package build which has 'thin' externals referencing a python distribution in the `support` folder of the package is the way to go and is provided by the `homebrew-pkg` build option for example.

It is also possible to package numpy in a fully relocatable external. It used to be quite involved, and can currently only be done with non-statically built relocatable externals, but a make target has been added to do this. The make command to do this is `make install-numpy`.
