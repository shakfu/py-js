"""Named Version handling.

Credit: versioning part heavily derived from semver packages's Version class,
but does not follow strict semantic versioning.

For version part:
     major.minor.patch<optional-alpahnumeric-tag>

     so

     1.1.3a is compatiable (would not be compatible to semantic version)
"""

import re


class NameVersion:
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

    def __init__(self, name, version):
        self.name = name
        self.version = version

        match = self._REGEX.match(version)
        if not match:
            raise ValueError(f"{version} is not valid semantic version string")
        matched_version_parts = match.groupdict()
        self._from_version_parts(**matched_version_parts)

    def __str__(self):
        return f"{self.name}{self.version}"

    def __repr__(self):
        return f"<NameVersion '{self.name}{self.version}'>"

    def __hash__(self) -> int:
        return hash(self.to_tuple()[:4])

    def _from_version_parts(self, major, minor, patch, tag=None):
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


if __name__ == "__main__":
    v = NameVersion("Python", "3.9.2")
