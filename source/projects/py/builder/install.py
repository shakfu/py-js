"""builder.install

installs python packages into builder python3 externals

"""

import logging
import os
from pathlib import Path

from typing import Optional

from .config import Project, LOG_FORMAT, LOG_LEVEL, DEFAULT_NP_VERSION
from .shell import ShellCmd

logging.basicConfig(format=LOG_FORMAT, level=LOG_LEVEL)

dirs = {
    'homebrew-pkg':    "{support}/Python.framework/Versions/{ver}/lib/python{ver}/site-packages",
    'homebrew-ext':    "{externals}/py.mxo/Contents/Resources/python{ver}/lib/python{ver}/site-packages",
    'shared-pkg':      "{support}/python{ver}/lib/python{ver}/site-packages",
    'shared-ext':      "{externals}/py.mxo/Contents/Resources/lib/python{ver}/site-packages",
    'shared-tiny-ext': "{externals}/py.mxo/Contents/Resources/lib/python{ver}/site-packages",
    'static-ext':      "{externals}/py.mxo/Contents/Resources/lib/python{ver}/site-packages",
    'static-tiny-ext': "{externals}/py.mxo/Contents/Resources/lib/python{ver}/site-packages",
    'framework-pkg':   "{support}/Python.framework/Versions/{ver}/lib/python{ver}/site-packages",
    'framework-ext':   "{support}/Python.framework/Versions/{ver}/lib/python{ver}/site-packages",
    'relocatable-pkg': "{support}/Python.framework/Versions/{ver}/lib/python{ver}/site-packages",
}


class Installer:
    def __init__(self, variant: Optional[str] = None, *pkgs):
        self.project = Project()
        self.variant = variant if variant else self.project.cache_get('variant')
        self.pkgs = pkgs
        self.log = logging.getLogger(self.__class__.__name__)
        self.cmd = ShellCmd(self.log)

    @property
    def site_packages(self):
        dikt = dict(
            support = self.project.support,
            externals = self.project.externals,
            ver = self.project.python.version_short,
        )
        return Path(dirs[self.variant].format(**dikt))

    def install_numpy(self, version=DEFAULT_NP_VERSION):
        np_ver = int(version[0])
        py_ver = self.project.python.version_short
        src = self.project.build_src
        src_np = src / "numpy"
        venv = src_np / "venv"
        venv_np = venv / "lib" / f"python{py_ver}" / "site-packages" / "numpy"
        installed_np =  self.project.build_lib / "numpy"
        site_np = self.site_packages / 'numpy'

        if site_np.exists():
            return

        if not installed_np.exists():
            if src_np.exists():
                self.cmd.remove(src_np)
            self.cmd.git_clone(
                "https://github.com/numpy/numpy.git",
                recurse=True,
                branch=f"v{version}",
                cwd=src,
            )
            self.cmd.cmd("virtualenv venv", cwd=src_np)
            os.system(f"cd {src_np} && source venv/bin/activate && pip install .")
            self.cmd.glob_remove(venv_np, patterns=["tests"], skip_dirs=[".git"])
            self.cmd.glob_remove(
                venv_np,
                patterns=[
                    "__pycache__",
                    "*.pyi",
                    "*.pxd",
                    "*.pyx",
                ],
                skip_dirs=[".git"],
            )
            subdirs = [
                "random/lib",
                "random/_examples",
                "f2py",
                "_pyinstaller",
                "testing",
                "typing",
            ]
            if np_ver >= 2:
                subdirs.extend(
                    [
                        "_core/include",
                        "_core/lib",
                    ]
                )
            else:
                subdirs.extend(
                    [
                        "array_api",
                        "core/include",
                        "core/lib",
                    ]
                )
            for subdir in subdirs:
                target_dir = venv_np / subdir
                self.cmd.remove(target_dir)

            self.cmd.copy(venv_np, installed_np)
        assert installed_np.exists()
        self.cmd.copy(installed_np, site_np)
