"""
Framework output for relocatabilizer:

install_name_tool -id @rpath/Versions/3.9/Python $PROJECTS/py-js/support/Python.framework/Versions/3.9/Python
install_name_tool -change $PROJECTS//py-js/source/py/targets/build/lib/Python.framework/Versions/3.9/Python @rpath/Versions/3.9/Python $PROJECTS//py-js/support/Python.framework/Versions/3.9/bin/python3.9
install_name_tool -add_rpath @executable_path/../../../ $PROJECTS/py-js/support/Python.framework/Versions/3.9/bin/python3.9

affected files:

{'/Users/sa/Downloads/projects/py-js/support/Python.framework/Versions/3.9/Python',
 '/Users/sa/Downloads/projects/py-js/support/Python.framework/Versions/3.9/bin/python3.9'}
"""
from .config import LOG_FORMAT, LOG_LEVEL
from .shell import ShellCmd
from .depend import DependencyManager
from pathlib import Path
import logging

logging.basicConfig(format=LOG_FORMAT, level=LOG_LEVEL)

class Fixer:
    """class to fix @rpath references"""

    def __init__(self, src, dst, type: str = '@loader_path'):
        self.src = Path(src)
        self.dst = Path(dst)
        self.log = logging.getLogger(self.__class__.__name__)
        self.cmd = ShellCmd(self.log)

    def src_is_dylib(self) -> bool:
        return self.src.suffix == '.dylib'

    def install_name_tool_id(self, new_id, target):
        """change dynamic shared library install names"""
        self.cmd(f"install_name_tool -id '{new_id}' '{target}'")

    def install_name_tool_change(self, src, dst, target):
        """change dependency reference"""
        self.cmd(f"install_name_tool -change '{src}' '{dst}' '{target}'")

    def install_name_tool_add_rpath(self, rpath, target):
        """add @rpath dependency reference"""
        self.cmd(f"install_name_tool -add_rpath '{rpath}' '{target}'")

    def fix_dylib_for_shared_pkg(self, dylib, short_ver, is_homebrew=False):
        """install to dylib @rpath of @loader' to dylib in a shared-pkg

        used in:
            PythonBuilder(Builder): dylib-pkg
            SharedPythonForPkgBuilder(SharedPythonBuilder): dylib-pkg
            HomebrewBuilder(PyJsBuilder): dylib-pkg
        """
        sep = "" if is_homebrew else "lib/"
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            ("@loader_path/../../../../support/"
            f"python{short_ver}/{sep}libpython{short_ver}.dylib"), 
            dylib
        )

    def fix_dylib_for_shared_ext(self, dylib, short_ver, is_homebrew=False):
        """install to dylib @rpath of @loader' to dylib in a shared-ext

        used in: 
            SharedPythonForExtBuilder(SharedPythonBuilder): dylib-ext (resources)
            HomebrewBuilder(PyJsBuilder): ext (resources)
        """
        sep = "" if is_homebrew else "lib/"
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            (f"@loader_path/../Resources/{sep}python{short_ver}/"
             f"libpython{short_ver}.dylib"),
            dylib
        )

    def fix_dylib_for_framework_pkg(self, dylib, short_ver):
        """install to dylib @rpath of @loader' to dylib in a framework-pkg

        used in:
            FrameworkBuilder
        """
        self.cmd.chmod(dylib)
        self.install_name_tool_id((
            "@loader_path/../../../../support/Python.framework/"
            f"Versions/{short_ver}/Python"), 
            dylib
        )
    
    def fix_dylib_for_framework_ext(self, dylib, short_ver):
        """install to dylib @rpath of @loader' to dylib in a framework-ext

        used in:
            FrameworkBuilderForExt: ext (resources)
        """
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            f"@loader_path/../Resources/Python.framework/Versions/{short_ver}/Python",
            dylib
        )

## -----------------------------------------------------------------------

    def fix_exe_for_shared_pkg(self, executable, short_ver, is_homebrew=False):
        # sourcery skip: use-named-expression
        """redirect ref of pythonX to libpythonX.Y.dylib"

        used in:
            SharedPythonForPkgBuilder(SharedPythonBuilder): exe-pkg
            HomebrewBuilder
        """
        sep = "" if is_homebrew else "lib/"
        dirs = DependencyManager(executable).analyze_executable()
        if dirs:
            dir_to_change = dirs[0]
            self.install_name_tool_change(
                dir_to_change,
                f"@executable_path/../{sep}libpython{short_ver}.dylib",
                executable
            )

    def fix_execs_for_framework_ext_or_pkg(self, py_exec, app_exec):
        # sourcery skip: use-named-expression
        """redirect ref of pythonX to libpythonX.Y.dylib"

        used in:
            FrameworkPythonForExtBuilder
            FrameworkPythonForPkgBuilder
        """
        fixes = [
            (py_exec, "@executable_path/../Python"),
            (app_exec, "@executable_path/../../../../Python"),
        ]
        for exe, backref in fixes:
            dirs = DependencyManager(exe).analyze_executable()
            if dirs:
                dir_to_change = dirs[0]
                self.install_name_tool_change(
                    dir_to_change,
                    backref,
                    exe
                )
