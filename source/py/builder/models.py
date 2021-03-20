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
from importlib import import_module
from types import SimpleNamespace

from .utils.dotmap import DotMap
from .utils.text import Text

# utility funcs
import_from_module = lambda module, obj: getattr(import_module(module), obj)


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

    # @abstractmethod
    def clean(self):
        """shallow cleanse build"""
        for builder in self.depends_on:
            builder.clean()
        print(f"\t\t{self} -> {self.product}")

    # @abstractmethod
    def reset(self):
        """reset (deep cleanse) build"""
        for builder in self.depends_on:
            builder.reset()
        print(f"\t\t{self} -> {self.product}")

    # @abstractmethod
    def build(self):
        """build product"""
        for builder in self.depends_on:
            builder.build()
        print(f"\t\t{self} -> {self.product}")

    # @abstractmethod
    def install(self):
        """install product"""
        for builder in self.depends_on:
            builder.install()
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
        """Associate builders with parent project during initialization"""
        _builders = []
        for builder in builders:
            builder.project = self
            _builders.append(builder)
        return _builders

    @classmethod
    def from_defaults(cls, name=None, builder_classes=None, **settings):
        """Create project instance from default class attributes."""
        if not builder_classes:
            builder_classes = cls.builder_classes
        _builders = []
        for builder_class in builder_classes:
            builder_name = Text(builder_class.__name__).mixed_to_word().abbreviate()
            builder = builder_class(builder_name, **settings)
            _builders.append(builder)
        return cls(name, _builders, **settings)

    # @abstractmethod
    def clean(self):
        """Sequence  builder cleaning in FIFO order"""
        print(f"\t{self}")
        for project in self.depends_on:
            project.clean()

        for builder in self.builders:
            builder.clean()

    # @abstractmethod
    def reset(self):
        """Sequence builder resetting in FIFO order"""
        print(f"\t{self}")
        for project in self.depends_on:
            project.reset()

        for builder in self.builders:
            builder.reset()

    # @abstractmethod
    def build(self):
        """Sequence builder building in FIFO order"""
        print(f"\t{self}")
        for project in self.depends_on:
            project.build()

        for builder in self.builders:
            builder.build()

    # @abstractmethod
    def install(self):
        """Sequence builder installing in FIFO order"""
        print(f"\t{self}")
        for project in self.depends_on:
            project.install()

        for builder in self.builders:
            builder.install()


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
        """Create from default class attributes."""
        if not project_classes:
            project_classes = cls.project_classes
        _projects = []
        for project_class in project_classes:            
            project_name = Text(project_class.__name__).mixed_to_word().abbreviate()
            project = project_class.from_defaults(project_name, **settings)
            _projects.append(project)
        return cls(name, _projects, **settings)

    def build(self):
        """build projects"""
        for project in self.projects:
            project.build()
