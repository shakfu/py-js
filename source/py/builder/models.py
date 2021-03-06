"""model: schema of builder

Baically a simplified copy of the xcode model. The only difference is that I have
renamed `Target` as `Builder` and `Workspace` as `Recipe` since it
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
from abc import ABC, abstractmethod
from types import SimpleNamespace

from utils.dotmap import DotMap
from utils.text import Text


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


class Product(ABC):
    """Produced by running a builder."""

    def __init__(self, name: str, path: pathlib.Path):
        self.name = name
        self.path = pathlib.Path(path)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def exists(self) -> bool:
        """returns True if product exists at self.path"""
        return self.path.exists()


class Builder(ABC):
    """A Builder know how to build a single product type in a project.

    A Builder is analagous to a Target in Xcode.
    """

    def __init__(self, name: str = None, depends_on: ['Builder'] = None, **settings):
        self.name = name
        self.depends_on = depends_on or []
        self.settings = Settings(**settings)
        self.project = None
        self.product = None

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def build(self):
        """build product"""
        print(f"\t\t{self} -> {self.product}")


class Project(ABC):
    """A repository for all the files, resources, and information required to
    build one or more software products.
    """

    def __init__(self, name: str = None , builders: list[Builder] = None, 
                 depends_on: ['Project'] = None, **settings):
        self.name = name
        self.depends_on = depends_on or []
        self.settings = Settings(**settings)
        self.builders = self.init_builders(builders) if builders else []

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def init_builders(self, builders):
        _builders = []
        for builder in builders:
            builder.project = self
            _builders.append(builder)
        return _builders

    @classmethod
    def from_defaults(cls, name=None, builder_classes=None, **settings):
        if not builder_classes:
            builder_classes = cls.builder_classes
        _builders = []
        for builder_class in builder_classes:
            builder_name = Text(builder_class.__name__).mixed_to_word().abbreviate()
            builder = builder_class(builder_name, **settings)
            _builders.append(builder)
        return cls(name, _builders, **settings)

    def build(self):
        """sequence builders in order of dependency"""
        print(f"\t{self}")
        for builder in self.builders:
            builder.build()


class Recipe(ABC):
    """A platform-specific container for multiple build projects.

    A recipe is analogous to a workspace in xcode
    """

    def __init__(self, name: str = None, projects: list[Project] = None, **settings):
        self.name = name or Text(self.__class__.__name__).mixed_to_word().abbreviate()
        self.settings = Settings(**settings)
        self.projects = projects

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    __repr__=__str__

    @classmethod
    def from_defaults(cls, name=None, project_classes=None, **settings):
        if not project_classes:
            project_classes = cls.project_classes
        _projects = []
        for project_class in project_classes:            
            project_name = Text(project_class.__name__).mixed_to_word().abbreviate()
            project = project_class.from_defaults(project_name, **settings)
            _projects.append(project)
        return cls(name, _projects, **settings)

    @classmethod
    def from_tree_yaml(cls, path):
        import yaml
        with open(path) as f:
            yml = f.read()
            cfg = yaml.safe_load(yml)

        # extract recipe
        recipe_cfg = cfg['recipe']
        recipe_name = recipe_cfg['name']
        if 'type' in recipe_cfg:
            classname = recipe_cfg['type']
        else:
            classname = Text(recipe_cfg).classname + 'Recipe'
        recipe_class = type(classname, (Recipe,), {})
        recipe = recipe_class(recipe_name, **recipe_cfg['settings'])

        _projects, _builders = {}, {}

        # extract recipe.projects
        recipe.projects = []
        for project_cfg in recipe_cfg['projects']:
            project_name = project_cfg['name']
            if 'type' in project_cfg:
                classname = project_cfg['type']
            else:
                classname = Text(project_name).classname + 'Project'
            project_class = type(classname, (Project,), {})
            project = project_class(project_name, depends_on=project_cfg['depends_on'], 
                                    **project_cfg['settings'])
            _projects[project_name] = project

            # extract recipe.project.builders
            project.builders = []
            for builder_cfg in project_cfg['builders']:
                builder_name = builder_cfg['name']
                if 'type' in builder_cfg:
                    classname = builder_cfg['type']
                else:
                    classname = Text(builder_name).classname + 'Builder'
                builder_class = type(classname, (Builder,), {})
                builder = builder_class(builder_name, depends_on=builder_cfg['depends_on'],
                                        **builder_cfg['settings'])
                _builders[builder_name] = builder
                project.builders.append(builder)   
            recipe.projects.append(project)

        # re-trace for dependencies
        for project in recipe.projects:
            project_depends_on = [_projects[name] for name in project.depends_on]
            project.depends_on = project_depends_on
            for builder in project.builders:
                builder_depends_on = [_builders[name] for name in builder.depends_on]
                builder.depends_on = builder_depends_on

        return recipe

    @classmethod
    def from_flat_yaml(cls, path):
        import yaml
        with open(path) as f:
            yml = f.read()
            cfg = yaml.safe_load(yml)

        # create builders
        _builders = {}
        for builder_cfg in cfg['builders']:
            builder_name = builder_cfg['name']
            if 'type' in builder_cfg:
                classname = builder_cfg['type']
            else:
                classname = Text(builder_name).classname + 'Builder'
            builder_class = type(classname, (Builder,), {})
            builder = builder_class(builder_name, depends_on=None, **builder_cfg['settings'])
            _builders[builder_name] = builder

        # set builder dependencies
        for builder_cfg in cfg['builders']:
            depends_on = [_builders[name] for name in builder_cfg['depends_on']]
            _builders[builder_cfg['name']].depends_on = depends_on

        # create projects
        _projects = {}
        for project_cfg in cfg['projects']:
            project_name = project_cfg['name']
            if 'type' in project_cfg:
                classname = project_cfg['type']
            else:
                classname = Text(project_name).classname + 'Project'
            project_class = type(classname, (Project,), {})
            project_builders = [_builders[key] for key in project_cfg['builders']]
            project = project_class(project_name, builders=project_builders, 
                                    depends_on=None, **project_cfg['settings'])
            _projects[project_cfg['name']] = project

        # set project dependencies
        for project_cfg in cfg['projects']:
            depends_on = [_projects[name] for name in project_cfg['depends_on']]
            _projects[project_cfg['name']].depends_on = depends_on

        recipe = cfg['recipe']
        recipe_projects = [_projects[key] for key in recipe['projects']]
        return cls(recipe['name'], recipe_projects, **recipe['settings'])

    @classmethod
    def gen_from_flat_yaml(cls, path):
        from textwrap import dedent
        import yaml
        from mako.template import Template

        with open(path) as f:
            yml = f.read()
            cfg = yaml.safe_load(yml)

        template = dedent("""

        <%
            # utility funcs
            classname = lambda x: Text(x).classname
            prefix = lambda p: lambda x: classname(x)+p
            Pd = prefix('Product')
            P = prefix('Project')
            B = prefix('Builder')
            R = prefix('Recipe')

            # filters
            def unquote(s):
                return s.replace("'", "")
        %>

        # product classes
        # -------------------------------------------------
        % for product in products:
        class ${Pd(product.name)}(Product):
            path = "${product.path}"

        % endfor

        # builder classes
        # -------------------------------------------------
        % for builder in builders:
        class ${B(builder.name)}(Builder):
            product_class = ${Pd(builder.product)}

        % endfor

        # project classes
        # -------------------------------------------------
        % for project in projects:
        class ${P(project.name)}(Project):
            builder_classes = ${[B(b) for b in project.builders] | unquote}

        % endfor

        # recipe classes
        # -------------------------------------------------
        class ${R(recipe.name)}(Recipe):
            project_classes = ${[P(p) for p in recipe.projects] | unquote} 

        """)

        cfg.update({'Text': Text})
        cfg = DotMap(cfg)
        print(Template(template).render(**cfg))


    def build(self):
        """build projects in order of dependency"""
        print(f"{self}")
        for project in self.projects:
            project.build()


def test_from_defaults():
    """testing the schema"""

    # product classes
    class PyExt(Product):
        name = "py.mxo"

    class PyJSExt(Product):
        name = "pyjs.mxo"

    class StaticPython(Product):
        name = "static-python"


    # builder classes
    class StaticPythonBuilder(Builder):
        product_class = StaticPython

    class PyExtBuilder(Builder):
        product_class = PyExt

    class PyJSExtBuilder(Builder):
        product_class = PyJSExt


    # project classes
    class StaticPythonProject(Project):
        builder_classes = [StaticPythonBuilder]

    class PyJSExternalsProject(Project):
        builder_classes = [PyExtBuilder, PyJSExtBuilder]

    # recipe classes
    class StaticPyJSRecipe(Recipe):
        project_classes = [StaticPythonProject, PyJSExternalsProject]


    # recipe / workspace
    r1 = StaticPyJSRecipe.from_defaults(a=1, b=2, x=2000)
    r1.build()


def test_from_flat_yaml():
    print()
    print("flat"*20)
    print()
    class MacOSRecipe(Recipe):
        """A macos-specific build recipe"""

    r1 = MacOSRecipe.from_flat_yaml('yaml/flat.yml')
    print(r1)
    for p in r1.projects:
        print('\t', p)
        for b in p.builders:
            print('\t\t', b)

def test_from_tree_yaml():
    print()
    print("tree"*20)
    print()
    class MacOSRecipe(Recipe):
        """A macos-specific build recipe"""

    r1 = MacOSRecipe.from_tree_yaml('yaml/tree.yml')
    print(r1)
    for p in r1.projects:
        print('\t', p)
        for b in p.builders:
            print('\t\t', b)

def test_gen_from_flat_yaml():
    class MacOSRecipe(Recipe):
        """A macos-specific build recipe"""

    r1 = MacOSRecipe.gen_from_flat_yaml('yaml/flat.yml')



if __name__ == "__main__":
    test_from_defaults()
    test_from_flat_yaml()
    test_from_tree_yaml()
    test_gen_from_flat_yaml()
