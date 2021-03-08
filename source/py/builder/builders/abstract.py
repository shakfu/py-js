"""abstract: collection of abstract classes

The following classes are available:

    - Builder: provides generic builder interface and common file-handling features.


"""
import logging
import os
import shutil
import sys
from abc import ABC, abstractmethod
from pathlib import Path

from ..config import IGNORE_ERRORS, LOG_FORMAT, LOG_LEVEL, Project

logging.basicConfig(level=LOG_LEVEL, format=LOG_FORMAT, stream=sys.stdout)


class Builder(ABC):
    """Abstract class to provide builder interface and common features."""
    name: str
    version: str
    url_template: str
    depends_on: []
    libs_static = []

    def __init__(self, project, version=None, depends_on=None):
        self.project = project or Project()
        self.version = version or self.version
        self.depends_on = ([B(project) for B in depends_on] if depends_on else
                           [B(project) for B in self.depends_on])
        self.log = logging.getLogger(self.__class__.__name__)

    def __repr__(self):
        return f"<{self.__class__.__name__} '{self.name}-{self.version}'>"

    def __iter__(self):
        for dependency in self.depends_on:
            yield dependency
            for subdependency in iter(dependency):
                yield subdependency

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
        """Pproduct-major.minor: python-3.9"""
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
    def download_path(self):
        """Returns path to downloaded product-version archive."""
        return self.project.downloads / self.name_archive

    @property
    def src_path(self):
        """Return product source directory."""
        return self.project.src / self.name_version

    @property
    def lib_path(self):
        """alias to self.prefix"""
        return self.prefix

    @property
    def prefix(self):
        """compiled product destination root directory."""
        return self.project.lib / self.name.lower()

    @property
    def prefix_lib(self):
        """compiled product destination lib directory."""
        return self.prefix / 'lib'

    @property
    def prefix_include(self):
        """compiled product destination include directory."""
        return self.prefix / 'include'

    @property
    def prefix_bin(self):
        """compiled product destination bin directory."""
        return self.prefix / 'bin'

    def libs_static_exist(self):
        for lib in self.libs_static:
            if not (self.prefix_lib / lib).exists():
                return False
        return True

    def cmd(self, shellcmd, *args, **kwargs):
        """Run shell command with args and keywords"""
        os.system(shellcmd.format(*args, **kwargs))

    def chdir(self, path):
        """Change current workding directory to path"""
        os.chdir(path)

    def move(self, src, dst):
        """Move from src path to dst path."""
        shutil.move(src, dst)

    def copytree(self, src, dst):
        """Copy recursively from src path to dst path."""
        shutil.copytree(src, dst)

    def copyfile(self, src, dst):
        """Copy file from src path to dst path."""
        shutil.copyfile(src, dst)

    def remove(self, path):
        """Gemove file or folder."""
        if path.is_dir():
            shutil.rmtree(path, ignore_errors=IGNORE_ERRORS)
        else:
            path.unlink(missing_ok=True)

    def reset(self):
        """remove product src directory and compiled product directory."""
        self.remove(self.src_path)
        self.remove(self.prefix)  # aka self.prefix

    @abstractmethod
    def download(self):
        "download target src"

    @abstractmethod
    def build(self):
        "build target from src"
