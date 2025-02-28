# Optimization + Stripping options for Xcode


## New Release Options

These are presently included in the `py-js/source/projects/py/target/optimize.xcconfig`:

```xcconfig
OPTIM_LINK_FLAGS = "-Xlinker -x"
DEPLOYMENT_POSTPROCESSING = YES
// Whether to strip debugging symbols when copying resources (like included binaries)
COPY_PHASE_STRIP = YES
// Whether to compile assertions in. 
ENABLE_NS_ASSERTIONS = NO
// The optimization level (0, 1, 2, 3, s) for the produced binary
GCC_OPTIMIZATION_LEVEL = s
// Preproccessor definitions to apply to each file compiled
GCC_PREPROCESSOR_DEFINITIONS = NDEBUG=1
// Whether to enable link-time optimizations (such as inlining across translation units)
LLVM_LTO = NO
// Whether to strip debugging symbols when copying the built product to its final installation location
STRIP_INSTALLED_PRODUCT = YES
DEAD_CODE_STRIPPING = YES

// Any option below causes build failure
// STRIP_STYLE = -x 
// STRIP_STYLE = -s
// STRIP_STYLE = -S

OTHER_LDFLAGS = $(inherited) $(OPTIM_LINK_FLAGS)
```

This file can be included in common xcode options via `#include "optimize.xcconfig"` at the end of `#include "common.xcconfig"`.


## Preliminary Results

```text
For 3.13.2 with `make static-tiny-ext` option:

    	MB        	MB        	MB      MB
		normal      optimized   delta   ratio
py  	10.9		9.4       	-1.5    0.86
pyjs    8.5      	7.3       	-1.2    0.86

```
