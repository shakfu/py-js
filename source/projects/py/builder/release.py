#!/usr/bin/env python3
"""release.py

Contains two classes:

    ReleaseManager
    CodesignExternal

## Release Manager



## CodesignExternal

Is a utility class which recursively walks through a bundle or folder structure
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


## TODO

New Workflow to implement:

VARIANT="shared-ext"

export DEV_ID="<first_name> <last_name>"

make ${VARIANT}

make sign

make dmg PKG_NAME="${VARIANT}"

export PRODUCT_DMG="<absolute-path-to-dmg>"

make sign-dmg ${PRODUCT_DMG}

xcrun notarytool submit ${PRODUCT_DMG} --keychain-profile "<keychain_profile>"

xcrun stapler staple ${PRODUCT_DMG}

mv ${PRODUCT_DMG} ~/Downloads/pyjs-builds


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
    format="%(asctime)s - %(levelname)s - %(funcName)s - %(message)s",
    datefmt="%H:%M:%S",
    level=logging.DEBUG if DEBUG else logging.INFO,
)


def match_suffix(target):
    return target.suffix in CodesignExternal.FOLDER_EXTENSIONS


def match_python_shared(target):
    """FIXME: shared-pkg does not notarize!!"""
    match = re.match(r"python3.\d{1,2}", target.name)
    if match:
        return match.group(0) == target.name
    return False


class ReleaseManager:
    """ Manages and executes the entire release process.
    """

    ENTITLEMENTS_PATH = "source/projects/py/resources/entitlements/entitlements.plist"

    def __init__(self, variant, dev_id=None, keychain_profile=None, dry_run=False):
        self.variant = variant
        self.dev_id = dev_id or os.environ['DEV_ID']
        self.keychain_profile = keychain_profile or os.environ['KEYCHAIN_PROFILE']
        self.dry_run = dry_run
        self.product_dmg = None
        self.project = Project()
        self.log = logging.getLogger(__class__.__name__)
        self.cmd = ShellCmd(self.log)

    @property
    def package_name(self):
        return self.project.get_package_name(self.variant)

    def process(self):
        self.sign_all()
        self.package_as_dmg()
        self.sign_dmg()
        self.notarize_dmg()
        self.staple_dmg()

    def sign_all(self):
        self.sign_folder(self.project.externals)
        self.sign_folder(self.project.support)

    def sign_folder(self, folder):
        matchers = [match_suffix, match_python_shared]
        root = pathlib.Path(__file__).parent.parent.parent.parent.parent
        target_folder = root / folder
        entitlements = root / self.ENTITLEMENTS_PATH
        assert target_folder.exists(), f"not found: {target_folder}"
        assert entitlements.exists(), f"not found: {entitlements}"
        self.log.info("target_folder: %s", target_folder)
        targets = list(target_folder.iterdir())
        assert len(targets) > 0, "no targets to sign"
        for target in targets:
            if any(match(target) for match in matchers):
                # if target.suffix in CodesignExternal.FOLDER_EXTENSIONS:
                signer = CodesignExternal(
                    target, dev_id=self.dev_id, entitlements=entitlements, dry_run=self.dry_run
                )
                if signer.dry_run:
                    signer.process_dry_run()
                else:
                    signer.process()

    def package_as_dmg(self):
        srcfolder = self.package()
        dmg = self.project.root / f"{self.package_name}.dmg"
        assert srcfolder.exists(), f"{srcfolder} does not exist"
        self.cmd(
            f"hdiutil create -volname {self.project.name.upper()} "
            f"-srcfolder {srcfolder} -ov "
            f"-format UDZO {dmg}"
        )
        assert dmg.exists(), f"{dmg} does not exist"
        self.cmd.remove(srcfolder)
        self.product_dmg = dmg
        env_file = os.getenv("GITHUB_ENV")
        if env_file:
            with open(env_file, "a") as fopen:
                fopen.write(f"PRODUCT_DMG={dmg}")

    def package(self):
        PACKAGE = self.project.root / "PACKAGE"
        targets = [
            "package-info.json",
            "package-info.json.in",
            "icon.png",
        ] + self.project.package_dirs

        destination = PACKAGE / self.project.name
        self.cmd.makedirs(destination)
        for target in targets:
            p = self.project.root / target
            if p.exists():
                if p.name in ["externals", "support"]:
                    dst = destination / p.name
                    self.cmd(f"ditto {p} {dst}")
                else:
                    self.cmd.copy(p, destination)
        for f in self.project.root.glob("*.md"):
            self.cmd.copy(f, PACKAGE)

        return PACKAGE

    def sign_dmg(self):
        self.cmd(
            f'codesign --sign "Developer ID Application: {self.dev_id}" '
            f'--deep --force --verbose --options runtime "{self.product_dmg}"'
        )


    def notarize_dmg_old(self):
        """
        xcrun altool --notarize-app \
            --file $1 \
            -t osx \
            -u "${APPLE_ID}" \
            -p "${APP_PASS}" \
            -primary-bundle-id "${BUNDLE_ID}"
        """

    def notarize_dmg(self):
        """notarize .dmg using notarytool"""
        self.cmd(
            f'xcrun notarytool submit "{self.product_dmg}" --keychain-profile "{self.keychain_profile}"'
        )

    def staple_dmg(self):
        """staple .dmg using notarytool"""
        self.cmd(
            f'xcrun stapler staple "{self.product_dmg}"'
        )

    @classmethod
    def cmdline(cls):
        """commandline interface to class."""
        parser = argparse.ArgumentParser(description=cls.__doc__)
        option = parser.add_argument
        option("variant", type=str, help="a folder containing binaries to sign")
        option("-i", "--dev-id", help="Developer ID")
        option("-k", "--keychain-profile", help="Keychain Profile")
        option("-d", "--dry-run",action="store_true", help="run without actual changes.")
        args = parser.parse_args()
        app = cls(args.variant, args.dev_id, args.keychain_profile, args.dry_run)
        app.process()



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
        return res

    def is_binary(self, path):
        """returns True if file is a binary file."""
        txt = self.cmd_check(["file", "-b", str(path)])
        if txt:
            return "binary" in txt.split()
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
                for _ in self.FILE_EXTENSIONS:
                    if path.suffix not in self.FILE_EXTENSIONS:
                        continue
                    if path.is_symlink():
                        continue
                    if path.suffix in self.FILE_EXTENSIONS:
                        self.log.debug("added binary: %s", path)
                        self.targets_internals.add(path)
            for folder in folders:
                path = pathlib.Path(root) / folder
                for _ in self.FOLDER_EXTENSIONS:
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





if __name__ == "__main__":
    # CodesignExternal.cmdline()
    ReleaseManager.cmdline()