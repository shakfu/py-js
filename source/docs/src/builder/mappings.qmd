# PyJS Mappings

```text
python-src 
    -> Python.framework 
        -> framework-ext
        -> framework-pkg

    -> static-python
        -> static-ext
    -> tiny-static-python
        -> tiny-static-ext
           
    -> shared-python
        -> shared-ext
        -> shared-pkg
    -> tiny-shared-python

homebrew-python
        -> homebrew-ext
        -> homebrew-pkg

python-pkg
        -> relocatable-pkg
```



## Base Python Builders

```python
class FrameworkPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-shared.local"
    product = "Python.framework"

class SharedPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-shared.local"
    product = "python-shared"

@NotYetImplemented
class TinySharedPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-shared-min6.local"
    product = "tiny-shared-python"

class StaticPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-static-min3.local"
    product = "static-python"

class TinyStaticPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-static-min6.local"
    product = "tiny-static-python"

class BeewarePythonBuilder(StaticPythonBuilder):
    setup_local = "setup.beeware"
    product = "beeware-ext"
```



## Specialized Python Builders

```python
class SharedPythonForExtBuilder(SharedPythonBuilder):
    setup_local = "setup-shared.local"
    product = "ext/shared-python"

class SharedPythonForPkgBuilder(SharedPythonBuilder):
    setup_local = "setup-shared.local"
    product = "pkg/shared-python"

class FrameworkPythonForExtBuilder(FrameworkPythonBuilder):
    setup_local = "setup-shared.local"
    product = "ext/Python.framework"

class FrameworkPythonForPkgBuilder(FrameworkPythonBuilder):
    setup_local = "setup-shared.local"
    product = "pkg/Python.framework"

```


## PyJs External Builders

```python
class HomebrewExtBuilder(PyJsBuilder):
    product = "homebrew-ext"

class HomebrewPkgBuilder(PyJsBuilder):
    product = "homebrew-pkg"

class LocalSystemBuilder(PyJsBuilder):
    product = "local-sys"

class StaticExtBuilder(PyJsBuilder):
    product = "static-ext"

class SharedExtBuilder(PyJsBuilder):
    product = "shared-ext"

class SharedPkgBuilder(PyJsBuilder):
    product = "shared-pkg"

class FrameworkExtBuilder(PyJsBuilder):
    product = "framework-ext"

class FrameworkPkgBuilder(PyJsBuilder):
    product = "framework-pkg"

class RelocatablePkgBuilder(PyJsBuilder):
    product = "relocatable-pkg"

class BeewareExtBuilder(PyJsBuilder):
    product = "beeware-ext"

```