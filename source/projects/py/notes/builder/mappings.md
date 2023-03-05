## Base Python

```python
class FrameworkPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-shared.local"
    build_dir = "Python.framework"

class SharedPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-shared.local"
    build_dir = "python-shared"

class StaticPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-static-min3.local"
    build_dir = "static-python"

class TinyStaticPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-static-min6.local"
    build_dir = "tiny-static-python"

class BeewarePythonBuilder(StaticPythonBuilder):
    setup_local = "setup.beeware"

```



## Specialized Python

```python
class SharedPythonForExtBuilder(SharedPythonBuilder):
    setup_local = "setup-shared.local"

class SharedPythonForPkgBuilder(SharedPythonBuilder):
    setup_local = "setup-shared.local"

class FrameworkPythonForExtBuilder(FrameworkPythonBuilder):
    setup_local = "setup-shared.local"

class FrameworkPythonForPkgBuilder(FrameworkPythonBuilder):
    setup_local = "setup-shared.local"
```
