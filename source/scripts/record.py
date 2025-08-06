#!/usr/bin/env python3

import argparse
import configparser
import platform
import sys
from pathlib import Path


# ----------------------------------------------------------------------------
# constants

Pathlike = Path | str
PY_VERSION = "{v.major}.{v.minor}.{v.micro}".format(v=sys.version_info)


class BuildRecord:
    project = "pyjs"
    system = platform.system()
    arch = platform.machine()

    def __init__(self, name: str, variant: str, version: str = PY_VERSION):
        self.name = name
        self.variant = variant
        self.version = version
        self.root = Path.cwd()
        self.build_cache = (
            self.root / "source" / "projects" / "py" / "targets" / "build" / "build.ini"
        )

    def cache_set(self, **kwds):
        config = configparser.ConfigParser()
        config["cache"] = kwds
        with open(self.build_cache, "w") as configfile:
            config.write(configfile)

    def cache_get(self, key: str, as_path=False) -> Pathlike:
        config = configparser.ConfigParser()
        config.read(self.build_cache)
        value = config["cache"][key]
        if as_path:
            value = Path(value)
        return value

    def get_package_name(self) -> str:
        """ensure package name has standard format.
        `<project>-<name>-<variant>-<system>-<arch>-<version>`
        """
        project = self.project
        name = self.name
        variant = self.variant
        system = self.system.lower()
        arch = self.arch
        ver = self.version
        return f"{project}-{name}-{variant}-{system}-{arch}-{ver}"

    def get_dmg(self) -> Path:
        """get final dmg package name and path"""
        package_name = self.get_package_name()
        dmg = self.root / f"{package_name}.dmg"
        return dmg.resolve()

    def record(self):
        self.cache_set(
            NAME=self.name,
            VARIANT=self.variant,
            PRODUCT_DMG=self.get_dmg(),
        )


def main():
    """commandline api entrypoint"""

    parser = argparse.ArgumentParser(
        prog="stamp.py",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="build stamp",
    )
    opt = parser.add_argument

    opt("name", help="external name")
    opt("variant", help="build variant")
    opt("-v", "--version", help="python version", default=PY_VERSION)
    opt("-d", "--dmg", help="get dmg path")

    args = parser.parse_args()

    rec = BuildRecord(args.name, args.variant, args.version)
    if args.dmg:
        print(rec.get_dmg())
        return
    rec.record()


if __name__ == "__main__":
    main()
