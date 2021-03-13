"""model: schema of builder

Baically a simplified copy of the xcode model. The only difference is that I have
renamed 'Target' as 'Builder' and 'Workspace' as 'Recipe' since it
makes more sense in this context.

Recipe -- Settings
|    |           ∆
|    |          |
|    Project -- Settings
|        |           ∆
| *      |*          |
| ------ Builder -- Settings
            |
            v
            Product

"""
import inspect
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


class Project:
    """A repository for all the files, resources, and information required to
    build one or more software products.
    """
    def __init__(self, name: str, **settings):
        self.name = name
        self.settings = Settings(**settings)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"




class Builder:
    """A Builder know how to build a single product type in a project.

    A Builder is analagous to a Target in Xcode.
    """
    depends_on: list["Builder"] = []

    def __init__(self, project: Project, 
                 depends_on: list["Builder"] = None, **settings):
        self.project = project
        # self.depends_on = depends_on or []
        self.depends_on = ([B(project) for B in depends_on] if depends_on else
                           [B(project) for B in self.depends_on])
        self.settings = Settings(**settings)
        self.product = None

    def __str__(self):
        return f"<{self.__class__.__name__}:'{id(self)}'>"

    def __iter__(self):
        for dependency in self.depends_on:
            yield dependency
            for subdependency in iter(dependency):
                yield subdependency

    def build(self):
        """build product"""
        print(f'{self}', f'{self.project}') 



class Recipe:
    """A platform-specific container for multiple build projects.

    A recipe is analogous to a workspace in xcode
    """

    def __init__(self, name: str, project: Project, builders: [Builder] = None, 
                 depends_on: list['Recipe'] = None, **settings):
        self.name = name
        self.project = self.setup_project(project)
        self.depends_on = depends_on or []
        self.builders = self.setup_builders(builders, settings)
        self.settings = Settings(**settings)

    def setup_project(self, project):
        if inspect.isclass(project):
            return project(f'{self.name}_project')
        else:
            return project

    def setup_builders(self, builders, settings):
        if builders:
            if inspect.isclass(builders[0]):
                return [klass(self.project, **settings) for klass in builders]
            else:
                return builders

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}-{self.settings}'>"

    def build(self):
        """build project in order of dependency"""
        print(f'{self.name} building..')
        for builder in self.builders:
            builder.build()



def test():
    """testing the schema"""

    class XzBuilder(Builder): pass
    class OpensslBuilder(Builder): pass
    class Bzip2Builder(Builder): pass
    class PythonStaticBuilder(Builder): pass

    class PyExtBuilder(Builder): pass
    class PyJSExtBuilder(Builder): pass

    class BuildPython(Project): pass
    class BuildPyJS(Project): pass


    p1 = Product("py_mxo", path="py.mxo")
    p2 = Product("pyjs_mxo", path="pyjs.mxo")
    p3 = Product("python_static", path="libpython3.9.a")

    # projects
    pr1 = BuildPython("build_python", a=4)
    pr2 = BuildPyJS("build_py_js", a=3)

    # recipe / workspace
    r1 = Recipe("python", project=pr1, builders=[PythonStaticBuilder])
    r2 = Recipe("py-js", project=pr2, builders=[PyExtBuilder,PyJSExtBuilder], depends_on=[r1])
    r1.build()
    r2.build()


def test1():
    """testing the schema"""


    class Builder1(Builder): pass
    class Builder2(Builder): pass

    class Project1(Project): pass
    class Project2(Project): pass

    p1 = Product("py_mxo", path="py.mxo")
    p2 = Product("pyjs_mxo", path="pyjs.mxo")
    p3 = Product("python_static", path="libpython3.9.a")

    # projects
    pr1 = Project("build_python", a=4)
    pr2 = Project("build_py_js", a=3)

    # recipe / workspace
    r1 = Recipe("py-js", project=pr1, builders=[Builder1,Builder2], x=2000)
    r2 = Recipe("python", project=pr2, builders=[Builder1], depends_on=[r1])
    r3 = Recipe("recipe3", project=Project, builders=[Builder2])
    r1.build()
    r2.build()
    r3.build()


if __name__ == "__main__":
    test()
