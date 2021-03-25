"""basic: basic base builder class

The following classes are available:

    - Builder: provides generic builder interface and common file-handling features.

"""
import logging
import os
import shutil
import sys
from abc import ABC
from pathlib import Path

from ..config import IGNORE_ERRORS, LOG_FORMAT, LOG_LEVEL

logging.basicConfig(level=LOG_LEVEL, format=LOG_FORMAT, stream=sys.stdout)


class Builder(ABC):
    """Abstract class to provide builder interface and common features."""
    name: str
    version: str
    url_template: str
    depends_on: ['Builder']
    libs_static: [str]
    project_class: 'Project'
    mac_dep_target = '10.14'

    def __init__(self, project=None, version=None, depends_on=None):
        self.project = project if project else self.project_class()
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

    # -------------------------------------------------------------------------
    # Name / Version Methods

    @property
    def ver(self) -> str:
        """provides major.minor version: 3.9.1 -> 3.9"""
        return ".".join(self.version.split('.')[:2])

    @property
    def ver_nodot(self) -> str:
        """provides 'majorminor' version without space in between: 3.9.1 -> 39"""
        return self.ver.replace('.', '')

    @property
    def name_version(self) -> str:
        """Product-version: Python-3.9.1"""
        return f'{self.name}-{self.version}'

    @property
    def name_ver(self) -> str:
        """Product-major.minor: python-3.9"""
        return f'{self.name.lower()}{self.ver}'

    @property
    def name_archive(self) -> str:
        """Archival name of Product-version: Python-3.9.1.tgz"""
        return f'{self.name_version}.tgz'

    @property
    def dylib(self) -> str:
        """name of dynamic library in macos case."""
        return f'lib{self.name.lower()}{self.ver}.dylib' # pylint: disable=E1101

    # -------------------------------------------------------------------------
    # Path Methods

    @property
    def url(self) -> Path:
        """Returns url to download product as a pathlib.Path instance."""
        return Path(self.url_template.format(name=self.name,
                                             version=self.version))

    @property
    def download_path(self) -> Path:
        """Returns path to downloaded product-version archive."""
        return self.project.downloads / self.name_archive

    @property
    def src_path(self) -> Path:
        """Return product source directory."""
        return self.project.src / self.name_version

    @property
    def lib_path(self) -> Path:
        """alias to self.prefix"""
        return self.prefix

    @property
    def prefix(self) -> Path:
        """compiled product destination root directory."""
        return self.project.lib / self.name.lower()

    @property
    def prefix_lib(self) -> Path:
        """compiled product destination lib directory."""
        return self.prefix / 'lib'

    @property
    def prefix_include(self) -> Path:
        """compiled product destination include directory."""
        return self.prefix / 'include'

    @property
    def prefix_bin(self) -> Path:
        """compiled product destination bin directory."""
        return self.prefix / 'bin'

    # -------------------------------------------------------------------------
    # Test Methods


    def libs_static_exist(self) -> bool:
        """tests for existance of all provided static libs"""
        return all((self.prefix_lib / lib).exists() for lib in self.libs_static)

    # -------------------------------------------------------------------------
    # Generic Shell Methods

    def cmd(self, shellcmd, *args, **kwargs):
        """Run shell command with args and keywords"""
        _cmd = shellcmd.format(*args, **kwargs)
        self.log.info(_cmd)
        os.system(_cmd)

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
            shutil.rmtree(path, ignore_errors=IGNORE_ERRORS)
        else:
            self.log.info("remove file: %s", path)
            path.unlink(missing_ok=True)

    def recursive_clean(self, name, pattern):
        """generic recursive clean/remove method."""
        self.cmd(f'find {name} | grep -E "({pattern})" | xargs rm -rf')

    def install_name_tool(self, src, dst, mode='id'):
        """change dynamic shared library install names"""
        _cmd = f'install_name_tool -{mode} {src} {dst}'
        self.log.info(_cmd)
        self.cmd(_cmd)

    def xcodebuild(self, project, target=None):
        """build via xcode the given targets"""
        if not target:
            self.cmd(f'xcodebuild -project {project}')
        else:
            self.cmd(f'xcodebuild -project {project} -target {target}')

    # -------------------------------------------------------------------------
    # Core Methods

    def reset_prefix(self):
        """remove prefix or compilation destinations"""
        self.remove(self.prefix)

    def reset(self):
        """remove product src directory and compiled product directory."""
        self.remove(self.src_path)
        self.remove(self.prefix)  # aka self.prefix

    def download(self):
        """download src using curl and tar.

        curl and tar are automatically available on mac platforms.
        """
        self.project.downloads.mkdir(parents=True, exist_ok=True)
        for dep in self.depends_on:
            dep.download()

        # download
        if not self.download_path.exists():
            self.log.info("downloading %s", self.download_path)
            self.cmd(f'curl -L --fail {self.url} -o {self.download_path}')

        # unpack
        if not self.src_path.exists():
            self.project.src.mkdir(parents=True, exist_ok=True)
            self.log.info("unpacking %s", self.src_path)
            self.cmd(f'tar -C {self.project.src} -xvf {self.download_path}')

    def build(self):
        """build target from src"""

    def pre_process(self):
        """pre-build operations"""

    def post_process(self):
        """post-build operations"""
