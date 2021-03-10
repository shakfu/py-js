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
    """A dictionary object with dotted access to its members.

    >>> settings = Settings(**dict)
    """

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

    def __init__(self, name: str, product: Product, **settings):
        self.name = name
        self.product = product
        self.settings = settings
        # self.settings = Settings(**settings)

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

    def __init__(self, name: str, builders: list[Builder], **settings):
        self.name = name
        self.settings = settings
        self.builders = self.setup_builders(builders, settings)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}-{self.settings}'>"

    def setup_builders(self, builders, settings):
        """setup builder objects"""
        _builders = []
        for builder in builders:
            project_settings = settings.copy()
            # update copy of project settings with builder.settings
            project_settings.update(builder.settings)
            builder.settings = project_settings
            _builders.append(builder)
        return _builders

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
        self.settings = settings
        self.projects = self.setup_projects(projects, settings)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}-{self.settings}'>"

    def setup_projects(self, projects, settings):
        """setup project objects"""
        _projects = []
        for project in projects:
            recipe_settings = settings.copy()
            # update copy of recipe settings with project.settings
            recipe_settings.update(project.settings)
            project.settings = recipe_settings
            _projects.append(project)
        return _projects

    def build(self):
        """build projects in order of dependency"""
        print(f"{self}")
        for project in self.projects:
            # project.setup(self)
            project.build()




def test():
    """testing the schema"""

    p1 = Product("py_mxo", path="py.mxo")
    p2 = Product("pyjs_mxo", path="pyjs.mxo")
    p3 = Product("python_static", path="libpython3.9.a")

    # builders / target
    b1 = Builder("python_static_builder", product=p3)
    b2 = Builder("py_builder", product=p1)
    b3 = Builder("pyjs_builder", product=p2, a=20, c=10)

    # projects
    pr1 = Project("build_python", builders=[b1], a=4)
    pr2 = Project("build_py_js", builders=[b2, b3], a=3)

    # recipe / workspace
    r1 = Recipe("py-js", projects=[pr1, pr2], a=1, b=2, x=2000)

    r1.build()



if __name__ == "__main__":
    test()
