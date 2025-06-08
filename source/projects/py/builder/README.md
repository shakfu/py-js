# builder

The python `builder` package organizes the compilation and packaging of externals built against custom python builds.

The initial flow is as follows:

```sh
builder.__main__.Application
    -> builder.factory.FactoryManager
    -> builder.core.Builder subclass
```

The `builder.core.Builder` variants are responsible for producing a single compiled product which are specified instances of `builder.core.Product`.

The `builder.core.Builder` instance uses instances of supporting classes such as `builder.depend.DependencyManager` and `builder.shell.ShellCmd` to execute this process.

Once the concrete compiled product is produced, it can be packaged, signed, notarized with the `builder.package.PackageManager`.

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
        CodesignExternal
    patch
        PythonSetupManager
    utils
    shell
        ShellCmd
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

## builder.package - Package Manager

Does the equivalent of the following workflow

1. Create Keychain Profile

```sh
xcrun notarytool \
    store-credentials "NOTARYTOOL_PROFILE" \
    --apple-id "firstname.lastname@example.com" \
    --team-id ABCDEF123X \
    --password "xxxx-xxxx-xxxx-xxxx"

# where `-—password` is an app-specific password

VARIANT="shared-ext"

export DEV_ID="<first_name> <last_name>"

make ${VARIANT}

make sign

make dmg PKG_NAME="${VARIANT}"

export PRODUCT_DMG="<absolute-path-to-dmg>"

make sign-dmg ${PRODUCT_DMG}

xcrun notarytool submit ${PRODUCT_DMG} --keychain-profile "<keychain_profile>"

xcrun stapler staple ${PRODUCT_DMG}

mv ${PRODUCT_DMG} ~/Downloads/pyjs-builds
```

## CodesignExternal

Is a utility class which recursively walks through a bundle or folder structure
and signs all of the internal binaries which fit the given pattern

Note: you can reduce the logging verbosity by making DEBUG=False

Steps to sign a Max package with a externals in the 'externals' folder
depending on a framework or two in the 'support' folder:

1. codesign externals `[<name>.mxo, ...]` in `externals` folder

    `builder.sign_folder('externals')`

2. codesign frameworks or libraries `[<name>.framework | python<ver> | ...]`

    `builder.sign_folder('support')`

3. create package as folder then convert to `.dmg`

    - create $package folder
    - copy or use ditto to put everything into $package
    - convert folder into .dmg

    `builder.package_as_dmg()`

    - defaults to project name

4. notarize $package.dmg

    `builder.notarize_dmg()`

5. staple $package.dmg

    `builder.staple_dmg()`
