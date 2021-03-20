# api.py: high level api of builder
import logging
import pathlib
from abc import ABC, abstractmethod

from ..models import Settings
from ..utils.text import Text

# -------------------------------------------------------------------------------
# MIXINS

class Shell:
    """Mixin for platform agnostic file/folder handling operations."""

    def cmd(self, shellcmd, *args, **kwargs):
        """Run shell command with args and keywords"""

    def chdir(self, path):
        """Change current workding directory to path"""

    def move(self, src, dst):
        """Move from src path to dst path."""

    def copytree(self, src, dst):
        """Copy recursively from src path to dst path."""

    def copyfile(self, src, dst):
        """Copy file from src path to dst path."""

    def remove(self, path):
        """Remove file or folder."""





# -------------------------------------------------------------------------------
# RECIPES


class Recipe(ABC):
    """A platform-specific container for multiple build projects.

    A recipe is analogous to a workspace in xcode
    """
    def __init__(self,
                 name: str = None,
                 projects: list["Project"] = None,
                 **settings):
        self.name = name or Text(
            self.__class__.__name__).mixed_to_word().abbreviate()
        self.settings = Settings(**settings)
        self.projects = projects

    @abstractmethod
    def reset(self):
        """clean projects in order of dependency"""

    @abstractmethod
    def clean(self):
        """clean projects in order of dependency"""

    @abstractmethod
    def build(self):
        """build projects in order of dependency"""

    @abstractmethod
    def install(self):
        """install projects in order of dependency"""


class PyJsRecipe(Recipe):
    def reset(self):
        """clean projects in order of dependency"""
        for project in self.projects:
            for dependent in project.depends_on:
                dependent.reset()
            project.reset()

    def clean(self):
        """clean projects in order of dependency"""
        for project in self.projects:
            for dependent in project.depends_on:
                dependent.clean()
            project.clean()

    def build(self):
        """build projects in order of dependency"""
        for project in self.projects:
            for dependent in project.depends_on:
                dependent.build()
            project.build()

    def install(self):
        """install projects in order of dependency"""
        for project in self.projects:
            for dependent in project.depends_on:
                dependent.install()
            project.install()

    # -------------------------------------------------------------------------
    # Class methods

    @classmethod
    def from_defaults(cls, name=None, project_classes=None, **settings):
        """Create from default class attributes."""

    @classmethod
    def from_tree_yaml(cls, path):
        """Create from hierarchical yaml file."""

    @classmethod
    def from_flat_yaml(cls, path):
        """Create from flat yaml file."""

    @classmethod
    def gen_from_flat_yaml(cls, path):
        """Generate code from flat yaml file."""


# -------------------------------------------------------------------------------
# PROJECTS


class Project(ABC):
    """A repository for all the files, resources, and information required to
    install one or more software products.
    """
    def __init__(
        self,
        name: str = None,
        builders: list["Builder"] = None,
        depends_on: ["Project"] = None,
        **settings,
    ):
        self.name = name
        self.depends_on = depends_on or []
        self.settings = Settings(**settings)
        self.builders = self.init_builders(builders) if builders else []

    def init_builders(self, builders):
        """Associate builders with parent project during initialization"""

    @classmethod
    def from_defaults(cls, name=None, builder_classes=None, **settings):
        """Create project instance from default class attributes."""

    @abstractmethod
    def clean(self):
        """shallow cleanse build"""

    @abstractmethod
    def reset(self):
        """reset (deep cleanse) build"""

    @abstractmethod
    def build(self):
        """build product"""

    @abstractmethod
    def install(self):
        """install product"""


class PythonProject(Project):
    """Project to build Python from source with different variations."""

    root = pathlib.Path.cwd()
    patch = root / "patch"
    targets = root / "targets"
    build = targets / "build"
    downloads = build / "downloads"
    src = build / "src"
    lib = build / "lib"


class PyJsProject(Project):
    """max external projects"""

    py_version_major = "3.9"
    py_version_minor = "2"
    py_semver = "3.9.2"
    py_version = py_version_major
    py_ver = "39"
    py_name = f"python{py_ver}"

    root = pathlib.Path.cwd()
    product = "python"
    support = root.parent.parent / "support"
    externals = root.parent.parent / "externals"
    source = root.parent / "source"
    frameworks = support / "Frameworks"
    scripts = root / "scripts"
    build = root / "targets" / "build"
    prefix = support / product
    bin = prefix / "bin"
    lib = prefix / "lib" / py_version
    dylib = f"libpython_{py_version}.dylib"

    py_external = externals / "py.mxo"
    PyJs_external = externals / "PyJs.mxo"

    bzip2_version = "1.0.8"
    ssl_version = "1.1.1g"
    mac_dep_target = "10.13"

    url_python = "https://www.python.org/ftp/python/${SEMVER}/Python-${SEMVER}.tgz"
    url_openssl = "https://www.openssl.org/source/openssl-${SSL_VERSION}.tar.gz"
    url_getpip = "https://bootstrap.pypa.io/get-pip.py"


class HomebrewProject(PyJsProject):
    """
    PREFIX=${SUPPORT}/${NAME}
    BIN=${SUPPORT}/${NAME}/bin
    LIB=${PREFIX}/lib/${NAME}
    HOMEBREW=/usr/local/opt/python3/Frameworks/Python.framework/Versions/${VERSION}
    """

    py_version_major = "3.9"
    py_version_minor = "2"
    py_semver = "3.9.2"
    py_version = py_version_major
    py_ver = "39"
    py_name = f"python{py_ver}"
    name = py_name

    root = pathlib.Path.cwd()
    product = "python"
    support = root.parent.parent / "support"
    externals = root.parent.parent / "externals"
    source = root.parent / "source"
    frameworks = support / "Frameworks"
    scripts = root / "scripts"
    build = root / "targets" / "build"
    prefix = support / product
    bin = prefix / "bin"
    lib = prefix / "lib" / py_version
    dylib = f"libpython_{py_version}.dylib"

    py_external = externals / "py.mxo"
    pyjs_external = externals / "pyjs.mxo"

    @property
    def prefix(self):
        """compiled product destination root directory."""
        return self.support / self.name


class SrcProject(PyJsProject):
    """py-js externals build from python source in different build formats"""


# -------------------------------------------------------------------------------
# PRODUCTS


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

    # -------------------------------------------------------------------------
    # Version Methods

    # @property
    def ver(self):
        """provides major.minor version: 3.9.1 -> 3.9"""

    # @property
    def ver_nodot(self):
        """provides 'majorminor' version without space in between: 3.9.1 -> 39"""

    # @property
    def name_version(self):
        """Product-version: Python-3.9.1"""

    # @property
    def name_ver(self):
        """Product-major.minor: python-3.9"""

    # -------------------------------------------------------------------------
    # Path Methods

    # @property
    def url(self):
        """Returns url to download product as a pathlib.Path instance."""

    # @property
    def name_archive(self):
        """Archival name of Product-version: Python-3.9.1.tgz"""

    # @property
    def download_path(self):
        """Returns path to downloaded product-version archive."""

    # @property
    def src_path(self):
        """Return product source directory."""

    # @property
    def lib_path(self):
        """alias to self.prefix"""

    # @property
    def prefix(self):
        """compiled product destination root directory."""

    # @property
    def prefix_lib(self):
        """compiled product destination lib directory."""

    # @property
    def prefix_include(self):
        """compiled product destination include directory."""

    # @property
    def prefix_bin(self) -> str:
        """compiled product destination bin directory."""

    # @property
    def dylib(self):
        """name of dynamic library in macos parlance."""

    def libs_static_exist(self) -> bool:
        """check that libs_static exists"""


# -------------------------------------------------------------------------------
# BUILDERS


class Builder(ABC):
    """A Builder know how to build a single product type in a project.

    A Builder is analagous to a Target in Xcode.
    """
    def __init__(self,
                 name: str = None,
                 version: str = None,
                 depends_on: ["Builder"] = None,
                 **settings):
        self.name = name
        self.version = version
        self.depends_on = depends_on or []
        self.settings = Settings(**settings)
        self.project = None
        self.product = None

    @abstractmethod
    def clean(self):
        """shallow cleanse build"""

    @abstractmethod
    def reset(self):
        """reset (deep cleanse) build"""

    @abstractmethod
    def build(self):
        """build product"""

    @abstractmethod
    def install(self):
        """install product"""


class BuilderCurrent(ABC, Shell):
    """Abstract class to provide builder interface and common features."""

    version: str
    url_template: str
    depends_on: []
    libs_static: []
    name = "osx"
    mac_dep_target = "10.14"

    def __init__(self, project, version=None, depends_on=None):
        self.project = project  # or Project()
        self.version = version or self.version
        self.depends_on = ([B(project) for B in depends_on] if depends_on else
                           [B(project) for B in self.depends_on])
        self.log = logging.getLogger(self.__class__.__name__)

    # -------------------------------------------------------------------------
    # Abstract Methods

    @abstractmethod
    def reset(self):
        """reset build state."""

    @abstractmethod
    def download(self):
        """download target src"""

    @abstractmethod
    def build(self):
        """build target from src"""

    @abstractmethod
    def pre_process(self):
        """pre-build operations"""

    @abstractmethod
    def post_process(self):
        """post-build operations"""

    # -------------------------------------------------------------------------
    # Version Methods

    # moved to Product

    # -------------------------------------------------------------------------
    # Path Methods

    # @property
    def url(self):
        """Returns url to download product as a pathlib.Path instance."""

    # @property
    def name_archive(self):
        """Archival name of Product-version: Python-3.9.1.tgz"""

    # @property
    def download_path(self):
        """Returns path to downloaded product-version archive."""

    # @property
    def src_path(self):
        """Return product source directory."""

    # @property
    def lib_path(self):
        """alias to self.prefix"""

    # @property
    def prefix(self):
        """compiled product destination root directory."""

    # @property
    def prefix_lib(self):
        """compiled product destination lib directory."""

    # @property
    def prefix_include(self):
        """compiled product destination include directory."""

    # @property
    def prefix_bin(self) -> str:
        """compiled product destination bin directory."""

    # @property
    def dylib(self):
        """name of dynamic library in macos parlance."""

    def libs_static_exist(self) -> bool:
        """check that libs_static exists"""


class PythonBuilder(Builder):
    """Generic Python from src builder."""

    name: str
    version: str
    depends_on: [str]
    suffix: str
    setup_local: str
    patch: str

    def __init__(self, project=None, version=None, depends_on=None):
        super().__init__(project, version, depends_on)

        # dependency manager attributes (revise)
        self.install_names = {}
        self.deps = []
        self.dep_list = []

    # ------------------------------------------------------------------------
    # python properties

    # @property
    def static_lib(self):
        """Name of static library: libpython.3.9.a"""

    # @property
    def python_lib(self):
        """python/lib/product.major.minor: python/lib/python3.9"""

    # @property
    def site_packages(self):
        """path to 'site-packages'"""

    # @property
    def lib_dynload(self):
        """path to 'lib-dynload'"""

    # ------------------------------------------------------------------------
    # src-level operations

    def download(self):
        """download requirements"""

    def reset(self):
        """reset to start"""

    def pre_process(self):
        """pre-build operations"""

    def post_process(self):
        """post-build operations"""

    def install(self):
        """install compilation product into lib"""
        self.reset()
        self.download()
        self.pre_process()
        self.build()
        self.post_process()

    # def install_python_pkg(self):
    #     self.install_python()
    #     self.fix_python_dylib_for_pkg()

    # def install_python_ext(self):
    #     self.install_python()
    #     self.fix_python_dylib_for_ext()

    # ------------------------------------------------------------------------
    # post-processing operations

    def is_valid_path(self, dep_path):
        """check if dependency path is a valid path."""

    def recursive_clean(self, name, pattern):
        """generic recursive clean/remove method."""

    def clean_python_pyc(self, name):
        """remove python .pyc files."""

    def clean_python_tests(self, name):
        """remove python tests files."""

    def rm_libs(self, names):
        """remove all named python dylib libraries"""

    def rm_exts(self, names):
        """remove all named extensions"""

    def rm_bins(self, names):
        """remove all named binary executables"""

    def clean_python_site_packages(self):
        """remove python site-packages"""

    def remove_packages(self):
        """remove list of non-critical packages"""

    def remove_extensions(self):
        """remove extensions: not implemented"""

    def remove_binaries(self):
        """remove list of non-critical executables"""

    def clean(self):
        """clean everything."""

    def ziplib(self):
        """zip python package in site-packages in .zip archive"""

    def fix_python_dylib_for_pkg(self):
        """fix dynamic lib to point to @loader_path in pkg"""

    def fix_python_dylib_for_ext(self):
        """fix dynamic lib to point to @loader_path in ext"""


class HomebrewBuilder(Builder):
    """Builder for Hombrew based PyJs products"""
    
    def cp_pkg(self, pkg):
        """copy homebrew package to prefix"""

    def clean_python(self):
        """clean python from detritus"""

    def fix_python_exec(self):
        """change executable references to make them portable"""

    def fix_python_dylib_for_pkg(self):
        """change dylib references to make them portable"""

    def fix_python_dylib_for_ext_executable(self):
        """change dylib for external format"""

    def fix_python_dylib_for_ext_executable_name(self):
        """change dylib refs for external"""

    def fix_python_dylib_for_ext_resources(self):
        """change dylib refs for ext resources"""

    def cp_python_to_ext_resources(self):
        """copy python to external resources"""

    def install_python(self):
        """install homebrew package to new destination"""

    def install_python_pkg(self):
        """install python to pkg structure"""

    def install_python_ext(self):
        """install python to ext structure"""
