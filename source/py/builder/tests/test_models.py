from ..models import Builder, Product, Project, Recipe


def test_from_defaults():
    """testing the schema"""

    # product classes
    class PyExt(Product):
        name = "py.mxo"

    class PyJsExt(Product):
        name = "pyjs.mxo"

    class StaticPython(Product):
        name = "static-python"

    # builder classes
    class StaticPythonBuilder(Builder):
        product_class = StaticPython

    class PyExtBuilder(Builder):
        product_class = PyExt

    class PyJsExtBuilder(Builder):
        product_class = PyJsExt


    # project classes
    class StaticPythonProject(Project):
        builder_classes = [StaticPythonBuilder]

    class PyJsExternalsProject(Project):
        builder_classes = [PyExtBuilder, PyJsExtBuilder]

    # recipe classes
    class StaticPyJsRecipe(Recipe):
        project_classes = [StaticPythonProject, PyJsExternalsProject]


    # recipe / workspace
    r1 = StaticPyJsRecipe.from_defaults(a=1, b=2, x=2000)
    r1.build()
