#!/usr/bin/env python3
"""depend: gather and resolve dylib depdencies."""
from types import SimpleNamespace
from textwrap import dedent
import sys
import os
import re
import shutil
import subprocess
import logging
from pathlib import Path
from typing import Union
from pprint import pprint
from termcolor import colored, cprint

DEBUG = False
LOG_LEVEL = logging.DEBUG if DEBUG else logging.INFO
LOG_FORMAT = "%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s"
URL_GETPIP = "https://bootstrap.pypa.io/get-pip.py"

logging.basicConfig(format=LOG_FORMAT, level=LOG_LEVEL)
title = lambda x: cprint(x, 'green', attrs=['bold'])

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
            shutil.rmtree(path, ignore_errors=not DEBUG)
        else:
            self.log.info("remove file: %s", path)
            path.unlink(missing_ok=True)

    def install_name_tool(self, src, dst, mode="id"):
        """change dynamic shared library install names"""
        _cmd = f"install_name_tool -{mode} {src} {dst}"
        self.log.info(_cmd)
        self.cmd(_cmd)


class DependencyManager:
    """Aggreggates, copies dylib dependencies and fixed references.

    target: dylib or executable to made relocatable
    dest_dir: where target dylib will be copied to with copied dependents
    exec_ref: back ref for executable or plugin
    """

    def __init__(self,
                 target: Union[str, Path],
                #  project: Project = None,
                 dest_dir: Path = None,
                 staticlibs_dir: Path = None,
                 exec_ref: str = None):
        self.target = Path(target)
        # self.project = project
        self.dest_dir = dest_dir or Path('build')
        self.staticlibs_dir = staticlibs_dir
        self.exec_ref = exec_ref or '@loader_path/../Frameworks'
        self.install_names = {}
        self.deps = []
        self.dep_list = []
        self.log = logging.getLogger(self.__class__.__name__)
        self.cmd = ShellCmd(self.log)

    def is_valid_path(self, dep_path):
        return (dep_path == '' 
                or dep_path.startswith('/opt/local/')
                or dep_path.startswith('/usr/local/')
                or dep_path.startswith('/Users/'))

    def is_executable(self):
        return (self.target.is_file()
                and os.access(self.target, os.X_OK)
                and self.target.suffix != '.dylib')

    def is_dylib(self):
        return (self.target.is_file()
                and self.target.suffix == '.dylib')

    def analyze(self):
        if self.is_executable():
            print("target is executable:", self.target)
            for ref in self.get_references():
                print(ref)
        elif self.is_dylib():
            print("target is dylib:", self.target)
        else:
            print("target is invalid:", self.target)

    def get_references(self):
        entries = []
        result = subprocess.check_output(["otool", "-L", self.target])
        lines = [line.decode("utf-8").strip()
                for line in result.splitlines()]
        for line in lines:
            match = re.match(
                r"\s*(\S+)\s*\(compatibility version .+\)$", line)
            if match:
                path = match.group(1)
                if self.is_valid_path(path):
                   entries.append(path)
        return entries

    # def fix_python_exec(self, dylib):
    #     """change ref on executable to point to relative dylib"""
    #     if not self.is_executable():
    #         return

    #     for path in self.get_references():
    #         if path.startswith("/usr/local/Cellar/python"):
    #             self.cmd.install_name_tool(
    #                 mode="change",
    #                 src=path,
    #                 dst=f"@executable_path/../{self.project.dylib} {self.target}",
    #             )

    def get_deps(self, target=None):
        if not target:
            target = self.target
        key = os.path.basename(target)
        self.install_names[key] = []
        result = subprocess.check_output(['otool', '-L', target])
        entries = [
            line.decode('utf-8').strip() for line in result.splitlines()
        ]
        for entry in entries:
            match = re.match(r'\s*(\S+)\s*\(compatibility version .+\)$',
                             entry)
            if match:
                path = match.group(1)
                (dep_path, dep_filename) = os.path.split(path)
                if self.is_valid_path(dep_path):
                    if dep_path == '':
                        path = os.path.join('/usr/local/lib', dep_filename)

                    dep_path, dep_filename = os.path.split(path)
                    item = (path, '@rpath/' + dep_filename)
                    self.install_names[key].append(item)
                    if path not in self.deps:
                        self.deps.append(path)
                        self.get_deps(path)

    def process_deps(self):
        for dep in self.deps:
            _, dep_filename = os.path.split(dep)
            # dep_path, dep_filename = os.path.split(dep)
            # dest = os.path.join(self.dest_dir, dep_filename)
            self.dep_list.append([dep, '@rpath/' + dep_filename])

    def copy_dylibs(self):
        if not os.path.exists(self.dest_dir):
            os.mkdir(self.dest_dir)

        # cp target to dest_dir
        if os.path.dirname(self.target) != self.dest_dir:
            dest = self.dest_dir / os.path.basename(self.target)
            shutil.copyfile(self.target, dest)
            os.chmod(dest, 0o644)
            cmdline = ['install_name_tool', '-id', self.exec_ref, dest]
            err = subprocess.call(cmdline)
            if err != 0:
                raise RuntimeError("Failed to change '{0}' '{1}'".format(
                    dest, self.exec_ref))

        # copy the rest
        for item in self.dep_list:
            # orig_path, transformed = item
            # dirname, dylib = os.path.split(orig_path)
            orig_path, _ = item
            _, dylib = os.path.split(orig_path)

            dest = os.path.join(self.dest_dir, dylib)

            if not os.path.exists(dest):
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
                    cmdline = ['install_name_tool', '-id', new, target]
                else:
                    cmdline = [
                        'install_name_tool', '-change', old, new, target
                    ]

                err = subprocess.call(cmdline)
                if err != 0:
                    raise RuntimeError(
                        "Failed to change '{0}' to '{1}' in '{2}".format(
                            old, new, target))

    def transform_exec(self, target):
        result = subprocess.check_output(['otool', '-L', target])
        entries = [
            line.decode('utf-8').strip() for line in result.splitlines()
        ]
        for entry in entries:
            match = re.match(r'\s*(\S+)\s*\(compatibility version .+\)$',
                             entry)
            if match:
                path = match.group(1)
                (dep_path, dep_filename) = os.path.split(path)
                if self.is_valid_path(dep_path):
                    if dep_path == '':
                        path = os.path.join('/usr/local/lib', dep_filename)

                    dep_path, dep_filename = os.path.split(path)

                    dest = os.path.join(self.exec_ref, dep_filename)
                    cmdline = [
                        'install_name_tool', '-change', path, dest, target
                    ]
                    subprocess.call(cmdline)

    def copy_staticlibs(self):
        if not self.staticlibs_dir:
            raise Exception("must set 'staticlibs_dir parameter")
        for i in self.deps:
            head, tail = os.path.split(i)
            name = tail.rstrip('.dylib')
            if '.' in name:
                name = os.path.splitext(name)[0] + '.a'
            static = os.path.join(head, name)
            exists = os.path.exists(static)
            if exists:
                shutil.copyfile(static, os.path.join(self.staticlibs_dir,
                                                     name))
            else:
                print("revise: not exists", static)

    def process(self):
        self.get_deps()
        self.process_deps()
        self.copy_staticlibs()
        self.copy_dylibs()
        self.change_install_names()
        self.transform_exec('./eg')

if __name__ == '__main__':
    try:
        path = sys.argv[1]
        dm = DependencyManager(path)
        title("dm.analyze:")
        dm.analyze()
        title("dm.get_deps:")
        dm.get_deps()
        pprint(dm.install_names)

    except IndexError:
        print("ERROR: depend")
