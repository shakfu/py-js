from ..agent_min import (Builder, Product, Project, ProjectRecipe, BuilderRecipe, 
    Bzip2Builder, OpensslBuilder, XzBuilder, 
    SharedPythonBuilder, FrameworkPythonBuilder, StaticPythonBuilder, 
    HomebrewBuilder)

def test_classes():
    klasses = [
        Bzip2Builder,
        OpensslBuilder,
        XzBuilder,
        SharedPythonBuilder,
        FrameworkPythonBuilder,
        StaticPythonBuilder,
        HomebrewBuilder,
    ]
    for klass in klasses:
        obj = klass()
        assert obj.product
        assert obj.project


def test_pyjs():
    """testing the schema"""

    # product classes
    class PyExt(Product):
        default_name = 'py'
        default_version = '0.0.1'
        url_template = None

    class PyJsExt(Product):
        default_name = 'pyjs'
        default_version = '0.0.1'
        url_template = None

    class StaticPython(Product):
        default_name = 'static-python'
        default_version = '3.9.2'
        url_template = None

    # builder classes
    class StaticPythonBuilder(Builder):
        product_class = StaticPython
        dependencies = []

    class PyExtBuilder(Builder):
        product_class = PyExt
        dependencies = []

    class PyJsExtBuilder(Builder):
        product_class = PyJsExt
        dependencies = []

    # project classes
    class PythonProject(Project):
        builder_classes = [StaticPythonBuilder]

    class MaxExternalProject(Project):
        builder_classes = [PyJsExtBuilder]

    # recipes
    class StaticPyJsRecipe(BuilderRecipe):
        "static-pyjs-recipe"

    class AltRecipe(ProjectRecipe):
        project_classes = [PythonProject, MaxExternalProject]

    # products
    recipe1 = AltRecipe(name="pyjs")
    recipe1.build()

    recipe2 = StaticPyJsRecipe(
        name="pyjs",
        builders=[
            StaticPythonBuilder(
                product=StaticPython(name="static-python", version="3.9.2"),
                project=PythonProject()),
            PyExtBuilder(
                product=PyExt(name="py.mxo", version="0.1"),
                project=MaxExternalProject()),
            PyJsExtBuilder(
                product=PyJsExt(name="pyjs.mxo", version="0.1"),
                project=MaxExternalProject()),
        ])
    recipe2.build()
