"""model: schema of builder

Baically a simplified copy of the xcode model. The only difference is that I have
renamed 'Target' as 'Builder' and 'Workspace' as 'Recipe' since it
makes more sense in this context.

Recipe -- Settings
    |           ∆
    |*          |
    Project -- Settings
        |           ∆
        |*          |
        Builder -- Settings
            |
            v
            Product

"""
import pathlib
from types import SimpleNamespace


class Settings(SimpleNamespace):
    """A dictionary object with dotted access to its members."""

    def copy(self):
        """provide a copy of the internal dictionary"""
        return Settings(**self.__dict__.copy())

    def update(self, other):
        """Like a dict.update but using the internal dict instead"""
        if isinstance(other, dict):
            self.__dict__.update(other)
        elif isinstance(other, Settings):
            self.__dict__.update(other.__dict__)
        else:
            raise TypeError


class Product:
    """Produced by running a builder."""

    def __init__(self, name: str, path: pathlib.Path):
        self.name = name
        self.path = pathlib.Path(path)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def exists(self) -> bool:
        """returns True if product exists at self.path"""
        return self.path.exists()


class Builder:
    """A Builder know how to build a single product type in a project.

    A Builder is analagous to a Target in Xcode.
    """

    def __init__(self, name: str, product: Product, 
                 depends_on: list["Builder"] = None, **settings):
        self.name = name
        self.product = product
        self.depends_on = depends_on or []
        self.settings = Settings(**settings)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}-{self.settings}'>"

    def setup(self, parent):
        """inherit settings of parent object"""
        parent_settings = parent.settings.copy()
        parent_settings.update(self.settings)
        self.settings = parent_settings

    def build(self):
        """build product"""
        print(f"\t\t{self} -> {self.product} [{self.product.exists()}]")


class Project:
    """A repository for all the files, resources, and information required to
    build one or more software products.
    """

    def __init__(self, name: str, builders: list[Builder], 
                 depends_on: list["Project"] = None, **settings):
        self.name = name
        self.depends_on = depends_on or []
        self.settings = Settings(**settings)
        self.builders = builders

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}-{self.settings}'>"

    def setup(self, parent):
        """inherit settings of parent object"""
        parent_settings = parent.settings.copy()
        parent_settings.update(self.settings)
        self.settings = parent_settings

    def build(self):
        """sequence builders in order of dependency"""
        print(f"\t{self}")
        for builder in self.builders:
            builder.setup(self)
            builder.build()


class Recipe:
    """A platform-specific container for multiple build projects.

    A recipe is analogous to a workspace in xcode
    """

    def __init__(self, name: str, projects: list[Project], **settings):
        self.name = name
        self.settings = Settings(**settings)
        self.projects = projects

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}-{self.settings}'>"

    def build(self):
        """build projects in order of dependency"""
        print(f"{self}")
        for project in self.projects:
            project.setup(self)
            project.build()


def test():
    """testing the schema"""
    # products
    py_mxo = Product("py_mxo", path="py.mxo")
    pyjs_mxo = Product("pyjs_mxo", path="pyjs.mxo")
    python_static = Product("python_static", path="libpython3.9.a")

    # builders / target
    python_static_builder = Builder("python_static_builder", product=python_static)
    py_builder = Builder("py_builder", product=py_mxo, depends_on=python_static_builder)
    pyjs_builder = Builder(
        "pyjs_builder", product=pyjs_mxo, depends_on=python_static_builder, a=20, c=10
    )

    # projects
    build_python = Project("build_python", builders=[python_static_builder], a=4)
    build_py_js = Project(
        "build_py_js",
        builders=[py_builder, pyjs_builder],
        depends_on=[build_python],
        a=3,
    )

    # recipe / workspace
    py_js = Recipe("py-js", projects=[build_python, build_py_js], a=1, b=2)

    py_js.build()


def test2():
    """testing the schema"""

    p1 = Product("py_mxo", path="py.mxo")
    p2 = Product("pyjs_mxo", path="pyjs.mxo")
    p3 = Product("python_static", path="libpython3.9.a")

    # builders / target
    b1 = Builder("python_static_builder", product=p3)
    b2 = Builder("py_builder", product=p1, depends_on=b1)
    b3 = Builder("pyjs_builder", product=p2, depends_on=b1, a=20, c=10)

    # projects
    pr1 = Project("build_python", builders=[b1], a=4)
    pr2 = Project("build_py_js", builders=[b2, b3], depends_on=[b1], a=3)

    # recipe / workspace
    r1 = Recipe("py-js", projects=[pr1, pr2], a=1, b=2, x=2000)
    r2 = Recipe("python", projects=[pr1, pr2], a=1, b=2, x=2000)

    r1.build()



if __name__ == "__main__":
    test2()
