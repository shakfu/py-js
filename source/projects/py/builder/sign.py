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
    - convert folder into .dmg

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
import re
import subprocess


from .config import Project
from .shell import ShellCmd


DEBUG = True

logging.basicConfig(
    format="%(asctime)s - %(levelname)s - %(message)s",
    datefmt="%H:%M:%S",
    level=logging.DEBUG if DEBUG else logging.INFO,
)


class CodesignExternal:
    """Recursively codesign an external."""

    FILE_PATTERNS = {
        Project.python.version_short: "runtime",
        Project.python.name: "runtime",
        "python3": "runtime",
    }
    FILE_EXTENSIONS = [".so", ".dylib"]
    FOLDER_EXTENSIONS = [".mxo", ".framework", ".app", ".bundle"]

    def __init__(
        self,
        path: str,
        dev_id: str = None,
        entitlements: str = None,
        dry_run: bool = False,
    ):
        self.path = path
        self.authority = f"Developer ID Application: {dev_id}"
        self.entitlements = entitlements
        self.dry_run = dry_run
        self.targets_runtimes = set()
        self.targets_internals = set()
        self.targets_frameworks = set()
        self.targets_apps = set()
        self.log = logging.getLogger(self.__class__.__name__)
        # self.cmd = ShellCmd(self.log)
        self._cmd_codesign = [
            "codesign",
            "--sign",
            repr(self.authority),
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
            self.log.debug(" ".join(["DONE"] + arglist))

    def is_binary(self, path):
        """returns True if file is a binary file."""
        txt = self.cmd_check(["file", "-b", str(path)])
        if txt:
            return "binary" in txt.split()
        else:
            return False

    def verify(self, path):
        """verifies codesign of path"""
        self.cmd(f"codesign --verify --verbose {path}")

    def section(self, *args):
        """display section"""
        print()
        print("-" * 79)
        print(*args)

    def collect(self):
        """build up a list of target binaries"""
        for root, folders, files in os.walk(self.path):
            for fname in files:
                path = pathlib.Path(root) / fname
                for pattern in self.FILE_PATTERNS:
                    if fname == pattern:
                        if self.FILE_PATTERNS[fname] == "runtime":
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
                        if path.suffix == ".framework":
                            self.targets_frameworks.add(path)
                        elif path.suffix == ".app":
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
        codesign_runtime = " ".join(
            self._cmd_codesign
            + [
                "--options",
                "runtime",
                "--entitlements",
                str(self.entitlements),
                str(path),
            ]
        )
        self.cmd(codesign_runtime)
        # self.cmd_check(self._cmd_codesign + [
        #      "--options", "runtime",
        #      "--entitlements", str(self.entitlements),
        #      str(self.path)
        # ])

    def process(self):
        """main process to recursive sign."""

        self.section("PROCESSING:", self.path)

        self.section("COLLECTING...")
        if not self.targets_internals:
            self.collect()

        self.section("SIGNING INTERNAL TARGETS")
        for path in self.targets_internals:
            self.sign_internal_binary(path)

        self.section("SIGNING APPS")
        for path in self.targets_apps:
            macos_path = path / "Contents" / "MacOS"
            for exe in macos_path.iterdir():
                self.sign_internal_binary(exe)
            self.sign_runtime(path)

        self.section("SIGNING OTHER RUNTIMES")
        for path in self.targets_runtimes:
            self.sign_runtime(path)

        self.section("SIGNING FRAMEWORKS")
        for path in self.targets_frameworks:
            self.sign_internal_binary(path)

        self.section("SIGNING MAIN RUNTIME")
        self.sign_runtime()

        self.section("VERIFYING SIGNATURE")
        self.verify(self.path)

        print()
        self.log.info("DONE!")

    def process_dry_run(self):
        """main process to recursive sign."""
        def right(x):
            return str(x).lstrip(str(self.path))

        self.section("PROCESSING:", self.path)

        self.section("COLLECTING...")
        if not self.targets_internals:
            self.collect()

        self.section("SIGNING INTERNAL TARGETS")
        for path in self.targets_internals:
            print("internal target:", right(path))

        self.section("SIGNING APPS")
        for path in self.targets_apps:
            print("APP:", right(path))
            macos_path = path / "Contents" / "MacOS"
            for exe in macos_path.iterdir():
                print("app.internal_target:", right(exe))
            print("sign app.runtime:", right(path))

        self.section("SIGNING OTHER RUNTIMES")
        for path in self.targets_runtimes:
            print("sign other.runtime:", right(path))

        self.section("SIGNING FRAMEWORKS")
        for path in self.targets_frameworks:
            print("sign framework:", right(path))

        self.section("SIGNING MAIN")
        # sign main runtime
        print("sign main.runtime:", self.path)
        self.log.info("DONE!")

    @classmethod
    def cmdline(cls):
        """commandline interface to class."""
        parser = argparse.ArgumentParser(description=cls.__doc__)
        option = parser.add_argument
        option("path", type=str, help="a folder containing binaries to sign")
        option("--dev-id", "-i", help="Developer ID")
        option(
            "--entitlements",
            "-e",
            default="entitlements.plist",
            help="path to entitlements.plist",
        )
        option(
            "--dry-run", "-d", action="store_true", help="run without actual changes."
        )
        args = parser.parse_args()
        if args.path:
            app = cls(args.path, args.dev_id, args.entitlements, args.dry_run)
            if args.dry_run:
                app.process_dry_run()
            else:
                app.process()


def match_suffix(target):
    return target.suffix in CodesignExternal.FOLDER_EXTENSIONS


def match_python_shared(target):
    """FIXME: shared-pkg does not notarize!!"""
    match = re.match(r"python3.\d{1,2}", target.name)
    if match:
        return match.group(0) == target.name
    else:
        return False


def sign_folder(folder="externals", dry_run=False):
    matchers = [match_suffix, match_python_shared]
    dev_id = os.environ["DEV_ID"]
    assert dev_id, "environment var DEV_ID not set"
    root = pathlib.Path(__file__).parent.parent.parent.parent.parent
    target_folder = pathlib.Path(root / folder)
    entitlements = pathlib.Path(
        root / "source/py/resources/entitlements/entitlements.plist"
    )
    assert target_folder.exists(), f"not found: {target_folder}"
    assert entitlements.exists(), f"not found: {entitlements}"
    targets = list(target_folder.iterdir())
    assert len(targets) > 0, "no targets to sign"
    for target in targets:
        if any(match(target) for match in matchers):
            # if target.suffix in CodesignExternal.FOLDER_EXTENSIONS:
            signer = CodesignExternal(
                target, dev_id=dev_id, entitlements=entitlements, dry_run=dry_run
            )
            if signer.dry_run:
                signer.process_dry_run()
            else:
                signer.process()


def package(package_name=Project.package_name):
    log = logging.getLogger("packager")
    cmd = ShellCmd(log)
    PACKAGE = Project.root / "PACKAGE"
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
            if p.name in ["externals", "support"]:
                dst = destination / p.name
                cmd(f"ditto {p} {dst}")
            else:
                cmd.copy(p, destination)
    for f in Project.root.glob("*.md"):
        cmd.copy(f, PACKAGE)

    return PACKAGE


def package_as_dmg(package_name=Project.package_name):
    srcfolder = package(package_name)
    log = logging.getLogger("dmg_packager")
    cmd = ShellCmd(log)
    # name = package_name.replace('-','')
    # dmgname = Project.root / f"{name}-{Project.python.tag}"
    dmg = Project.root / f"{package_name}.dmg"
    if srcfolder.exists():
        cmd(
            f"hdiutil create -volname {package_name.upper()} "
            f"-srcfolder {srcfolder} -ov "
            f"-format UDZO {dmg}"
        )
        assert dmg.exists()
        cmd.remove(srcfolder)

    env_file = os.getenv("GITHUB_ENV")
    if env_file:
        with open(env_file, "a") as fopen:
            fopen.write(f"PRODUCT_DMG={dmg}")


def sign_dmg(dmg=None):
    log = logging.getLogger("dmg_packager")
    cmd = ShellCmd(log)
    dev_id = os.environ["DEV_ID"]
    product_dmg = os.getenv("PRODUCT_DMG")
    if dmg:
        product_dmg = dmg
    else:
        product_dmg = Project.dmg
    assert product_dmg, "PRODUCT_DMG or dmg path not set"
    assert dev_id, "environment var DEV_ID not set"
    cmd(
        f'codesign --sign "Developer ID Application: {dev_id}" '
        f"--deep --force --verbose --options runtime {product_dmg}"
    )


def notarize_dmg():
    """
    xcrun altool --notarize-app \
        --file $1 \
        -t osx \
        -u "${APPLE_ID}" \
        -p "${APP_PASS}" \
        -primary-bundle-id "${BUNDLE_ID}"
    """


def sign_all(dry_run=False):
    sign_folder("externals", dry_run)
    sign_folder("support", dry_run)


if __name__ == "__main__":
    CodesignExternal.cmdline()
