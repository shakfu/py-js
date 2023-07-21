#!/usr/bin/env python3

"""populate.py

populates the site-packages of an external

"""
import os
import shutil
import sysconfig
from pathlib import Path


class PackageInstaller:
    PACKAGES = []
    INCLUDES = []
    EXCLUDES = []
    PREFIX_PATTERNS = [
        "_distutils",
        "_virtualenv",
        "distutils",
        "pip",
        "pkg_resources",
        "setuptools",
        "wheel",   
    ]

    SUFFIX_PATTERNS = [
        ".dist-info",
    ]

    def __init__(self, destination_dir, working_dir='.', **options):
        self.destination_dir = destination_dir
        self.working_dir = Path(working_dir)
        self.venv = options.get("venv", "pyenv")
        self.packages = options.get("packages", PACKAGES)
        self.includes = options.get("includes", INCLUDES)
        self.excludes = options.get("excludes", EXCLUDES)
        self.requirements_txt = options.get("requirements_txt")
        self.py_ver = sysconfig.get_config_var("py_version_short")
        self.venv_dir = self.working_dir / self.venv
        self.site_packages = self.venv_dir / 'lib' / f'python{self.py_ver}' / 'site-packages'


    def cmd(self, shellcmd, *args, **kwds):
        os.system(shellcmd.format(*args, **kwds))

    def vcmds(self, shellcmds, *args, **kwds):
        shellcmd = " && ".join(shellcmds)
        os.system(shellcmd)

    def remove(self, target):
        assert os.path.exists(target)
        if os.path.isfile(target):
            os.remove(target)
            print('removed file:', target)
        else:
            shutil.rmtree(target)
            print('removed dir:', target)

    def cleanup(self):
        for i in self.site_packages.iterdir():
            if any(i.startswith(p) for p in PREFIX_PATTERNS):
                self.remove(i)
            if any(i.endswith(p) for p in SUFFIX_PATTERNS):
                self.remove(i)

    def build(self):
        self.vcmds(
            [
                f"virtualenv {self.venv}",
                f"source {self.venv}/bin/activate",
                "pip install {}".format(" ".join(self.packages)),
            ]
        )
        self.cleanup()
        # copy to external site-packages


if __name__ == "__main__":
    # checks()
    builder = PackageInstaller()
    builder.build()
