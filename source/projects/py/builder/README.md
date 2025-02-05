# builder

The python `builder` package organizes the compilation and packaging of externals built against custom python builds.

The initial flow is as follows:

```text
builder.Application
    -> factory.FactoryManager
    -> core.Builder subclass
```

The `core.Builder` variants are responsible for producing a single compiled product which are specified instances of `core.Product`.

The `core.Builder` instance uses instances of builder supporting classes such as `depend.DependencyManager` and `shell.ShellCmd` to execute this process.

Once the concrete compiled product is produced, it can be packaged, signed, notarized with the `package.PackageManager`.

## project structure

```text
builder
    cli
        MetaCommander
        Commander
    depend
        DependencyManager
    factory
        FactoryManager
    package
        PackageManager
    patch
        PythonSetupManager
    utils
    shell
        ShellCmm
    config
        Python
        Project
    core
        Settings
        Product
        Recipe
        Builder
            Bzip2Builder
            OpensslBuilder
            XzBuilder
            PythonBuilder
                RelocatablePythonBuilder
                PythonSrcBuilder
                    FrameworkPythonBuilder
                        FrameworkPythonForExtBuilder
                        FrameworkPythonForPkgBuilder
                    SharedPythonBuilder
                        SharedPythonForExtBuilder
                        SharedPythonForPkgBuilder
                    StaticPythonBuilder
                        BeewarePythonBuilder
                    TinyStaticPythonBuilder
                    TinySharedPythonBuilder
                PyJsBuilder
                    LocalSystemBuilder
                    HomebrewExtBuilder
                    HomebrewPkgBuilder            
                    StaticExtBuilder
                    StaticPkgBuilder (not implemented, useless)
                    SharedExtBuilder
                    SharedPkgBuilder (not used)
                    FrameworkExtBuilder
                    FrameworkPkgBuilder
                    BeewareExtBuilder
                    RelocatablePkgBuilder
```
