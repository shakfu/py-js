#!/usr/bin/env python3
"""sign.py

Provides a utility class which applies a recursive `lipo -remove` for
binaries of a given architecture in a folder.

Note: you can reduce the logging verbosity by making DEBUG=False

"""

import argparse
import logging
import os
import pathlib
import subprocess


DEBUG = True

logging.basicConfig(
    format="%(asctime)s - %(levelname)s - %(message)s",
    datefmt="%H:%M:%S",
    level=logging.DEBUG if DEBUG else logging.INFO
)

FILE_EXTENSIONS = ['.so', '.dylib']
FOLDER_EXTENSIONS = ['.framework', '.mxo', '.bundle']

class CodesignExternal:
    """Recursively codesign an external."""

    def __init__(self, path: str, dev_id: str = None, 
                 entitlements: str = None, dry_run: bool = False):
        self.path = path
        self.authority = f"Developer ID Application: {dev_id}"
        self.entitlements = entitlements
        self.dry_run = dry_run
        self.targets = set()
        self.log = logging.getLogger(self.__class__.__name__)
        self._cmd_codesign = [
            "codesign", 
            "--sign", repr(self.authority), 
            "--timestamp", 
            "--deep",
            "--force",
        ]

    def _cmd(self, shellcmd, *args, **kwds):
        """run system command"""
        syscmd = shellcmd.format(*args, **kwds)
        self.log.debug(syscmd)
        os.system(syscmd)

    def _cmd_check(self, arglist):
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
        txt = self._cmd_output(["file", "-b", str(path)])
        return "binary" in txt.split()

    def collect(self):
        """build up a list of target binaries"""
        for root, folders, files in os.walk(self.path):
            for fname in files:
                path = pathlib.Path(root) / fname
                for ext in FILE_EXTENSIONS:
                    if path.suffix not in FILE_EXTENSIONS:
                        continue
                    if path.is_symlink():
                        continue
                    if path.suffix in FILE_EXTENSIONS:
                        self.log.debug("added binary: %s", path)
                        self.targets.add(path)
            for folder in folders:
                path = pathlib.Path(root) / folder
                for ext in FOLDER_EXTENSIONS:
                    if path.suffix not in FOLDER_EXTENSIONS:
                        continue
                    if path.is_symlink():
                        continue
                    if path.suffix in FOLDER_EXTENSIONS:
                        self.log.debug("added bundle: %s", path)
                        self.targets.add(path)

    def sign_internal_binary(self, path: pathlib.Path):
        """sign internal binaries"""
        codesign_cmd = " ".join(self._cmd_codesign + [str(path)])
        self._cmd(codesign_cmd)
        # self._cmd_check(self._cmd_codesign + [str(path)])

    def sign_runtime(self):
        """sign top-level bundle runtime"""
        # "codesign", "-s", repr(self.authority), "--timestamp", "--deep"
        codesign_runtime = " ".join(self._cmd_codesign + [
             "--options", "runtime", 
             "--entitlements", str(self.entitlements), 
             str(self.path)
        ])
        self._cmd(codesign_runtime)
        # self._cmd_check(self._cmd_codesign + [
        #      "--options", "runtime", 
        #      "--entitlements", str(self.entitlements), 
        #      str(self.path)
        # ])

    def process(self):
        """main process to recursive remove unneeded arch."""

        if not self.targets:
            self.collect()

        for path in self.targets:
            if not self.dry_run:
                self.sign_internal_binary(path)

        if not self.dry_run:
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
            cls(args.path, args.dev_id, args.entitlements, args.dry_run).process()


if __name__ == "__main__":
    CodesignExternal.cmdline()
