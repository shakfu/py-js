#!/usr/bin/env python3
"""fixer: gather and resolve dylib dependencies.

class Fixer
    Takes
        a path to framework or a shared library, 
        a flag to specify external or package
qqq                 
    init(
        target: Pathlike, 
        dest_dir: Pathlike = None, 
        staticlibs_dir: Pathlike = None,
        exec_ref: str = None
    )

    def fix_dylib_for_shared_pkg(dylib):
        install to dylib @rpath of @loader' to dylib in a shared-pkg

    def fix_dylib_for_framework_pkg(dylib):
        install to dylib @rpath of @loader' to dylib in a framework-pkg

    def fix_dylib_for_framework_ext(dylib):
        install to dylib @rpath of @loader' to dylib in a framework-ext

    def fix_dylib_for_shared_ext(dylib):
        install to dylib @rpath of @loader' to dylib in a shared-ext

    def fix_exe_for_shared_pkg(executable):
        redirect ref of pythonX to libpythonX.Y.dylib

    def fix_execs_for_framework_ext_or_pkg(py_exec, app_exec):
        redirect ref of pythonX to libpythonX.Y.dylib

    def path_is_valid_path(path) -> bool

    def target_is_executable() -> bool

    def target_is_dylib() -> bool

    def analyze_target()

    def analyze_executable() -> references

    def get_target_references() -> references

    def get_target_deps(target=None) -> references

    def process_target_deps()

    def copy_target_dylibs()

    def change_target_install_names()

    def transform_target_exec(target)

    def copy_staticlibs()

    def process()


def ensure_current_version_link(framework_path, short_version) -> bool
    Make sure the framework has Versions/Current


def relativize_interpreter_path(framework_path, script_dir, shebang_line) -> path
    Takes a shebang line and generates a relative path to the interpreter
    from the script dir. This is complicated by the fact the shebang line
    might start with the current framework_path _or_
    the default framework path

def is_framework_shebang(framework_path, text) -> bool
    Returns a boolean to indicate if the text starts with a shebang
    referencing the framework_path or the default
    /Library/Frameworks/Python.framework path

def fix_script_shebangs(framework_path, short_version)
    Attempt to make the scripts in the bin directory relocatable

def fix_other_things(framework_path, short_version)
    Wrapper function in case there are other things we need to fix in the
    future

def fix_broken_signatures(files_relocatablized)
    Re-sign the binaries and libraries that were relocatablized with ad-hoc
    signatures to avoid them having invalid signatures and to allow them to
    run on Apple Silicon
    

"""
import logging
import os
import re
import shutil
import subprocess
import sys
import sysconfig
from pathlib import Path
from pprint import pprint
from typing import Optional, Union
from enum import Enum

# from .config import LOG_FORMAT, LOG_LEVEL
# from .shell import ShellCmd

DEBUG = True
LOG_LEVEL = logging.DEBUG if DEBUG else logging.INFO
LOG_FORMAT = "%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s"

Pathlike = Union[str, Path]


PATTERNS_TO_FIX = [
    "/usr/local",
    "/opt/local" "/Library/Frameworks/Python.framework",
    "/tmp/",
    "/Users/",
]

logging.basicConfig(format=LOG_FORMAT, level=LOG_LEVEL)
title = lambda x: print(x)

# ----------------------------------------------------------------------------
# Utility Classes

class ShellCmd:
    """Provides platform agnostic file/folder handling."""

    def __init__(self, log):
        self.log = log

    def cmd(self, shellcmd, *args, **kwargs):
        """Run shell command with args and keywords"""
        _cmd = shellcmd.format(*args, **kwargs)
        self.log.info(_cmd)
        os.system(_cmd)

    __call__ = cmd

    def chdir(self, path):
        """Change current workding directory to path"""
        self.log.info("changing working dir to: %s", path)
        os.chdir(path)

    def chmod(self, path, perm=0o777):
        """Change permission of file"""
        self.log.info("change permission of %s to %s", path, perm)
        os.chmod(path, perm)

    def makedirs(self, path, mode=511, exist_ok=False):
        """Recursive directory creation function"""
        self.log.info("making directory: %s", path)
        os.makedirs(path, mode, exist_ok)

    def move(self, src, dst):
        """Move from src path to dst path."""
        self.log.info("move path %s to %s", src, dst)
        shutil.move(src, dst)

    def copy(self, src: Path, dst: Path):
        """copy file or folders -- tries to be behave like `cp -rf`"""
        self.log.info("copy %s to %s", src, dst)
        src, dst = Path(src), Path(dst)
        if dst.exists():
            dst = dst / src.name
        if src.is_dir():
            shutil.copytree(src, dst)
        else:
            shutil.copy2(src, dst)

    def remove(self, path):
        """Remove file or folder."""
        if path.is_dir():
            self.log.info("remove folder: %s", path)
            shutil.rmtree(path, ignore_errors=(not DEBUG))
        else:
            self.log.info("remove file: %s", path)
            try:
                # path.unlink(missing_ok=True)
                path.unlink()
            except FileNotFoundError:
                self.log.warning("file not found: %s", path)

    def install_name_tool(self, src, dst, mode="id"):
        """change dynamic shared library install names"""
        _cmd = f"install_name_tool -{mode} {src} {dst}"
        self.log.info(_cmd)
        self.cmd(_cmd)

class ProductType(Enum):
    RELOCATABLE = 1
    EXTERNAL = 2
    PACKAGE = 3

class Fixer:
    """Aggreggates, copies dylib dependencies and fixed references.

    target: dylib or executable to made relocatable
    dest_dir: where target dylib will be copied to with copied dependents
    exec_ref: back ref for executable or plugin

    Takes
        a path to framework or a shared library, 
        a flag to specify external or package

    """

    def __init__(self, framework_path: Pathlike, 
                 product_type: ProductType = ProductType.RELOCATABLE):
        self.framework_path = Path(framework_path)
        self.short_version = self.get_short_version(self.framework_path)
        self.product_type = product_type
        # self.install_names_before = {}
        # self.install_names_after = {}
        self.log = logging.getLogger(self.__class__.__name__)
        self.cmd = ShellCmd(self.log)
        self.install_names = {}
        self.deps = []
        self.dep_list = []
        self.refs = {}
    #     self.dest_dir = Path(dest_dir) if dest_dir else Path("build")
    #     self.staticlibs_dir = staticlibs_dir
    #     self.exec_ref = exec_ref or "@loader_path/../Frameworks"

    @property
    def prefix(self):
        return self.framework_path / 'Versions' / self.short_version

    @property
    def prefix_bin(self):
        return self.prefix / 'bin'

    @property
    def prefix_lib(self):
        return self.prefix / 'lib'

    @property
    def python_lib(self):
        return self.prefix_lib / f'python{self.short_version}'

    @property
    def lib_dynload(self):
        return self.python_lib / 'lib-dynload'

    def get_short_version(self, framework_path: Path):
        """get short python version from framework"""
        versions = self.framework_path / 'Versions'
        dirs = list(d.name for d in versions.iterdir())
        short_version = None
        for d in dirs:
            m = re.match(r'\d\.\d+', d)
            if m:
                short_version = m.group(0)
        if short_version:
            if not 'Current' in dirs:
                ensure_current_version_link(self.framework_path, short_version)
            return short_version
        else:
            raise FileNotFoundError


    def install_name_tool_id(self, new_id, target):
        """change dynamic shared library install names"""
        _cmd = f"install_name_tool -id '{new_id}' '{target}'"
        self.cmd(_cmd)

    def install_name_tool_change(self, src, dst, target):
        """change dependency reference"""
        _cmd = f"install_name_tool -change '{src}' '{dst}' '{target}'"
        self.cmd(_cmd)

    def install_name_tool_add_rpath(self, rpath, target):
        """change dependency reference"""
        _cmd = f"install_name_tool -add_rpath '{rpath}' '{target}'"
        self.cmd(_cmd)


    def fix_dylib_for_shared_pkg(self, dylib):
        """install to dylib @rpath of @loader' to dylib in a shared-pkg"""
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            (
                "@loader_path/../../../../support/"
                f"{self.product.name_ver}/lib/{self.product.dylib}"
            ),
            dylib,
        )

    def fix_dylib_for_framework_pkg(self, dylib):
        """install to dylib @rpath of @loader' to dylib in a framework-pkg"""
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            f"@loader_path/../../../../support/Python.framework/Versions/{self.product.ver}/Python",
            dylib,
        )

    def fix_dylib_for_framework_ext(self, dylib):
        """install to dylib @rpath of @loader' to dylib in a framework-ext"""
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            f"@loader_path/../Resources/Python.framework/Versions/{self.product.ver}/Python",
            dylib,
        )

    def fix_dylib_for_shared_ext(self, dylib):
        """install to dylib @rpath of @loader' to dylib in a shared-ext"""
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            f"@loader_path/../Resources/lib/{self.product.dylib}", dylib
        )

    def fix_exe_for_shared_pkg(self, executable):
        """redirect ref of pythonX to libpythonX.Y.dylib"""
        dirs = DependencyManager(executable).analyze_executable()
        if dirs:
            dir_to_change = dirs[0]
            self.install_name_tool_change(
                dir_to_change,
                f"@executable_path/../lib/{self.product.dylib}",
                executable,
            )

    def fix_execs_for_framework_ext_or_pkg(self, py_exec, app_exec):
        """redirect ref of pythonX to libpythonX.Y.dylib"""
        fixes = [
            (py_exec, "@executable_path/../Python"),
            (app_exec, "@executable_path/../../../../Python"),
        ]
        for exe, backref in fixes:
            dirs = DependencyManager(exe).analyze_executable()
            if dirs:
                dir_to_change = dirs[0]
                self.install_name_tool_change(dir_to_change, backref, exe)

    def is_valid_path(self, dep_path: Pathlike) -> bool:
        return (
            dep_path == ""
            or dep_path.startswith("/opt/local/")
            or dep_path.startswith("/usr/local/")
            or dep_path.startswith("/Users/")
            or dep_path.startswith("/tmp/")
        )

    def target_is_executable(self, target: Pathlike) -> bool:
        target = Path(target)
        return (
            target.is_file()
            and os.access(target, os.X_OK)
            and target.suffix != ".dylib"
        )

    def target_is_dylib(self, target: Pathlike) -> bool:
        return target.is_file() and target.suffix == ".dylib"

    def analyze(self, target: Pathlike):
        target = Path(target)
        if self.target_is_executable(target):
            print("target is executable:", target)
            for ref in self.get_target_references(target):
                print(ref)
        elif self.target_is_dylib(target):
            print("target is dylib:", target)
        else:
            print("target is invalid:", target)

    def analyze_executable(self, exe: Pathlike):
        exe = Path(exe)
        assert self.target_is_executable(exe), "target is not an executable"
        return self.get_references(exe)

    def get_target_references(self, target: Pathlike):
        target = Path(target)
        entries = []
        result = subprocess.check_output(["otool", "-L", target])
        lines = [line.decode("utf-8").strip() for line in result.splitlines()]
        for line in lines:
            match = re.match(r"\s*(\S+)\s*\(compatibility version .+\)$", line)
            if match:
                path = match.group(1)
                if self.is_valid_path(path):
                    entries.append(path)
        return entries

    def get_deps(self, target: Pathlike = None):
        key = os.path.basename(target)
        self.install_names[key] = []
        result = subprocess.check_output(["otool", "-L", target], text=True)
        entries = [line.strip() for line in result.splitlines()]
        for entry in entries:
            match = re.match(r"\s*(\S+)\s*\(compatibility version .+\)$", entry)
            if match:
                path = match.group(1)
                (dep_path, dep_filename) = os.path.split(path)
                # print(dep_path, dep_filename)
                if self.is_valid_path(dep_path):
                    if dep_path == "":
                        path = os.path.join("/usr/local/lib", dep_filename)

                    dep_path, dep_filename = os.path.split(path)
                    item = (path, "@rpath/" + dep_filename)
                    self.install_names[key].append(item)
                    if path not in self.deps:
                        self.deps.append(path)
                        self.get_deps(path)

    def process_deps(self):
        for dep in self.deps:
            _, dep_filename = os.path.split(dep)
            # dep_path, dep_filename = os.path.split(dep)
            # dest = os.path.join(self.dest_dir, dep_filename)
            self.dep_list.append([dep, f"@rpath/{dep_filename}"])

    def copy_dylibs(self):
        # if not os.path.exists(self.dest_dir):
        #     os.mkdir(self.dest_dir)

        # cp target to dest_dir
        # if os.path.dirname(self.target) != self.prefix_lib:
        #     dest = self.prefix_lib / os.path.basename(self.target)
        #     shutil.copyfile(self.target, dest)
        #     os.chmod(dest, 0o644)
        #     cmdline = ["install_name_tool", "-id", self.exec_ref, dest]
        #     err = subprocess.call(cmdline)
        #     if err != 0:
        #         raise RuntimeError(
        #             "Failed to change '{0}' '{1}'".format(dest, self.exec_ref)
        #         )

        # copy the rest
        for item in self.dep_list:
            # orig_path, transformed = item
            # dirname, dylib = os.path.split(orig_path)
            orig_path, _ = item
            _, dylib = os.path.split(orig_path)

            # dest = os.path.join(self.dest_dir, dylib)
            dest = self.prefix_lib / dylib

            if not dest.exists():
                shutil.copyfile(orig_path, dest)
                os.chmod(dest, 0o644)

    def change_install_names(self):
        for key in sorted(self.install_names.keys()):
            # print(key)
            # for i in self.install_names[key]:
            #     print('\t', i)
            # print()

            target = os.path.join(self.dest_dir, key)
            deps = self.install_names[key]
            for dep in deps:
                old, new = dep

                # (old_name_path, old_name_filename) = os.path.split(old)
                _, old_name_filename = os.path.split(old)
                if key == old_name_filename:
                    cmdline = ["install_name_tool", "-id", new, target]
                else:
                    cmdline = ["install_name_tool", "-change", old, new, target]

                err = subprocess.call(cmdline)
                if err != 0:
                    raise RuntimeError(
                        "Failed to change '{0}' to '{1}' in '{2}".format(
                            old, new, target
                        )
                    )

    def transform_exec(self, target):
        result = subprocess.check_output(["otool", "-L", target])
        entries = [line.decode("utf-8").strip() for line in result.splitlines()]
        for entry in entries:
            match = re.match(r"\s*(\S+)\s*\(compatibility version .+\)$", entry)
            if match:
                path = match.group(1)
                (dep_path, dep_filename) = os.path.split(path)
                if self.is_valid_path(dep_path):
                    if dep_path == "":
                        path = os.path.join("/usr/local/lib", dep_filename)

                    dep_path, dep_filename = os.path.split(path)

                    dest = os.path.join(self.exec_ref, dep_filename)
                    cmdline = ["install_name_tool", "-change", path, dest, target]
                    subprocess.call(cmdline)

    def copy_staticlibs(self):
        if not self.staticlibs_dir:
            raise Exception("must set 'staticlibs_dir parameter")
        for i in self.deps:
            head, tail = os.path.split(i)
            name = tail.rstrip(".dylib")
            if "." in name:
                name = os.path.splitext(name)[0] + ".a"
            static = os.path.join(head, name)
            exists = os.path.exists(static)
            if exists:
                shutil.copyfile(static, os.path.join(self.staticlibs_dir, name))
            else:
                print("revise: not exists", static)

    def process(self):
        self.get_deps()
        self.process_deps()
        self.copy_staticlibs()
        self.copy_dylibs()
        self.change_install_names()
        self.transform_exec("./eg")

def ensure_current_version_link(framework_path, short_version):
    '''Make sure the framework has Versions/Current'''
    versions_current_path = os.path.join(framework_path, "Versions/Current")
    if not os.path.exists(versions_current_path):
        specific_version = os.path.join(
            framework_path, "Versions", short_version)
        if not os.path.exists(specific_version):
            print("Path %s doesn't exist!" % short_version, file=sys.stderr)
            return False
        try:
            print("Creating Versions/Current symlink...")
            os.symlink(short_version, versions_current_path)
        except OSError as err:
            print("Could not create Versions/Current symlink to %s: %s"
                  % (short_version, err), file=sys.stderr)
            return False
    return True


def relativize_interpreter_path(framework_path, script_dir, shebang_line):
    '''Takes a shebang line and generates a relative path to the interpreter
    from the script dir. This is complicated by the fact the shebang line
    might start with the current framework_path _or_
    the default framework path'''
    original_path = shebang_line[2:]
    current_framework_path = os.path.abspath(framework_path).encode("UTF-8")
    default_framework_path = b"/Library/Frameworks/Python.framework"
    # normalize the original path so it refers to the current framework path
    if original_path.startswith(default_framework_path):
        original_path = original_path.replace(
            default_framework_path, current_framework_path, 1)
    return os.path.relpath(
        original_path, os.path.abspath(script_dir).encode("UTF-8"))


def is_framework_shebang(framework_path, text):
    """Returns a boolean to indicate if the text starts with a shebang
    referencing the framework_path or the default
    /Library/Frameworks/Python.framework path"""
    this_framework_shebang = b"#!" + os.path.abspath(framework_path).encode("UTF-8")
    prefixes = [
        this_framework_shebang,
        b"#!/Library/Frameworks/Python.framework",
        b"#!/Library/Developer/CommandLineTools/usr/bin/python3",
        b"#!/Applications/Xcode.app/Contents/Developer/usr/bin/python3",
    ]
    return any(text.startswith(x) for x in prefixes)


def fix_script_shebangs(framework_path, short_version):
    '''Attempt to make the scripts in the bin directory relocatable'''

    relocatable_shebang = b"""#!/bin/sh
'''exec' "$(dirname "$0")/%s" "$0" "$@"
' '''
# the above calls the %s interpreter relative to the directory of this script
"""
    bin_dir = os.path.join(framework_path, "Versions", short_version, "bin")
    for filename in os.listdir(bin_dir):
        try:
            original_filepath = os.path.join(bin_dir, filename)
            if (os.path.islink(original_filepath) or
                    os.path.isdir(original_filepath)):
                # skip symlinks and directories
                continue
            with open(original_filepath, 'rb') as original_file:
                first_line = original_file.readline().strip()
                if is_framework_shebang(framework_path, first_line):
                    # we found a script that references an interpreter inside
                    # the framework
                    print("Modifying shebang for %s..." % original_filepath)
                    relative_interpreter_path = relativize_interpreter_path(
                        framework_path, bin_dir, first_line)
                    new_filepath = original_filepath + ".temp"
                    with open(new_filepath, 'wb') as new_file:
                        new_file.write(
                            relocatable_shebang
                            % (relative_interpreter_path,
                               relative_interpreter_path)
                        )
                        for line in original_file.readlines():
                            new_file.write(line)
                    # replace original with modified
                    shutil.copymode(original_filepath, new_filepath)
                    os.remove(original_filepath)
                    os.rename(new_filepath, original_filepath)
        except (IOError, OSError) as err:
            print("Could not fix shebang for %s: %s"
                  % (os.path.join(bin_dir, filename), err))
            return False
    return True


def fix_other_things(framework_path, short_version):
    '''Wrapper function in case there are other things we need to fix in the
    future'''
    return (ensure_current_version_link(framework_path, short_version) and
            fix_script_shebangs(framework_path, short_version))


def fix_broken_signatures(files_relocatablized):
    """
    Re-sign the binaries and libraries that were relocatablized with ad-hoc
    signatures to avoid them having invalid signatures and to allow them to
    run on Apple Silicon
    """
    CODESIGN_CMD = ["/usr/bin/codesign",
                    "-s", "-", "--deep", "--force",
                    "--preserve-metadata=identifier,entitlements,flags,runtime"]
    for pathname in files_relocatablized:
        print("Re-signing %s with ad-hoc signature..."
              % pathname)
        cmd = CODESIGN_CMD + [pathname]
        subprocess.check_call(cmd)


if __name__ == '__main__':
    framework_path = '/Users/sa/Downloads/projects/py-js/source/py/targets/build/lib/Python.framework'
    libs = '/Users/sa/Downloads/projects/py-js/source/py/targets/build/lib/Python.framework/Versions/3.10/lib/python3.10/lib-dynload'
    f = Fixer(framework_path)

