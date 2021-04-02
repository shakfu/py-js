
import re


class Product:
    """Associates a name and version and provides different formats."""

    _REGEX = re.compile(
        r"""
            ^
            (?P<major>0|[1-9]\d*)
            \.
            (?P<minor>0|[1-9]\d*)
            \.
            (?P<patch>0|[1-9]\d*)
            (?P<tag>0|\w*)
            $
        """,
        re.VERBOSE,
    )

    def __init__(self, name, version, url=None, **kwds):
        self.name = name
        self.version = version
        self.url = url
        self.kwds = kwds

        match = self._REGEX.match(version)
        if not match:
            raise ValueError(f"{version} is not valid semantic version string")
        matched_version_parts = match.groupdict()
        self._from_version_parts(**matched_version_parts)

    def __str__(self):
        return f"{self.name}{self.version}"

    def __repr__(self):
        return f"<Metadata '{self.name}{self.version}'>"

    def __hash__(self) -> int:
        return hash(self.to_tuple()[:4])

    def _from_version_parts(self, major, minor, patch, tag=None,):
        # Build a dictionary of the arguments except prerelease and build
        version_parts = {"major": int(major), "minor": int(minor), "patch": int(patch)}

        for name, value in version_parts.items():
            if value < 0:
                raise ValueError(
                    "{!r} is negative. A version can only be positive.".format(name)
                )

        self._major = version_parts["major"]
        self._minor = version_parts["minor"]
        self._patch = version_parts["patch"]
        self._tag = None if tag is None else str(tag)

    @property
    def major(self) -> int:
        """The major part of a version (read-only)."""
        return self._major

    @property
    def minor(self) -> int:
        """The minor part of a version (read-only)."""
        return self._minor

    @property
    def patch(self) -> int:
        """The patch part of a version (read-only)."""
        return self._patch

    @property
    def tag(self) -> str:
        """The tag part of a version (read-only)."""
        return self._tag

    def to_tuple(self) -> tuple:
        """convert name version object to tuple"""
        return (self.name, self.major, self.minor, self.patch, self.tag)

    def to_dict(self) -> dict:
        """convert name version object to dict"""
        return dict(
            name=self.name,
            major=self.major,
            minor=self.minor,
            tag=self.tag,
        )

    def finalize_version(self) -> "Version":
        """remove tag if any and create semantic version compatible instance"""
        cls = type(self)
        return cls(self.name, f"{self.major}.{self.minor}.{self.patch}")

    # -------------------------------------------------------------------------
    # Name Version format variation properties

    @property
    def ver(self) -> str:
        """provides major.minor version: 3.9.1 -> 3.9"""
        return f"{self.major}.{self.minor}"

    @property
    def ver_nodot(self) -> str:
        """provides 'majorminor' version without space in between: 3.9.1 -> 39"""
        return f"{self.major}{self.minor}"

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
    def dylib(self) -> str:
        """name of dynamic library in macos case."""
        return f"lib{self.name.lower()}{self.ver}.dylib"  # pylint: disable=E1101

    @property
    def staticlib(self) -> str:
        """name of dynamic library in macos case."""
        return f"lib{self.name.lower()}{self.ver}.a"  # pylint: disable=E1101


class Builder:
    """Abstract class to provide builder interface and common features."""
    product_class: Product
    depends_on: [Product]

    def __init__(self, project=None, product=None):
        # domain data
        self.project = project
        self.product = product or self.product_class(project)

        # tools
        self.cmd = ShellCmd(self.log)
        self.log = logging.getLogger(self.__class__.__name__)

    def __repr__(self):
        return f"<{self.__class__.__name__}>"

    @property
    def src_path(self):
        """Return product source directory."""
        return self.project.src / self.product.name_version

    @property
    def prefix(self):
        """Compiled product destination root directory."""
        return self.product.home

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

    # -----------------------------------------------------------------------------
    # Actions

    def reset(self):
        """Remove product src directory and compiled product directory."""
        self.cmd.remove(self.src_path)
        self.cmd.remove(self.prefix)  # aka self.prefix

    def clean(self):
        """Remove product src directory and compiled product directory."""

    def download(self):
        """Download target src."""

    def pre_build(self):
        """Pre-build operations."""

    def build(self):
        """Build target from src."""

    def post_build(self):
        """Post-build operations."""

    def install(self):
        """deploy product in its final destination."""



class Inner:
    a = 10


class P:
    b = Inner()
    def __init__(self, **kwds):
        self.kwds = kwds
    def cmd(self, shellcmd, *args, **kwds):
        if kwds:
            env = self.kwds.copy()
            env.update(kwds)
        else:
            env = self.kwds
        return shellcmd.format(*args, **env)




registry = {}


class MyProduct(Product):
    builder_class = None

class MyBuilder(Builder):
    product_class: MyProduct
    depends_on = []

registry['MyBuilder'] = MyProduct
registry['MyProduct'] = MyBuilder
