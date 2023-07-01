# python3 objects for max

Simple (and extensible) [python3](https://www.python.org) externals for [MaxMSP](https://cycling74.com).

Currently builds 'natively' on macOS `x86_64`and `arm64`.

repo - <https://github.com/shakfu/py-js>

[![py-js test](./media/screenshot.png)](patchers/py_test_standalone.maxpat)

## Preface

This project started out as an attempt (during a covid-19 lockdown) to develop a basic python3 external for maxmsp. It then evolved into an umbrella project for exploring different ways of using python3 in max.

Along the way, a number of python3 or python-related externals have been developed for use in a live Max environment:

name       | sdk        | lang   | description
:--------- | :--------- | :----: | :---------------------------------------------------
[py]       | max-sdk    | c      | well-featured, many packaging options + [cython](https://cython.org) api
[pyjs]     | max-sdk    | c      | js-friendly -- written as a Max javascript-extension
[mxpy]     | max-sdk    | c      | a translation of [pdpython](https://github.com/shakfu/pdpython) into Max
[pymx] [1] | min-devkit | c++    | concise, modern, using [pybind11](https://github.com/pybind/pybind11)
[zpy]      | max-sdk    | c      | uses [zeromq](https://zeromq.org) for 2way-comms with an external python process
[cobra]    | max-sdk    | c      | python3 external providing deferred and clocked function execution
[mamba]    | max-sdk    | c      | single-header c library to nest a python3 interpreter in any external
[krait]    | max-sdk    | c++    | single-header c++ library to nest a python3 interpreter in any external
[pktpy]    | max-sdk    | c++    | uses the [pocketpy](https://github.com/blueloveTH/pocketpy) single-header c++ library
[zedit]    | max-sdk    | c      | a web-based python editor using [codemirror](https://codemirror.net) and the [mongoose](https://github.com/cesanta/mongoose) embedded webserver.
[mpy]      | max-sdk    | c      | a proof-of-concept embedding [micropython](https://github.com/micropython/micropython)

[1] pymx has been moved to its own [github project](https://github.com/shakfu/min.pymx) because it uses the [min-devkit](https://github.com/Cycling74/min-devkit) sdk.

[py]: source/projects/py
[pyjs]: source/projects/pyjs
[mxpy]: source/projects/mxpy
[pymx]: https://github.com/shakfu/min.pymx
[zpy]: source/projects/zpy
[cobra]: source/projects/cobra
[mamba]: source/projects/mamba
[krait]: source/projects/krait
[pktpy]: source/projects/pktpy
[zedit]: source/projects/zedit
[mpy]: source/projects/mpy


The common objective in these externals is to help you use and distribute your python code and libraries in your Max applications. Many can be considered experimental, with 80% of development time going to the first two externals (`py` and `pyjs`). Please see below for an overview and feature comparison.

At the time of this writing, and since the switch to the new [max-sdk-base](https://github.com/cycling74/max-sdk-base), the project **is compatible with Apple Silicon-based machines** but intentionally only produces 'native' (`x86_64` or `arm64`) externals with no plans for 'fat' or universal externals to serve both architectures. You can download codesigned, notarized `x86_64`-based and `arm64`-based python3 externals from the [releases](https://github.com/shakfu/py-js/releases) section.

This README will mostly cover the first two mature externals (`py.mxo` and `pyjs.mxo`) and their many build variations available via a custom python-based build system which was specifically developed to cater for different scenerios of packaging and deploying the externals in Max packages and standalones.

If you are interested in any of the other subprojects, please look into the respective folder in the `py-js/source/projects` section.

The [Quickstart](#quickstart) section below covers general setup for all of the externals, and will get you up and running with the `py` and `pyjs` externals. The [Building Experimental Externals using Cmake](#building-experimental-externals-using-cmake) section provides additional info to build the other remaining externals, and the [Building self-contained Python3 Externals for Packages and Standalones](#building-self-contained-python3-externals-for-packages-and-standalones) section covers more advanced building and deployment scenarios.

Please feel free to ask questions or make suggestions via the project's github issue tracker.

