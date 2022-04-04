# Github Workflows 


## How to Delete

- There is no obvious way to 'delete' a workflow, just to disable it.

- To make a workflow 'disappear':

	1. make sure it has > 0 residual runs
	2. disable it
	3. run the script `rm_disabled_workflow_runs.sh` in `scripts`
	4. remove the `<workflow>.yml` file from `.github/workflows`



## Useful Articles

- [How to automatically sign macOS apps using GitHub Actions](https://localazy.com/blog/how-to-automatically-sign-macos-apps-using-github-actions)

## Github Action Toolkit

- [The GitHub ToolKit for developing GitHub Actions](https://github.com/actions/toolkit)


## Github Actions

### Xcode

- [mxcl-xcodebuild](https://github.com/marketplace/actions/mxcl-xcodebuild)

- [xcode-build](https://github.com/marketplace/actions/xcode-build)


### Codesigning & Notarization



- [Import Code-Signing Certificates](https://github.com/marketplace/actions/import-code-signing-certificates): GitHub Action to import Apple Code-signing Certificates and Keys

- [xcode-notarize](https://github.com/marketplace/actions/xcode-notarize)

- [xcode-staple](https://github.com/marketplace/actions/xcode-staple)

- [xcode-archive](https://github.com/marketplace/actions/xcode-archive)

- [complete example](https://github.com/devbotsxyz/example-macos-rings)

### Caching

- [Cache](https://github.com/marketplace/actions/cache): This action allows caching dependencies and build outputs to improve workflow execution time.


### Python

[Setup Python](https://github.com/marketplace/actions/setup-python):

This action sets up a Python environment for use in actions by:

- optionally installing and adding to PATH a version of Python that is already installed in the tools cache.

- downloading, installing and adding to PATH an available version of Python from GitHub Releases (actions/python-versions) if a specific version is not available in the tools cache.

- failing if a specific version of Python is not preinstalled or available for download.

- optionally caching dependencies for pip, pipenv and poetry.
registering problem matchers for error output.



### xcodegen

- [xcodegen GitHub Action](https://github.com/marketplace/actions/xcodegen):

- [Setup xcodegen GitHub Action](https://github.com/marketplace/actions/setup-xcodegen)

