"""model: schema of builder

Basically a simplified copy of the xcode model. The only difference is that I have
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
import os
import shutil
from abc import ABC
from pathlib import Path
import logging
from types import SimpleNamespace
from typing import Type, List

DEBUG = True
if DEBUG:
    LOG_LEVEL = logging.DEBUG
else:
    LOG_LEVEL = logging.INFO
LOG_FORMAT = '%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s'


class ShellCmd:
    """Provides platform agnostic file/folder handling."""

    def __init__(self, log):
        self.log = log

    def cmd(self, shellcmd, *args, **kwargs):
        """Run shell command with args and keywords"""
        _cmd = shellcmd.format(*args, **kwargs)
        self.log.info(_cmd)
        os.system(_cmd)

    __call__ = cmd

    def chdir(self, path):
        """Change current workding directory to path"""
        self.log.info("changing working dir to: %s", path)
        os.chdir(path)

    def chmod(self, path, perm=0o777):
        """Change permission of file"""
        self.log.info("change permission of %s to %s", path, perm)
        os.chmod(path, perm)

    def move(self, src, dst):
        """Move from src path to dst path."""
        self.log.info("move path %s to %s", src, dst)
        shutil.move(src, dst)

    def copytree(self, src, dst):
        """Copy recursively from src path to dst path."""
        self.log.info("move tree %s to %s", src, dst)
        shutil.copytree(src, dst)

    def copyfile(self, src, dst):
        """Copy file from src path to dst path."""
        self.log.info("copy %s to %s", src, dst)
        shutil.copyfile(src, dst)

    def remove(self, path):
        """Remove file or folder."""
        if path.is_dir():
            self.log.info("remove folder: %s", path)
            shutil.rmtree(path, ignore_errors=not DEBUG)
        else:
            self.log.info("remove file: %s", path)
            path.unlink(missing_ok=True)


class Settings(SimpleNamespace):
    """A dictionary object with dotted access to its members.

    >>> settings = Settings(**dict)
    """

    def copy(self) -> 'Settings':
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
    default_name: str
    default_version: str
    libs_static: list[str]

    def __init__(self, name: str = None, version: str = None, path: Path = None,
                url_template: str = None):
        self.name = name or self.default_name
        self.version = version or self.default_version
        self.path = Path(path) if path else None
        self.url_template = url_template

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def exists(self) -> bool:
        """returns True if product exists at self.path"""
        return self.path.exists()

    @property
    def ver(self) -> str:
        """provides major.minor version: 3.9.1 -> 3.9"""
        return ".".join(self.version.split(".")[:2])

    @property
    def ver_nodot(self) -> str:
        """provides 'majorminor' version without space in between: 3.9.1 -> 39"""
        return self.ver.replace(".", "")

    @property
    def name_version(self) -> str:
        """Product-version: Python-3.9.1"""
        return f"{self.name}-{self.version}"

    @property
    def name_ver(self) -> str:
        """Product-major.minor: python-3.9"""
        return f"{self.name.lower()}{self.ver}"

    @property
    def name_archive(self) -> str:
        """Archival name of Product-version: Python-3.9.1.tgz"""
        return f"{self.name_version}.tgz"

    @property
    def url(self) -> Path:
        """Returns url to download product src as a pathlib.Path instance."""
        return Path(self.url_template.format(name=self.name, version=self.version))

    @property
    def dst(self) -> Path:
        """compiled product destination root directory."""
        # return self.project.lib / self.name.lower()
        return self.path / self.name.lower()

    @property
    def prefix(self) -> Path:
        """compiled product destination root directory."""
        # return self.project.lib / self.name.lower()
        return self.dst

    @property
    def prefix_lib(self) -> Path:
        """compiled product destination lib directory."""
        return self.dst / "lib"

    @property
    def prefix_include(self) -> Path:
        """compiled product destination include directory."""
        return self.dst / "include"

    @property
    def prefix_bin(self) -> Path:
        """compiled product destination bin directory."""
        return self.dst / "bin"

    @property
    def has_static_libs(self) -> bool:
        """check for presence of static libs"""
        return all((self.prefix_lib / lib).exists() for lib in self.libs_static)


class Builder(ABC):
    """A Builder know how to build a single product type in a project.

    A Builder is analagous to a Target in Xcode.
    """
    product_class: Type[Product]

    def __init__(self, product: Product = None, project: 'Project' = None,
                 depends_on: list["Builder"] = None, **settings):
        self.product = product if product else self.product_class()
        self.project = project
        self.depends_on = depends_on or []
        self.settings = Settings(**settings)
        self.log = logging.getLogger(self.__class__.__name__)
        self.cmd = ShellCmd(self.log)

    def __str__(self):
        return f"<{self.__class__.__name__}>"

    def recursive_clean(self, path, pattern):
        """generic recursive clean/remove method."""
        self.cmd(f'find {path} | grep -E "({pattern})" | xargs rm -rf')

    def install_name_tool(self, src, dst, mode="id"):
        """change dynamic shared library install names"""
        _cmd = f"install_name_tool -{mode} {src} {dst}"
        self.log.info(_cmd)
        self.cmd(_cmd)

    def xcodebuild(self, project, target, flag=None):
        """build via xcode the given targets"""
        if not flag:
            self.cmd(
                f"xcodebuild -project targets/{project}/py-js.xcodeproj -target {target}"
            )
        else:
            _flag = f"{flag}=1"
            self.cmd(
                f"xcodebuild -project targets/{project}/py-js.xcodeproj -target {target} "
                f"GCC_PREPROCESSOR_DEFINITIONS='$GCC_PREPROCESSOR_DEFINITIONS {_flag}'"
            )

    def xbuild_targets(self, project, targets=None, flag=None):
        """build via xcode the given targets"""
        for target in targets:
            self.xcodebuild(project, target, flag)

    def clean(self):
        """shallow cleanse build"""
        for builder in self.depends_on:
            builder.clean()

    def reset(self):
        """reset (deep cleanse) build"""
        for builder in self.depends_on:
            builder.reset()

    def build(self):
        """build product"""
        for builder in self.depends_on:
            builder.build()

    def install(self):
        """install product"""
        for builder in self.depends_on:
            builder.install()


class Project(ABC):
    """A repository for all the files, resources, and information required to
    build one or more software products.
    """
    builder_classes: List[Type['Builder']]

    def __init__(self, name: str = None, builders: list['Builder'] = None, **settings):
        self.name = name
        self.builders = builders if builders else [
            builder_class(project=self) for builder_class in self.builder_classes]
        self.settings = Settings(**settings)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def build(self):
        """delegate building to builders"""
        for builder in self.builders:
            builder.build()


class ProjectRecipe(ABC):
    """A project-centric platform-specific container for multiple build projects.

    A recipe is analogous to a workspace in xcode
    """
    project_classes: List[Type['Project']]

    def __init__(self, name: str = None, projects: list[Project] = None, **settings):
        self.name = name
        self.settings = Settings(**settings)
        self.projects = projects if projects else [
            project_class() for project_class in self.project_classes]

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    __repr__ = __str__

    def build(self):
        """build projects"""
        for project in self.projects:
            project.build()


class BuilderRecipe(ABC):
    """A platform-specific container for multiple builder-centric projects.

    A recipe is analogous to a workspace in xcode
    """

    def __init__(self, name: str = None, builders: list[Builder] = None, **settings):
        self.name = name
        self.settings = Settings(**settings)
        self.builders = builders

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    __repr__ = __str__

    def build(self):
        """build builders"""
        for builder in self.builders:
            builder.build()
