#!/usr/bin/env python3
"""sign.py

Provides a utility class which recursively walks through a bundle or folder structure
and signs all of the internal binaries which fit the given pattern

Note: you can reduce the logging verbosity by making DEBUG=False


Steps to sign a Max package with a externals in the 'externals' folder
depending on a framework or two in the 'support' folder:

1. codesign externals [<name>.mxo, ...] in 'externals' folder

builder.sign_folder('externals')

2. codesign frameworks or libraries [<name>.framework | python<ver> | ...]

builder.sign_folder('support')

3. create package as folder then convert to .dmg

    - create $package folder
    - copy or use ditto to put everything into $package
    - covert folder into .dmg

builder.package_as_dmg()

    - defaults to project name


4. notarize $package.dmg

builder.notarize_dmg()


5. staple $package.dmg

builder.staple_dmg()

"""

import argparse
import logging
import os
import pathlib
import subprocess
import sys
from pathlib import Path

from .config import Project
from .shell import ShellCmd


DEBUG = True

logging.basicConfig(
    format="%(asctime)s - %(levelname)s - %(message)s",
    datefmt="%H:%M:%S",
    level=logging.DEBUG if DEBUG else logging.INFO
)


class CodesignExternal:
    """Recursively codesign an external."""
    FILE_PATTERNS = {Project.python.version_short: 'runtime'}
    FILE_EXTENSIONS = ['.so', '.dylib']
    FOLDER_EXTENSIONS = ['.framework', '.mxo', '.bundle', '.app']

    def __init__(self, path: str, dev_id: str = None, 
                 entitlements: str = None, dry_run: bool = False):
        print(path)
        self.path = path
        self.authority = f"Developer ID Application: {dev_id}"
        self.entitlements = entitlements
        self.dry_run = dry_run
        self.targets_runtimes = set()
        self.targets_internals = set()
        self.targets_apps = set()
        self.log = logging.getLogger(self.__class__.__name__)
        # self.cmd = ShellCmd(self.log)
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
                        if path.suffix == '.app':
                            self.targets_apps.add(path)
                        else:
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

        for path in self.targets_apps:
            macos_path = path / 'Contents' / 'MacOS'
            for exe in macos_path.iterdir():
                if not self.dry_run:
                    self.sign_internal_binary(path)
            self.sign_runtime(path)

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
        option("path", type=str, help="a folder containing binaries to sign")
        option("--dev-id", "-i", help="Developer ID")
        option("--entitlements", "-e", default="entitlements.plist", help="path to entitlements.plist")
        option("--dry-run", "-d", action="store_true", help="run without actual changes.")
        args = parser.parse_args()
        if args.path:
            app = cls(args.path, args.dev_id, args.entitlements, args.dry_run)
            app.process()


def sign_folder(folder='externals'):
    dev_id = os.environ['DEV_ID']
    assert dev_id, "environment var DEV_ID not set"
    root = pathlib.Path(__file__).parent.parent.parent.parent
    target_folder = pathlib.Path(root / folder)
    entitlements = pathlib.Path(root / 'source/py/resources/entitlements/entitlements.plist')    
    assert target_folder.exists()
    assert entitlements.exists()
    targets = list(target_folder.iterdir())
    assert len(targets) > 0, "no targets to sign"
    for target in targets:
        if target.suffix in CodesignExternal.FOLDER_EXTENSIONS:
            signer = CodesignExternal(target, dev_id=dev_id, entitlements=entitlements)
            signer.process()


def package(package_name=Project.package_name):
    log = logging.getLogger('packager')
    cmd = ShellCmd(log)
    PACKAGE = Project.root / 'PACKAGE'
    # print(PACKAGE.absolute())
    targets = [
        "package-info.json",
        "package-info.json.in",
        "icon.png",
    ] + Project.package_dirs

    destination = PACKAGE / package_name
    cmd.makedirs(destination)
    for target in targets:
        p = Project.root / target
        if p.exists():
            if p.name in ['externals', 'support']:
                dst = destination / p.name
                cmd(f'ditto {p} {dst}')
            else:
                cmd.copy(p, destination)
    for f in Project.root.glob('*.md'):
        cmd.copy(f, PACKAGE)

    return PACKAGE


def package_as_dmg(package_name=Project.package_name):
    srcfolder = package(package_name)
    log = logging.getLogger('dmg_packager')
    cmd = ShellCmd(log)
    # name = package_name.replace('-','')
    # dmgname = Project.root / f"{name}-{Project.python.tag}"
    dmg = Project.root / f"{package_name}.dmg"
    if srcfolder.exists():
        cmd(f"hdiutil create -volname {package_name.upper()} " 
            f"-srcfolder {srcfolder} -ov -fs HFS+ "
            f"-format UDZO {dmg}"
        )
        assert dmgname.exists()
        cmd.remove(srcfolder)

    env_file = os.getenv('GITHUB_ENV')
    with open(env_file, "a") as fopen:
        fopen.write(f"PRODUCT_DMG={dmg}")



def sign_all():
    sign_folder('externals')
    sign_folder('support')


if __name__ == "__main__":
    CodesignExternal.cmdline()
