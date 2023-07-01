# Quickstart

As mentioned earlier, the `py` and `pyjs` objects are the most mature and best documented of the collection. Happily, there is also no need to compile them as they are available for download, fully codesigned and notarized, from the [releases](https://github.com/shakfu/py-js/releases) section.

If you'd rather build them or any of the other externals yourself then the process is straightforward:

1. You should have a modern `python3` cpython implementation installed on your Mac: preferably either from [python.org](https://www.python.org) or from [Homebrew](https://brew.sh). Note that even system python3 provided by Apple will work in a number of cases. Python versions from 3.8 to 3.11 are tested and known to work.

2. Make sure you also have [Xcode](https://xcodereleases.com/) installed.

3. Git clone the `py-js` [repo](https://github.com/shakfu/py-js) to a path without a space and without possible icloud syncing (i.e don't clone to `$HOME/Documents/Max 8/Packages`) [?] and run the following in the cloned repo:

    ```bash
    make setup
    ```

    The above will initialize and update the required git submodules and symlink the repo to `$HOME/Documents/Max 8/Packages/py-js` to install it as a Max Package and enable you to test the externals and run the patches.

    [?] It is possible to install `py-js` directly into `$HOME/Documents/Max 8/Packages`, but it requires moving the place of compilation to a location in your filesystem that is not exposed to errors due to icloud syncing or spaces in the path. This split is possible, but it is not recommended for the purposes of this quickstart.

4. Install [cython](https://cython.org) via `pip3 install cython`, which is used for wrapping the max api and worth installing in case you want to play around or extend the max api wrapper (which is written in cython) for the `py` or `pyjs` externals.

5. To build only the `py` or `pyjs` externals, type the following in the root directory of the `py-js` project (other installation options are detailed below):

    ```bash
    make
    ```

Note that typing `make` here is the same as typing `make default` or `make all`. This will create two externals `py.mxo` and `pyjs.mxo` in your `externals` folder. These are quite small in size and are linked to your system python3 installation. This has the immediate benefit that you have access to your curated collection of python packages. The tradeoff is that these externals are dynamically linked with local dependencies and therefore not usable in standalones and relocatable Max packages.

No worries, if you need portable relocatable python3 externals for your package or standalone then make sure to read the [Building self-contained Python3 Externals for Packages and Standalones](https://github.com/shakfu/py-js#building-self-contained-python3-externals-for-packages-and-standalones) section

Open up any of the patch files in the `patchers` directory of the repo or the generated max package, and also look at the `.maxhelp` patchers to understand how the `py` and the `pyjs` objects work.

## Building Experimental Externals using Cmake

You can also use `cmake` to build **all** externals using similar methods to the `max-sdk`.

First make sure you have completed the [Quickstart](#quickstart) section above. Next you will install `cmake` if necessary and a couple of additional dependencies for some of the subprojects. Of course, skip what is already installed:

```bash
brew install cmake zmq czmq
```

Now you can build all externals (including `py` and `pyjs`) in one shot using cmake:

```bash
make projects
```

After doing the above, the recommended iterative development workflow is to make changes to the source code in the respective project and then `cd py-js/build` and `make`. This will cause cmake to only build modified projects efficiently.

Note that for some of the less developed externals and more experimental features please don't be surprised if Max seg-faults (especially if you start experimenting with the cython wrapped `api` module which operates on the c-level of the Max SDK).

Also note that for `py` and `pyjs` externals the `cmake` build method described does not yet create self-contained python externals which can be used in Max Packages and Standalones.

The following section addresses this requirement.

## Building self-contained Python3 Externals for Packages and Standalones

Currently only the `py` and `pyjs` externals are built using these methods, which rely on a python [builder manager](https://github.com/shakfu/py-js/tree/main/source/projects/py/builder). The `Makefile` in the project root provides a simplified interface to this builder. See the [Current Status of Builders](https://github.com/shakfu/py-js#current-status-of-builders) section for further information.

idx  | command                | type       | format     | py size |  pyjs size
:--: | :--------------------- | :--------- | :--------- | :------ | :----------
1    | `make static-ext`      | static     | external   | 9.0     | 8.8
2    | `make static-tiny-ext` | static     | external   | 6.7     | 6.2  [2]
3    | `make shared-ext`      | shared     | external   | 16.4    | 15.8
4    | `make shared-tiny-ext` | shared     | external   | 6.7     | 6.2  [2]
5    | `make framework-pkg`   | framework  | package    | 22.8    | 22.8 [3]

[2] These 'tiny' variants are intended to be the smallest possible portable pyjs externals. In this table, size figures are for python 3.10.x but for python 3.11.4 they increase to 8.5 MB and 8.1 respectively. Generally, external size increases with each new python version as features are added, but this is also somewhat mitigated by the removal of deprecated builtin packages and extensions. If you want to achieve the theoreticla minimal size for the `py` and `pyjs` externals, use python 3.8.x and/or a tiny variant (with a more recent version). Another option, if you need circa 1 MB size for a self-contained external, is the `pktpy` subproject in this repo.

[3] Size, in this case, is not the individual external but the uncompressed size of the package which includes patches, help files and **both** externals. This can also vary by python version used to compile the external.

This section assumes that you have completed the [Quickstart](#quickstart) above and have a recent python3 installation (python.org, homebrew or otherwise).

Again, if you'd rather not compile anything there are self-contained python3 externals which can be included in standalones in the [releases](https://github.com/shakfu/py-js/releases) section.

If you don't mind compiling (and have xcode installed) then pick one of the following options:

1. To build statically-compiled self-contained python3 externals:

    ```bash
    make static-ext
    ```

    You may also prefer the tiny variant:

    ```bash
    make static-tiny-ext
    ```

2. To build self-contained python3 exernals which include a dynamically linked libpythonX.Y.dylib:

    ```bash
    make shared-ext
    ```

    or for the corresponding tiny variant:

    ```bash
    make shared-tiny-ext
    ```    

3. To build python3 externals in a package, linked to a python installation in its `support` folder

    ```bash
    make framework-pkg
    ```

With all of the above options, a python3 source distribution (matching your own python3 version) is automatically downloaded from [python.org](https://www.python.org) with dependencies, and then compiled into a static or shared version of python3 which is then used to compile the externals.

At the end of this process you should find two externals in the `py-js/externals` folder: `py.mxo` and `pyjs.mxo`.

Although the above options deliver somewhat different products (see below for details), with options (1) and (2) the external 'bundle' contains an embedded python3 interpreter with a zipped standard library in the `Resources` folder and also has a `site-packages` directory for your own code; with option (3), the externals are linked to, and have been compiled against, a relocatable python3 installation in the `support` folder.

Depending on your choice above, the python interpreter in each external is either statically compiled or dynamically linked, and in all three cases we have a self-contained and relocatable structure (external or package) without any non-system dependencies. This makes it appropriate for use in Max Packages and Standalones.

There are other [build variations](#build-variations) which are discussed in more detail below. You can always see which ones are available via typing `make help` in the `py-js` project folder:

```bash
$ make help

>>> general
make projects             : build all subprojects using standard cmake process

>>> pyjs targets
make                      : non-portable pyjs externals linked to your system
make homebrew-pkg         : portable package w/ pyjs (requires homebrew python)
make homebrew-ext         : portable pyjs externals (requires homebrew python)
make shared-pkg           : portable package with pyjs externals (shared)
make shared-ext           : portable pyjs externals (shared)
make shared-tiny-ext      : tiny portable pyjs externals (shared)
make static-ext           : portable pyjs externals (static)
make static-tiny-ext      : tiny portable pyjs externals (static)
make framework-pkg        : portable package with pyjs externals (framework)
make framework-ext        : portable pyjs externals (framework)
make relocatable-pkg      : portable package w/ more custom options (framework)

>>> python targets
make python-shared        : minimal shared python build
make python-shared-ext    : minimal shared python build for externals
make python-shared-pkg    : minimal shared python build for packages
make python-static        : minimal statically-linked python build
make python-framework     : minimal framework python build
make python-framework-ext : minimal framework python build for externals
make python-framework-pkg : minimal framework python build for packages
make python-relocatable   : custom relocatable python framework build
```

## Automated Test of Build Variations

If you would like to see which build variations are compatible with your current setup, there's an automated test which attempts to compile all build variations in sequence and will log all results to a `logs` directory:

```bash
make test
````

This can take a long time, but it is worth doing to understand which variations work on your particular setup.

If you want to test or retest one individual variation, just prefix `test-` to the name of variation as follows:

```bash

make test-shared-pkg
```

## Using Self-contained Python Externals in a Standalone

If you have downloaded any pre-built externals from [releases](https://github.com/shakfu/py-js/releases) or if you have built self-contained python externals as per the methods above, then you should be ready to use these in a standalone.

To release externals in a standalone they must be codesigned and notarized. To this end, there are scripts in `py-js/source/projects/py/scripts` to make this a little easier.

## py.mxo

If you included `py.mxo` as an external in your standalone, then you should have no issue as Max will install it automatically during its build-as-standalone process.

You can test if it works without issues by building either of these two example patcher documents, included in `py-js/patchers`, as a max standalone:

1. `py_test_standalone_info_py.maxpat`

2. `py_test_standalone_only_py.maxpat`

Open the resulting standalone and test that the `py` object works as expected.

To demonstrate the above, a pre-built standalone that was built using exactly the same steps as above is in the releases section: `py_test_standalone_demo.zip`.

## pyjs.mxo

If you opted to include `pyjs.mxo` as an external in your standalone, then it may be a litte more involved:

You can first test if it works without issues by building 'a max standalone' from the `py_test_standalone_only_pyjs.maxpat` patcher whici is included in `py-js/patchers`.

Open the resulting standalone and test that the `pyjs` object works as expected. If it doesn't then try the following workaround:

To fix a sometimes recurrent issue where the standalone build algorithm doesn't pick up `pyjs.mxo`: if you look inside the built standalone bundle, `py_test_standalone_only_pyjs.app/Contents/Resources/C74/externals` you may not find `pyjs.mxo`. This is likely a bug in Max 8 but easily resolved. Fix it by manually copying the `pyjs.mxo` external into this folder and then copy the `javascript` and `jsextensions` folders from the root of the `py-js` project and place them into the `pyjs_test_standalone.app/Contents/Resources/C74` folder. Now re-run the standalone app again and now the `pyjs` external should work. A script is provided in `py-js/source/projects/py/scripts/fix-pyjs-standalone.sh` to do the above in an automated way.

Please read on for further details about what the py-js externals can do.

Have fun!