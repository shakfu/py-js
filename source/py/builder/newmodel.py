import logging
from abc import ABC
from pathlib import Path
from types import SimpleNamespace


from utils.shell import ShellCmd


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
    name = 'prototype1'
    version = '0.0.1'
    libs_static = []
    url_template: str

    def __init__(self, name: str, version: str = None, path: Path = None):
        self.name = name or self.name
        self.version = version or self.version
        self.path = Path(path) if path else None

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def exists(self) -> bool:
        """returns True if product exists at self.path"""
        return self.path.exists()

    @property
    def ver(self):
        """provides major.minor version: 3.9.1 -> 3.9"""
        return ".".join(self.version.split('.')[:2])

    @property
    def ver_nodot(self):
        """provides 'majorminor' version without space in between: 3.9.1 -> 39"""
        return self.ver.replace('.', '')

    @property
    def name_version(self):
        """Product-version: Python-3.9.1"""
        return f'{self.name}-{self.version}'

    @property
    def name_ver(self):
        """Product-major.minor: python-3.9"""
        return f'{self.name.lower()}{self.ver}'

    @property
    def url(self):
        """Returns url to download product as a pathlib.Path instance."""
        return Path(
            self.url_template.format(name=self.name, version=self.version))

    @property
    def name_archive(self):
        """Archival name of Product-version: Python-3.9.1.tgz"""
        return f'{self.name_version}.tgz'

    @property
    def dst(self):
        """compiled product destination root directory."""
        # return self.project.lib / self.name.lower()
        return self.path / self.name.lower()

    @property
    def prefix(self):
        """compiled product destination root directory."""
        # return self.project.lib / self.name.lower()
        return self.dst

    @property
    def prefix_lib(self):
        """compiled product destination lib directory."""
        return self.dst / 'lib'

    @property
    def prefix_include(self):
        """compiled product destination include directory."""
        return self.dst / 'include'

    @property
    def prefix_bin(self):
        """compiled product destination bin directory."""
        return self.dst / 'bin'

    @property
    def has_static_libs(self):
        for lib in self.libs_static:
            if not (self.prefix_lib / lib).exists():
                return False
        return True


class Builder(ABC):
    """Abstract class to provide builder interface and common features."""
    product_class = None
    depends_on: []

    def __init__(self, project=None, product=None):
        self.project = project
        self.product = product if product else self.product_class(project)
        self.log = logging.getLogger(self.__class__.__name__)
        self.cmd = ShellCmd(self.log)

    def __repr__(self):
        return f"<{self.__class__.__name__}>"

    @property
    def version(self):
        """provides major.minor.patch version: 3.9.1"""
        return str(self.product.version)

    @property
    def ver(self):
        """provides major.minor version: 3.9.1 -> 3.9"""
        return self.product.ver

    @property
    def ver_nodot(self):
        """provides 'majorminor' version without space in between: 3.9.1 -> 39"""
        return self.product.ver_nodot

    @property
    def name_version(self):
        """Product-version: Python-3.9.1"""
        return self.product.name_version

    @property
    def name_ver(self):
        """Product-major.minor: python-3.9"""
        return self.product.name_ver

    @property
    def url(self):
        """Returns url to download product as a pathlib.Path instance."""
        return Path(self.product.url)

    @property
    def name_archive(self):
        """Archival name of Product-version: Python-3.9.1.tgz"""
        return self.product.archive

    @property
    def download_path(self):
        """Returns path to downloaded product-version archive."""
        return self.project.downloads / self.name_archive

    @property
    def src_path(self):
        """Return product source directory."""
        return self.project.src / self.name_version

    @property
    def prefix(self):
        """Compiled product destination root directory."""
        return self.product.dst

    @property
    def prefix_lib(self):
        """Compiled product destination lib directory."""
        return self.product.lib

    @property
    def prefix_include(self):
        """Compiled product destination include directory."""
        return self.product.include

    @property
    def prefix_bin(self):
        """Compiled product destination bin directory."""
        return self.product.bin

    @property
    def product_static_libs_exist(self):
        return self.product.has_static_libs()

    def reset(self):
        """Remove product src directory and compiled product directory."""
        self.cmd.remove(self.src_path)
        self.cmd.remove(self.prefix)  # aka self.prefix

    def download(self):
        """Download target src."""

    def build(self):
        """Build target from src."""

    def pre_process(self):
        """Pre-build operations."""

    def post_process(self):
        """Post-build operations."""


class Project(ABC):
    """A repository for all the files, resources, and information required to
    build one or more software products.
    """
    builder_classes: ['Builder']

    def __init__(self, name: str = None, root: Path = None, builders: list[Builder] = None,
                 depends_on: ['Project'] = None, **settings):
        self.name = name
        self.root = root or Path('.')
        self.depends_on = depends_on or []
        self.settings = Settings(**settings)
        self.builders = self.init_builders(builders) if builders else []

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def __iter__(self):
        for dep in self.depends_on:
            yield dep
            for subdep in iter(dep):
                yield subdep

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
            builder_name = None
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


class PythonProduct(Product):
    name = 'Python'
    version = '3.9.2'
    libs_static = ['libpython3.9.a']


class MyBuilder(Builder):
    product_class = PythonProduct
    depends_on = []
