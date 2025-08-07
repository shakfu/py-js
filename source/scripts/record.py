#!/usr/bin/env python3

import argparse
import configparser
import platform
import subprocess
import shlex
import sys
from pathlib import Path
from typing import Union

Pathlike = Union[Path, str]

# ----------------------------------------------------------------------------
# constants

PY_VERSION = "{v.major}.{v.minor}.{v.micro}".format(v=sys.version_info)


class BuildDB:
    project = "pyjs"
    system = platform.system()
    arch = platform.machine()

    def __init__(self, target: str, version: str = PY_VERSION):
        _parts = target.split('-')
        self.target = target
        self.name = _parts[0]
        self.variant = "-".join(_parts[1:])
        self.version = version
        self.root = Path.cwd()
        self.build_cache = (
            self.root / "source" / "projects" / "py" / "targets" / "build" / "build.ini"
        )

    @property
    def short_sha(self) -> str:
        return subprocess.check_output(
            shlex.split("git rev-parse --short HEAD")).decode().strip()

    @property
    def package_name(self) -> str:
        """ensure package name has standard format.
        `<project>-<name>-<variant>-<system>-<arch>-<version>`
        """
        project = self.project
        target = self.target
        system = self.system.lower()
        arch = self.arch
        ver = self.version
        return f"{project}-{target}-{system}-{arch}-{ver}".lower()

    @property
    def dmg(self) -> Path:
        """get final dmg package name and path"""
        package_name = self.get_package_name()
        dmg = self.root / f"{package_name}.dmg"
        return dmg.resolve()

    def print_metadata(self):
        print(f"pkg={self.package_name}")

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

    def record(self):
        self.cache_set(
            TARGET=self.target,
            NAME=self.name,
            VARIANT=self.variant,
            PACKAGE_NAME=self.package_name(),
            PRODUCT_DMG=self.dmg(),
        )


def main():
    """commandline api entrypoint"""

    parser = argparse.ArgumentParser(
        prog="stamp.py",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="build stamp",
    )
    opt = parser.add_argument

    opt("target", help="target name")
    opt("-v", "--version", help="python version", default=PY_VERSION)
    opt("-d", "--dmg", help="get dmg path", action="store_true")
    opt("-r", "--record", help="record a build", action="store_true")
    opt("-p", "--package-name", help="print package name", action="store_true")
    opt("-m", "--metadata", help="print metadata", action="store_true")

    args = parser.parse_args()

    db = BuildDB(args.target, args.version)
    if args.dmg:
        print(db.dmg)
        return
    elif args.package_name:
        print(db.package_name)
    elif args.record:
        db.record()
    elif args.metadata:
        github_output_path = os.environ.get('GITHUB_OUTPUT')

        # Check if GITHUB_OUTPUT is available (it should be in a GitHub Actions environment)
        if github_output_path:
            # Open the file in append mode and write the output
            with open(github_output_path, 'a') as fh:
                print(f'pkg={args.package_name}', file=fh)
        else:
            # Fallback for local testing or environments outside GitHub Actions
            print(f"GITHUB_OUTPUT environment variable not found.")

if __name__ == "__main__":
    main()
