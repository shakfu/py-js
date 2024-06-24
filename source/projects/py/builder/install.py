"""builder.install

installs python packages into builder python3 externals

"""
import logging
import os
import subprocess
from fnmatch import fnmatch
from pathlib import Path
from typing import Callable, Optional, Union

from .config import Project, LOG_FORMAT, LOG_LEVEL
from .shell import ShellCmd


Pathlike = Union[str, Path]
MatchFn = Callable[[Path], bool]
ActionFn = Callable[[Path], None]

logging.basicConfig(format=LOG_FORMAT, level=LOG_LEVEL)

class Installer(ShellCmd):
    def cmd(self, shellcmd: str, cwd: Pathlike = "."):
        """Run shell command within working directory"""
        self.log.info(shellcmd)
        try:
            subprocess.check_call(shellcmd, shell=True, cwd=str(cwd))
        except subprocess.CalledProcessError:
            self.log.critical("", exc_info=True)
            sys.exit(1)

    def git_clone(
        self,
        url: str,
        branch: Optional[str] = None,
        directory: Optional[str] = None,
        recurse: bool = False,
        cwd: Pathlike = ".",
    ):
        """git clone a repository source tree from a url"""
        _cmds = ["git clone --depth 1"]
        if branch:
            _cmds.append(f"--branch {branch}")
        if recurse:
            _cmds.append("--recurse-submodules --shallow-submodules")
        _cmds.append(url)
        if directory:
            _cmds.append(str(directory))
        self.cmd(" ".join(_cmds), cwd=cwd)

    def walk(
        self,
        root: Pathlike,
        match_func: MatchFn,
        action_func: ActionFn,
        skip_patterns: list[str],
    ):
        """general recursive walk from root path with match and action functions"""
        for root_, dirs, filenames in os.walk(root):
            _root = Path(root_)
            if skip_patterns:
                for skip_pat in skip_patterns:
                    if skip_pat in dirs:
                        dirs.remove(skip_pat)
            for _dir in dirs:
                current = _root / _dir
                if match_func(current):
                    action_func(current)

            for _file in filenames:
                current = _root / _file
                if match_func(current):
                    action_func(current)

    def glob_remove(self, root: Pathlike, patterns: list[str], skip_dirs: list[str]):
        """applies recursive glob remove using a list of patterns"""

        def match(entry: Path) -> bool:
            # return any(fnmatch(entry, p) for p in patterns)
            return any(fnmatch(entry.name, p) for p in patterns)

        def remove(entry: Path):
            self.remove(entry)

        self.walk(root, match_func=match, action_func=remove,
                  skip_patterns=skip_dirs)


def install_numpy(version='2.0.0'):
    np_ver = int(version[0])
    p = Project()
    p.shell = Installer(log=logging.getLogger('Installer'))
    py_ver = p.python.version_short
    src_np = p.build_src / 'numpy'
    if src_np.exists():
        p.shell.remove(src_np)
    p.shell.git_clone("https://github.com/numpy/numpy.git", recurse=True, branch=f'v{version}', cwd=p.build_src)
    src_np = p.build_src / 'numpy'
    venv = src_np / 'venv'
    p.shell.cmd('virtualenv venv', cwd=src_np)
    os.system(f'cd {src_np} && source venv/bin/activate && pip install .')
    venv_np = venv / 'lib' / f'python{py_ver}' / 'site-packages' / 'numpy'
    p.shell.glob_remove(venv_np, patterns=["tests"], skip_dirs=[".git"])
    p.shell.glob_remove(venv_np, patterns=[
        "__pycache__",
        "*.pyi",
        "*.pxd",
        "*.pyx",
    ], skip_dirs=[".git"])
    subdirs = [
        "random/lib",
        "random/_examples",
        "f2py",
        "_pyinstaller",
        "testing",
        "typing",
    ]
    if np_ver >= 2:
        subdirs.extend([
            "_core/include",
            "_core/lib",
        ])
    else:
        subdirs.extend([
            "array_api",
            "core/include",
            "core/lib",
        ])        
    for subdir in subdirs:
        target_dir = venv_np / subdir
        p.shell.remove(target_dir)

    p.shell.copy(venv_np, p.build_lib / 'numpy')
