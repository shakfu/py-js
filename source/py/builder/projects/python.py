"""python: python project


"""

import pathlib

from ..models import Project


class PythonProject(Project):
    """Project to build Python from source with different variations."""
    root = pathlib.Path.cwd()
    patch = root / 'patch'
    targets = root / 'targets'
    build = targets / 'build'
    downloads = build / 'downloads'
    src = build / 'src'
    lib = build / 'lib'
