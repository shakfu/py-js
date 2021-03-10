"""projects: project classes


"""



import pathlib


class PythonProject:
    """Project to build Python from source with different variations."""
    root = pathlib.Path.cwd()
    patch = root / 'patch'
    targets = root / 'targets'
    build = targets / 'build'
    downloads = build / 'downloads'
    src = build / 'src'
    lib = build / 'lib'




class PyJS:
    """max external projects"""
