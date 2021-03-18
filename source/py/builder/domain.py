"""domain: domain objects


"""

import pathlib

# from .builders import (FrameworkPythonBuilder, SharedPythonBuilder,
#                        StaticPythonBuilder)
# from .models import Builder, Product, Project, Recipe
from .models import Project

# -------------------------------------------------------------------------------
# PROJECTS


# class PythonProject(Project):
class PythonProject(Project):
    """Project to build Python from source with different variations."""
    root = pathlib.Path.cwd()
    patch = root / 'patch'
    targets = root / 'targets'
    build = targets / 'build'
    downloads = build / 'downloads'
    src = build / 'src'
    lib = build / 'lib'


class PyJSProject(Project):
    """max external projects"""
    py_version_major = '3.9'
    py_version_minor = '2'
    py_semver = '3.9.2'
    py_version = py_version_major
    py_ver = '39'
    py_name = f'python{py_ver}'

    root = pathlib.Path.cwd()
    product = 'python'
    support = root.parent.parent / 'support'
    externals = root.parent.parent / 'externals'
    source = root.parent / 'source'
    frameworks = support / 'Frameworks'
    scripts = root / 'scripts'
    build = root / 'targets' / 'build'
    prefix = support / product
    bin = prefix / 'bin'
    lib = prefix / 'lib' / py_version
    dylib = f'libpython_{py_version}.dylib'

    py_external = externals / 'py.mxo'
    pyjs_external = externals / 'pyjs.mxo'

    bzip2_version = '1.0.8'
    ssl_version='1.1.1g'
    mac_dep_target='10.13'

    url_python='https://www.python.org/ftp/python/${SEMVER}/Python-${SEMVER}.tgz'
    url_openssl='https://www.openssl.org/source/openssl-${SSL_VERSION}.tar.gz'
    url_getpip='https://bootstrap.pypa.io/get-pip.py'


# -------------------------------------------------------------------------------
# RECIPES

# static_python = Recipe(
#     "static_python", project=PythonProject, builders=[SharedPythonBuilder]
# )

# shared_python = Recipe(
#     "shared_python", project=PythonProject, builders=[SharedPythonBuilder]
# )

# framework_python = Recipe(
#     "framework_python", project=PythonProject, builders=[FrameworkPythonBuilder]
# )



# # product classes
# class PyExt(Product):
#     name = "py.mxo"

# class PyJSExt(Product):
#     name = "pyjs.mxo"

# class StaticPython(Product):
#     name = "static-python"

# # builder classes
# class StaticPythonBuilder(Builder):
#     product_class = StaticPython

# class PyExtBuilder(Builder):
#     product_class = PyExt

# class PyJSExtBuilder(Builder):
#     product_class = PyJSExt


# # project classes
# class StaticPythonProject(Project):
#     builder_classes = [StaticPythonBuilder]

# class SharedPythonProject(Project):
#     builder_classes = [SharedPythonBuilder]

# class FrameworkPythonProject(Project):
#     builder_classes = [FrameworkPythonBuilder]

# class PyJSExternalsProject(Project):
#     builder_classes = [PyExtBuilder, PyJSExtBuilder]

# # recipe classes
# class StaticPyJSRecipe(Recipe):
#     project_classes = [StaticPythonProject, PyJSExternalsProject]


# # recipe / workspace
# r1 = StaticPyJSRecipe.from_defaults(a=1, b=2, x=2000)
# r1.build()
