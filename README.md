# py-js: python3 objects for max

Simple (and extensible) [python3](https://www.python.org) externals for [MaxMSP](https://cycling74.com).

Cross-platform: currently builds 'natively' on macOS (`x86_64` or `arm64`) and Windows (MSVC) with Python versions 3.8 to 3.13 inclusive.

repo - <https://github.com/shakfu/py-js>

pdf documentation - [Python3-Externals-for-Max-MSP.pdf](https://github.com/shakfu/py-js/tree/main/source/docs/_book/Python3-Externals-for-Max-MSP.pdf)

ai-enabled deep wiki - [py-js deep wiki](https://deepwiki.com/shakfu/py-js)

[![py-js py.maxhelp](./media/py-maxhelp.png)](patchers/py_test_anything.maxpat)

## Overview

This project started out as an attempt (during a covid-19 lockdown) to develop a basic python3 external for Max/MSP. It then evolved into an umbrella project for exploring different ways of using python3 in Max/MSP.

Along the way, a number of externals (17 so far) have been developed for use in a live Max environment. In most cases, each external can be built on its own via `make <external-name>` or as part of a related group (see below and click on the linked names of the externals in the below tables to see more detailed documentation):

**Python3 Core Externals:**

name       | sdk        | lang   | description
:--------- | :--------- | :----: | :---------------------------------------------------
[py]       | max-sdk    | c      | well-featured, many packaging options + [cython](https://cython.org) api
[pyjs]     | max-sdk    | c      | js/v8-friendly -- written as a Max javascript-extension

These two externals have many [build options](https://github.com/shakfu/py-js/tree/main/source/projects/py#building-self-contained-python3-externals-for-packages-and-standalones) covering almost all deployment scenarios, but if you just want to get up and running quickly use the cmake option `-DBUILD_PYTHON3_CORE_EXTERNALS=ON` or just `make core`.

**Python3 Experimental Externals:**

name       | sdk        | lang   | description
:--------- | :--------- | :----: | :---------------------------------------------------
[mamba]    | max-sdk    | c      | single-header c library to nest a python3 interpreter in any external
[cobra]    | max-sdk    | c++    | single-header c++ library to nest a python3 interpreter in any external
[pyx]      | maxcpp6    | c++    | uses [cobra] with Graham Wakefield's [maxcpp](https://github.com/grrrwaaa/maxcpp)
[mpyx]     | min-api    | c++    | single-header c++ library to nest a python3 interpreter in [min-api](https://github.com/Cycling74/min-api) externals
[krait]    | max-sdk    | c      | python3 external providing deferred and clocked function execution
[mxpy]     | max-sdk    | c      | translation of [pdpython](https://github.com/shakfu/pdpython) into Max
[zedit]    | max-sdk    | c      | web-based python editor using [codemirror](https://codemirror.net) and the [mongoose](https://github.com/cesanta/mongoose) embedded webserver.
[pymx]     | min-api    | c++    | concise python3 external, modern, using [pybind11](https://github.com/pybind/pybind11) and [min-api](https://github.com/Cycling74/min-api)
[xpyc]     | max-sdk    | c      | uses [xpc](https://developer.apple.com/documentation/xpc?language=objc) for inter-process communication with a python service

The experimental subgroup can be built as a whole with the cmake option, `-DBUILD_PYTHON3_EXPERIMENTAL_EXTERNALS=ON`, or just `make experimentals`.

**Alternative Python Implementation Externals:**

name       | sdk        | lang   | description
:--------- | :--------- | :----: | :---------------------------------------------------
[pktpy]    | max-sdk    | c++    | uses v.1.4.6 of the [pocketpy](https://github.com/blueloveTH/pocketpy) single-header c++ library
[pktpy2]   | max-sdk    | c      | uses the newer v2.0.5 [pocketpy](https://github.com/blueloveTH/pocketpy) c11 library
[mpy]      | max-sdk    | c      | a proof-of-concept embedding [micropython](https://github.com/micropython/micropython)

The two pocketpy variants can be built with the cmake option, `-DBUILD_POCKETPY_EXTERNALS=ON` or `make pocketpy`, whereas the external based on micropython, `mpy` is not enabled by default since it is still in early stages and more of a proof-of-concept to embed micropython in an external. To build it use the `-DBUILD_MICROPYTHON_EXTERNAL=ON` option with cmake or `make mpy`.

**Networking Externals:**

name       | sdk        | lang   | description
:--------- | :--------- | :----: | :---------------------------------------------------
[ztp]      | max-sdk    | c      | non-blocking communications with zeromq + threads + spawned python
[jmx]      | max-sdk    | c      | explore how to embed a [jupyter client](https://jupyter-client.readthedocs.io/en/stable/messaging.html) or [kernel](https://github.com/jupyter-xeus/xeus-python) in an external
[zpy]      | max-sdk    | c      | uses [zeromq](https://zeromq.org) for 2way-comms with an external python process

Note: networking (zmq-based) externals are not enabled by default since they require zeromq libraries to be installed. To build them use the `-DBUILD_NETWORKING_EXTERNALS=ON` option with cmake or just `make net`

The common objective in these externals is to help use and distribute python code and libraries in Max applications. Many can be considered experimental, with 80% of development time going to the first two externals (`py` and `pyjs`), and `py` receiving the most recent attention, due to ongoing development of its builtin `api` module which uses cython to wrap a [growing subset of the Max c-api](https://github.com/shakfu/py-js/blob/main/source/projects/py/api.md).

At the time of this writing, and since the switch to [max-sdk-base](https://github.com/cycling74/max-sdk-base), the project has the following compatibility profile:

- **MacOS**: both x86_64 and Apple Silicon compatible. Note that the project intentionally only produces 'native' (`x86_64` xor `arm64`) externals with no current plans for 'fat' or universal externals to serve both architectures. You can download codesigned, notarized `x86_64`-based and `arm64`-based python3 externals from the [releases](https://github.com/shakfu/py-js/releases) section.

- **Windows**: windows support was provided relatively recently, with currently all Python3 externals and also the `pktpy` projects building without issues on Windows. The only caveat is that as of this writing python3 externals are dynamically linked to the local Python3 `.dll` and are therefore not relocatable. One idea to overcome this constraint is to include the external's dependencies in the 'support' folder. This will hopefully be addressed in future iterations. The `pktpy` externals, however, are fully portable and self-contained.

The [Quickstart](#quickstart) section below covers general setup for all of the externals, and will get you up and running with the `py` and `pyjs` externals. More details are provided in the [py] documentation section: the [Building Experimental Externals using Cmake](https://github.com/shakfu/py-js/tree/main/source/projects/py#building-experimental-externals-using-cmake) section provides additional info to build the other remaining externals, and the [Building self-contained Python3 Externals for Packages and Standalones](https://github.com/shakfu/py-js/tree/main/source/projects/py#building-self-contained-python3-externals-for-packages-and-standalones) section covers more advanced building and deployment scenarios for the `py` and `pyjs` externals with details about their many build variations available via a custom python-based build system which was specifically developed to cater for different scenerios of packaging and deploying the externals in Max packages and standalones.

If you are interested in more detailed project-specific documentation, please refer to the links in the [Overview](#overview) section which link to respective project folders in the `py-js/source/projects` section.

Please feel free to ask questions or make suggestions via the project's github issue tracker.

## Quickstart

This repo has a git submodule dependency with [max-sdk-base](https://github.com/cycling74/max-sdk-base). This is quite typical for Max externals.

This means you should `git clone` as follows:

```sh
git clone https://github.com/shakfu/py-js.git
git submodule init
git submodule update
```

or more concisely:

```sh
git clone --recursive https://github.com/shakfu/py-js.git
```

### MacOS

As mentioned earlier, the `py` and `pyjs` objects are the most mature and best documented of the collection. Happily, there is also no need to compile them as they are available for download, fully codesigned and notarized, from the [releases](https://github.com/shakfu/py-js/releases) section.

If you'd rather build them or any of the other externals yourself then the process is straightforward:

1. You should have a modern `python3` cpython implementation installed on your Mac: preferably either from [python.org](https://www.python.org) or from [Homebrew](https://brew.sh). Note that even system python3 provided by Apple will work in a number of cases. Python versions from 3.8 to 3.13 are tested and known to work.

2. Make sure you also have [Xcode](https://xcodereleases.com/) installed.

3. Git clone the `py-js` [repo](https://github.com/shakfu/py-js) as per the above method to a path without a space and without possible icloud syncing (i.e don't clone to `$HOME/Documents/Max 8/Packages`) [?] and run the following in the cloned repo:

    ```bash
    make setup
    ```

    The above will initialize and update the required git submodules and symlink the repo to `$HOME/Documents/Max 8/Packages/py-js` to install it as a Max Package and enable you to test the externals and run the patches.

    [?] It is possible to install `py-js` directly into `$HOME/Documents/Max 8/Packages`, but it requires moving the place of compilation to a location in your filesystem that is not exposed to errors due to icloud syncing or spaces in the path. This split is possible, but it is not recommended for the purposes of this quickstart.

4. Optionally, install [cython](https://cython.org) via `pip3 install cython`, if you want to make changes to the cython-based `api.pyx` module, which wraps the Max c-api.

5. Any one of the named externals can be built individually via `make <name>` in the root directory of the `py-js` project, otherwise to build both the `py` and `pyjs` externals, type the following (other installation options are detailed below):

    ```sh
    make
    ```

    Note that typing `make` here is the same as typing `make default` or `make all`. This will create two externals `py.mxo` and `pyjs.mxo` in your `externals` folder. These are relatively small in size and are linked to your system python3 installation. This has the immediate benefit that you have access to your curated collection of existing python packages.

    The `make` or `make default` command bypasses an intermediate buildsystem and builds the two core externals via Xcode

    Another build option for core externals is to use `cmake` as an intermediate build system to drive Xcode builds:

    ```sh
    make core
    ```

The `make`, or `make core` methods of building core the externals are generally very fast and produce externals which have access to python libraries of the local python system referemced during compilation. The tradeoff is that since the external are dynamically linked with local dependencies, they are therefore not usable in standalones and relocatable Max packages.

No worries, if you need portable relocatable python3 externals for your package or standalone with more granular build options such as specifying the python version and the type of build then make sure to read the [Building self-contained Python3 Externals for Packages and Standalones](https://github.com/shakfu/py-js#building-self-contained-python3-externals-for-packages-and-standalones) section.

In any case, open up any of the patch files in the `patchers` directory of the repo or the generated Max package, and look at the `.maxhelp` patchers to understand how the `py` and the `pyjs` objects work.

### Windows

Since Windows support still is relatively new, no releases have been made pending further testing.

Currently, the externals which are enabled by default in this project can be built with only a few requirements:

1. Install [Visual Studio Community Edition](https://visualstudio.microsoft.com/vs/community/) or use the commercial versions as you like.

2. Install [Python3 for Windows](https://www.python.org/downloads/windows) from python.org

3. (Optional) since Visual Studio has its captive cmake, [you can use that](https://stackoverflow.com/questions/70178963/where-is-cmake-located-when-downloaded-from-visual-studio-2022), but it is preferable to [install cmake](https://cmake.org/download/#latest) independently.

After installation of the above you can now build the externals inside your `Documents/Max (8 or 9)/Packages` folder. The entails opening a terminal inside this folder, and then type:

```sh
git clone --recursive https://github.com/shakfu/py-js
cd py-js
```

If you have a working `make.exe` executable on your path then typing `make core` should build the core externals otherwise type the following:

```sh
mkdir build
cd build
cmake .. -DBUILD_PYTHON3_CORE_EXTERNALS=ON
cmake --build . --config Release
```

Now open one of the `.maxhelp` files or any of the files in the `patchers` folders to see how things work.

### Building Externals using Cmake

You can use `cmake` to build **all** externals using similar methods to the `max-sdk`.

First make sure you have completed the [Quickstart](#quickstart) section above. Next you will install `cmake` if necessary and a couple of additional dependencies for some of the subprojects. Of course, skip what is already installed:

```sh
brew install cmake
```

Now you can build almost all the externals (including `py` and `pyjs`) in one shot using cmake:

```sh
make projects
```

This builds all of the externals *except* for the networking variants, since they have some additional dependencies related to the lightweight networking [zeromq](https://zeromq.org) library.

If you would like to build them first install:

```sh
brew install zmq
```

Then

```sh
make net
```

Look at the `Makefile` and the root `CMakeLists.txt` for further specialized build options.

After doing the above, the recommended iterative development workflow is to make changes to the source code in the respective project and then `cd py-js/build` and `cmake --build .`. This will cause cmake to only build modified projects efficiently.

Note that for some of the less developed externals and more experimental features please don't be surprised if Max occassionaly segfaults (especially if you start experimenting with the the more experimental parts of the cython wrapped `api` module which operates on the c-level of the Max SDK).

Also note that for `py` and `pyjs` externals the `cmake` build method described does not yet create self-contained python externals which can be used in Max Packages and Standalones. The [Building self-contained Python3 Externals for Packages and Standalones](https://github.com/shakfu/py-js/tree/main/source/projects/py#building-self-contained-python3-externals-for-packages-and-standalones) section addresses this requirement.

Visit the project-specific links above for more detailed documentation.

## Related projects

- [relocatable-python](https://github.com/gregneagle/relocatable-python): A tool for building standalone relocatable Python.framework bundles. (used in this project)

- [python-build-standalone](https://github.com/indygreg/python-build-standalone): Produce redistributable builds of Python. (Interesting but not used in this project)

- [Python Apple Support](https://github.com/beeware/Python-Apple-support): A meta-package for building a version of Python that can be embedded into a macOS, iOS, tvOS or watchOS project. (directly inspired static linking approach)

- [python-cmake-buildsystem](https://github.com/python-cmake-buildsystem/python-cmake-buildsystem): A cmake buildsystem for compiling Python. Not currently used in this project, but may be used in the future.

- [py2max](https://github.com/shakfu/py2max) : using python3 with Max in an offline capacity to generate max patches.

- [maxutils](https://github.com/shakfu/maxutils) : scripts and utilities to help with codesigning and notarization of Max standalones and externals.

- [pocketpy](https://github.com/blueloveTH/pocketpy): C++17 header-only Python interpreter for game engines.

- [micropython](https://github.com/micropython/micropython): a lean and efficient Python implementation for microcontrollers and constrained systems

## Prior Art and Thanks

![py-js testing](./media/xkcd-python-environment.png)

I was motivated to start this project because I found myself recurrently wanting to use some python libraries or functions in Max.

Looking around for a python max external I found the following:

- Thomas Grill's [py/pyext – Python scripting objects for Pure Data and Max](https://grrrr.org/research/software/py/) is the most mature Max/Python implementation and when I was starting this project, it seemed very promising but then I read that the 'available Max port is not actively maintained.' I also noted that it was written in C++ and that it needed an additional [c++ flext](http://grrrr.org/ext/flext) layer to compile. I was further dissuaded from diving in as it supported, at the time, only python 2 which seemed difficult to swallow considering Python2 is basically not developed anymore. Ironically, this project has become more active recently, and I finally was persuaded to go back and try to compile it and finally got it running. I found it to be extremely technically impressive work, but it had probably suffered from the burden of having to maintain several moving dependencies (puredata, max, python, flext, c++). The complexity probably put off some possible contributors which would have made the maintenance of the project easier for Thomas. In any case, it's an awesome project and it would be great if this project could somehow help py/ext in some way or the other.

- [max-py](https://github.com/njazz/max-py) -- Embedding Python 2 / 3 in MaxMSP with `pybind11`. This looks like a reasonable effort, but only 9 commits and no further commits for 2 years as of this writing.

- [nt.python_for_max](https://github.com/2bbb/nt.python_for_max) -- Basic implementation of python in max using a fork of Graham Wakefield's old c++ interface. Hasn't really been touched in 3 years.

- [net.loadbang.jython](https://github.com/cassiel/net.loadbang.jython) -- A
  jython implementation for Max which uses the MXJ java interface. It's looks
  like a solid effort using Jython 2.7 but the last commit was in 2015.

Around the time of the beginning of my first covid-19 lockdown, I stumbled upon Iain Duncan's [Scheme for Max](https://github.com/iainctduncan/scheme-for-max) project, and I was quite inspired by his efforts and approach to embed a scheme implementation into a Max external.

So it was decided, during a period with less distractions than usual, to try to make a minimal python3 external, learn the max sdk, the python c-api, and also how to write more than a few lines of c that didn't crash.

It's been an education and I have come to understand precisely a quote I remember somewhere about the c language: that it's "like a scalpel". I now understand this to mean that in skilled hands it can do wonders, otherwise you almost always end up killing the patient.

Thanks to Luigi Castelli for his help with Max/MSP questions, to Stefan Behnel for his help with Cython questions, and to Iain Duncan for providing the initial inspiration and for saving me time with some great implementation ideas.

Thanks to Greg Neagle for zeroing in on the relocatability problem and sharing his elegant solution for Python frameworks via his [relocatable-python](https://github.com/gregneagle/relocatable-python) project on Github.

[cobra]: https://github.com/shakfu/py-js/tree/main/source/projects/cobra
[jmx]: https://github.com/shakfu/py-js/tree/main/source/projects/jmx
[krait]: https://github.com/shakfu/py-js/tree/main/source/projects/krait
[mamba]: https://github.com/shakfu/py-js/tree/main/source/projects/mamba
[mpy]: https://github.com/shakfu/py-js/tree/main/source/projects/mpy
[mpyx]: https://github.com/shakfu/py-js/tree/main/source/projects/mpyx
[mxpy]: https://github.com/shakfu/py-js/tree/main/source/projects/mxpy
[pktpy]: https://github.com/shakfu/py-js/tree/main/source/projects/pktpy
[pktpy2]: https://github.com/shakfu/py-js/tree/main/source/projects/pktpy2
[py]: https://github.com/shakfu/py-js/tree/main/source/projects/py
[pyjs]: https://github.com/shakfu/py-js/tree/main/source/projects/pyjs
[pymx]: https://github.com/shakfu/py-js/tree/main/source/projects/pymx
[pyx]: https://github.com/shakfu/py-js/tree/main/source/projects/pyx
[xpyc]: https://github.com/shakfu/py-js/tree/main/source/projects/xpyc
[zedit]: https://github.com/shakfu/py-js/tree/main/source/projects/zedit
[zpy]: https://github.com/shakfu/py-js/tree/main/source/projects/zpy
[ztp]: https://github.com/shakfu/py-js/tree/main/source/projects/ztp
