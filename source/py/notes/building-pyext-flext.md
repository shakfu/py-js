# Notes on how to Compile Grrrr's Py/Pyext and Flext

## Observations

- I managed to compile to both flext and py (albeit for system python 2.7 only)

- Very complicated multi-step compilation process which included a need to change a header reference in the maxsdk

- Documentation is pretty poor but the project and its ambitions are impressive. It's a shame that it's so complex and that's it still not usable out-of-the-box for python3 (although there seems to be an effort to make that happen).

- Here are some notes in case I forget

- Have to build build and install flext first

- After compiling everything successfully and puting the files in a package format as follows, the py.maxhelp opened without errors.

```bash
py/
├── externals
│   └── py.mxo
│       └── Contents
│           ├── MacOS
│           │   └── py
│           └── Pkginfo
├── help
│   ├── py.maxhelp
│   └── scripts
│       ├── buffer.py
│       ├── pak.py
│       ├── script.py
│       ├── sendrecv.py
│       ├── sig.py
│       ├── simple.py
│       ├── tcltk.py
│       └── threads.py
└── init
    └── py-objectmappings.txt
```

Evidently, the location of the scripts folder should be in the help folder with the `py.maxhelp`.

## Building

- The pattern is that you compile flext first

```bash
bash ../flext/build.sh max gcc build
sudo bash ../flext/build.sh max gcc install
```

- Then PD or MAX from flext:

```bash
bash ../flext/build.sh pd gcc build
#and
bash ../flext/build.sh max gcc build
```

This the above assumes that compilation is fully configure as per the following examples:

### flext/buildsys/config-mac-pd-gcc.txt

```Makefile
# where is the PD installation including source code?
# (this should point to the main folder, which has a "src" (PD Vanilla) or "include" (PD extended) subfolder)
PDPATH=/Applications/Studio/Pd-0.51-4.app/Contents/Resources

# where is the PD executable?
PDBIN=$(PDPATH)/bin/pd

###############################################################

# prefix for flext installation
# headers are in $(FLEXTPREFIX)/include/flext
# libraries are in $(FLEXTPREFIX)/lib
# build system is in $(FLEXTPREFIX)/lib/flext

FLEXTPREFIX=/usr/local

###############################################################

# where should the external be built?
OUTPATH=pd-darwin

# where should the external be installed?
INSTPATH=$(PDPATH)/extra

###############################################################

# STK (synthesis tool kit) support
# http://ccrma.stanford.edu/software/stk

# where to find the STK header files (e.g. stk.h)
#STK_INC=/usr/local/include/stk

# where to find the STK library (normally libstk.a)
# (comment out STK_LIB if you don't use STK)
#STK_LIB=/usr/local/lib/libstk.a

###############################################################

# SndObj support
# http://music.nuim.ie//musictec/SndObj

# where to find the SndObj header files (e.g. sndobj.h)
#SNDOBJ_INC=/usr/local/include/sndobj

# where to find the SndObj library (normally libsndobj.a)
# (comment out SNDOBJ_LIB if you don't use SndObj)
#SNDOBJ_LIB=/usr/local/lib/libsndobj.a

###############################################################

# make flags (e.g. use multiprocessor)
MFLAGS=-j 2

# user defined compiler flags
UFLAGS += -ffast-math -mmacosx-version-min=10.9 # -march=native

# user defined linker flags
LDFLAGS += -mmacosx-version-min=10.9

# user defined optimization flags
OFLAGS += -O3 -mtune=native

# user defined debugging flags
DFLAGS +=

# architecture-specific flags (optional)
UFLAGS_ppc += -faltivec
OFLAGS_ppc +=
DFLAGS_ppc +=

UFLAGS_i386 +=
OFLAGS_i386 +=
DFLAGS_i386 +=

UFLAGS_x86_64 +=
OFLAGS_x86_64 += 
DFLAGS_x86_64 +=

# cross-compilation (optional)
ARCH=x86_64

# SDK for 10.6
#OSXSDK=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX11.1.sdk
```

### flext/buildsys/config-mac-max-gcc.txt

```Makefile
# where are the Max/MSP SDK header files?

MAXSDKPATH=$(HOME)/Downloads/src/py-js/source/maxsdk/source/c74support

###############################################################

# prefix for flext installation
# headers are in $(FLEXTPREFIX)/include/flext
# libraries are in $(FLEXTPREFIX)/lib
# build system is in $(FLEXTPREFIX)/lib/flext

FLEXTPREFIX=/usr/local

###############################################################

# where should the external be built?
OUTPATH=max-darwin

# where should the external be installed?
INSTPATH=$(HOME)/Downloads/src/grr/package/externals

# where should the initialization files be installed?
INITPATH=$(HOME)/Downloads/src/grr/package/init

# where should the help files be installed?
HELPPATH=$(HOME)/Downloads/src/grr/package/help

###############################################################

# STK (synthesis tool kit) support
# http://ccrma.stanford.edu/software/stk

# where to find the STK header files (e.g. stk.h)
#STK_INC=/usr/local/include/stk

# where to find the STK library (normally libstk.a)
# (comment out STK_LIB if you don't use STK)
#STK_LIB=/usr/local/lib/libstk.a

###############################################################

# SndObj support
# http://music.nuim.ie//musictec/SndObj

# where to find the SndObj header files (e.g. sndobj.h)
#SNDOBJ_INC=/usr/local/include/sndobj

# where to find the SndObj library (normally libsndobj.a)
# (comment out STK_LIB if you don't use SndObj)
#SNDOBJ_LIB=/usr/local/lib/libsndobj.a

###############################################################

# make flags (e.g. use multiprocessor)
MFLAGS=-j 2

# user defined compiler flags
UFLAGS += -ffast-math -mmacosx-version-min=10.9 # -march=native

# user defined linker flags
LDFLAGS += -mmacosx-version-min=10.9

# user defined optimization flags
# for now, don't use -O3 !(Max will hang)
OFLAGS += -Os -mtune=native # -ftree-vectorize

# user defined debugging flags
DFLAGS +=

# architecture-specific flags (optional)
UFLAGS_ppc += -faltivec 
OFLAGS_ppc += 
DFLAGS_ppc +=

UFLAGS_i386 +=
OFLAGS_i386 += 
DFLAGS_i386 +=

UFLAGS_x86_64 +=
OFLAGS_x86_64 += 
DFLAGS_x86_64 +=

# list of architectures to build
ARCH=x86_64

# SDK for 10.4
OSXSDK=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX11.1.sdk
```

### Modify maxsdk header

As a final point, I had to modify a header reference in the maxsdk to get it to compile after getting a <Multiprocessing.h> not found error.

With a bit of research discovered the fix and that <Multiprocessing.h> was deprecated in this stackoverflow [post](https://stackoverflow.com/questions/11912815/compiling-pulseaudio-on-mac-os-x-with-coreservices-h)

The change in ../maxsdk/source/c74support/max-includes/ext_critical.h

was to comment out as follows

```c

#ifdef MAC_VERSION
//#include <Multiprocessing.h>
#include <CoreServices/CoreServices.h>
typedef MPCriticalRegionID t_critical;  ///< a critical region  @ingroup critical

#endif // MAC_VERSION
```
