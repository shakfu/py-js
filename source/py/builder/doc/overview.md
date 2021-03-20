# CURRENT

Builder(ABC)
    - OSXBuilder
        - Bzip2Builder
        - OpensslBuilder
        - XzBuilder
        - PythonBuilder
            - StaticPythonBuilder
            - SharedPythonBuilder
            - FrameworkPythonBuilder

Application
    - do_shared -> SharedPythonBuilder
    - do_static -> StaticPythonBuilder
    - do_framework -> FrameworkPythonBuilder
    - do_all

# TO BE

Mixins
    Shell
    Commander

The Project provide context 
    (place, location of source files, prefix or destination of product)

The Builder knows which product it can build (one-to-one correspondance)

Two Examples relevant here
1. A PythonRecipe contains a
    - A PythonProject provides three folders (downloads, src, lib folders)
        - downloads: stores compressed versions of dependencies
        - src: uncompressed src folders of dependencies
        - lib: product folders
    - A PythonProject launches a PythonBuilder (Framework, Shared, Static)
        - download the dependencies
        - build them to produce products (... ProductTypes ...)
    - Products are checked to exist and may be tested additionally

2. A PyJs Recipe contains a
    - A PyJsProject provides the location and structure of a max packages
        - docs
        - examples
        - externals
        - help
        - init
        - javascript
        - jsextensions
        - media
        - patchers
        - source
        - support
    - PyJsProject launches a PyJsBuilder {...}
        - action 1
        - action 2
        - ... 
    - Products are checked to exist and may be tested additionally


- Recipe
    - PyJsRecipe -> (PythonProject, PyJsProject)

- Product
    - ProductType -> BuilderType

- Project 
    - PythonProject (builds python from source)
    - PyJsProject
        - HomebrewProject (provides context)
            - bin-homebrew-sys  -> HomebrewSysBuilder
            - bin-homebrew-pkg  -> HomebrewPkgBuilder
            - bin-homebrew-ext  -> HomebrewExtBuilder

        - SrcProject (provides context)
            - src-framework-pkg -> SrcFrameworkPkgBuilder
            - src-framework-ext -> SrcFrameworkExtBuilder
            - src-shared-pkg    -> SrcSharedPkgBuilder
            - src-shared-ext    -> SrcSharedExtBuilder
            - src-static-pkg    -> SrcStaticPkgBuilder
            - src-static-ext    -> SrcStaticExtBuilder

- Builder(ABC, Shell)
    - PythonBuilder
        - StaticPythonBuilder
        - SharedPythonBuilder
        - FrameworkPythonBuilder

    - PyJsHomebrewBuilder
        - HomebrewSysBuilder
        - HomebrewPkgBuilder
        - HomebrewExtBuilder

    - PyJsSrcBuilder
        - SrcFrameworkPkgBuilder
        - SrcFrameworkExtBuilder
        - SrcSharedPkgBuilder
        - SrcSharedExtBuilder
        - SrcStaticPkgBuilder
        - SrcStaticExtBuilder

Application(Commander)
    - do_python --static --shared --framework --all
    
    - do_local
        builds locally-linked homebrew version

    - do_package
        builds homebrew package version

    - do_external
        builds homebrew external version
