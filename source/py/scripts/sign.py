#!/usr/bin/env python3
"""sign.py

Provides a utility class which recursively walks through a bundle or folder structure
and signs all of the internal binaries which fit the given pattern

Note: you can reduce the logging verbosity by making DEBUG=False

"""

import argparse
import logging
import os
import pathlib
import subprocess
import sys


DEBUG = True

PYTHON_VER = "python{x}.{y}".format(
    x=sys.version_info.major, 
    y=sys.version_info.minor)

logging.basicConfig(
    format="%(asctime)s - %(levelname)s - %(message)s",
    datefmt="%H:%M:%S",
    level=logging.DEBUG if DEBUG else logging.INFO
)


class CodesignExternal:
    """Recursively codesign an external."""

    FILE_PATTERNS = {PYTHON_VER: 'runtime'}
    FILE_EXTENSIONS = ['.so', '.dylib']
    FOLDER_EXTENSIONS = ['.framework', '.mxo', '.bundle']

    def __init__(self, path: str, dev_id: str = None, 
                 entitlements: str = None, dry_run: bool = False):
        self.path = path
        self.authority = f"Developer ID Application: {dev_id}"
        self.entitlements = entitlements
        self.dry_run = dry_run
        self.targets_runtimes = set()
        self.targets_internals = set()
        self.log = logging.getLogger(self.__class__.__name__)
        self._cmd_codesign = [
            "codesign",
            "--sign", repr(self.authority),
            "--timestamp",
            "--deep",
            "--force",
        ]

    def cmd(self, shellcmd, *args, **kwds):
        """run system command"""
        syscmd = shellcmd.format(*args, **kwds)
        self.log.debug(syscmd)
        os.system(syscmd)

    def cmd_check(self, arglist):
        """capture and check shell _cmd output."""
        res = subprocess.run(
            arglist,
            capture_output=True,
            encoding="utf8",
            check=True,
        )
        if res.returncode != 0:
            self.log.critical(res.stderr)
        else:
            self.log.debug(" ".join(['DONE']+arglist))

    def is_binary(self, path):
        """returns True if file is a binary file."""
        txt = self.cmd_check(["file", "-b", str(path)])
        if txt:
            return "binary" in txt.split()
        else:
            return False

    def collect(self):
        """build up a list of target binaries"""
        for root, folders, files in os.walk(self.path):
            for fname in files:
                path = pathlib.Path(root) / fname
                for pattern in self.FILE_PATTERNS:
                    if fname == pattern:
                        if self.FILE_PATTERNS[fname] == 'runtime':
                            self.targets_runtimes.add(path)
                        else:
                            self.targets_internals.add(path)
                for ext in self.FILE_EXTENSIONS:
                    if path.suffix not in self.FILE_EXTENSIONS:
                        continue
                    if path.is_symlink():
                        continue
                    if path.suffix in self.FILE_EXTENSIONS:
                        self.log.debug("added binary: %s", path)
                        self.targets_internals.add(path)
            for folder in folders:
                path = pathlib.Path(root) / folder
                for ext in self.FOLDER_EXTENSIONS:
                    if path.suffix not in self.FOLDER_EXTENSIONS:
                        continue
                    if path.is_symlink():
                        continue
                    if path.suffix in self.FOLDER_EXTENSIONS:
                        self.log.debug("added bundle: %s", path)
                        self.targets_internals.add(path)

    def sign_internal_binary(self, path: pathlib.Path):
        """sign internal binaries"""
        codesign_cmd = " ".join(self._cmd_codesign + [str(path)])
        self.cmd(codesign_cmd)
        # self.cmd_check(self._cmd_codesign + [str(path)])

    def sign_runtime(self, path=None):
        """sign top-level bundle runtime"""
        if not path:
            path = self.path
        codesign_runtime = " ".join(self._cmd_codesign + [
             "--options", "runtime",
             "--entitlements", str(self.entitlements),
             str(path)
        ])
        self.cmd(codesign_runtime)
        # self.cmd_check(self._cmd_codesign + [
        #      "--options", "runtime", 
        #      "--entitlements", str(self.entitlements), 
        #      str(self.path)
        # ])

    def process(self):
        """main process to recursive remove unneeded arch."""

        if not self.targets_internals:
            self.collect()

        for path in self.targets_internals:
            if not self.dry_run:
                self.sign_internal_binary(path)

        if not self.dry_run:
            for path in self.targets_runtimes:
                self.sign_runtime(path)
            # sign main runtime
            self.sign_runtime()
        self.log.info("DONE!")

    @classmethod
    def cmdline(cls):
        """commandline interface to class."""
        parser = argparse.ArgumentParser(description=cls.__doc__)
        option = parser.add_argument
        option("path", type=str, help="a folder containing binaries to shrink")
        option("--dev-id", "-i", help="Developer ID")
        option("--entitlements", "-e", default="entitlements.plist", help="path to entitlements.plist")
        option("--dry-run", "-d", action="store_true", help="run without actual changes.")
        args = parser.parse_args()
        if args.path:
            app = cls(args.path, args.dev_id, args.entitlements, args.dry_run)
            app.process()


if __name__ == "__main__":
    CodesignExternal.cmdline()
