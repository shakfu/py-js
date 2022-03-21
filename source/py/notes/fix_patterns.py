"""
PythonBuilder [1,2]
    PythonSrcBuilder
        FrameworkPythonBuilder
        SharedPythonBuilder
            SharedPythonForExtBuilder [3]
            SharedPythonForPkgBuilder [4,5]
        StaticPythonBuilder
            StaticPythonFullBuilder
    PyJsBuilder
        HomebrewBuilder [6,7,8]
        StaticExtBuilder
        SharedExtBuilder
        SharedPkgBuilder
 
[x] fix_shared_dylib_for_pkg
[x] fix_shared_dylib_for_ext
[ ] fix_framework_dylib_for_pkg
[ ] fix_framework_dylib_for_ext

"""

class Fixer:
    """class to fix @rpath references"""

    def fix_shared_dylib_for_pkg(self):
        """install to dylib @rpath ref of external 'loader' to dylib in a pkg

        used in: 
            PythonBuilder(Builder): dylib-pkg
            SharedPythonForPkgBuilder(SharedPythonBuilder): dylib-pkg
        """
        self.cmd.chdir(self.prefix_lib)
        self.cmd.chmod(self.product.dylib)
        self.install_name_tool_id(
            f"@loader_path/../../../../support/{self.product.name_ver}/lib/{self.product.dylib}",
            self.product.dylib,
        )
        self.cmd.chdir(self.project.root)

    def fix_shared_dylib_for_ext(self):
        """install to dylib @rpath ref of external 'loader' to dylib in an ext

        used in: SharedPythonForExtBuilder(SharedPythonBuilder): dylib-ext (resources) 
        """
        self.cmd.chdir(self.prefix_lib)
        dylib_path = self.prefix_lib / self.product.dylib
        self.cmd.chmod(self.product.dylib)
        self.install_name_tool_id(
            f"@loader_path/../Resources/lib/{self.product.dylib}",
            self.product.dylib,
        )
        self.cmd.chdir(self.project.root)

    def fix_shared_exe_for_pkg(self):
        """redirect ref of pythonX to libpythonX.Y.dylib"

        used in:
            SharedPythonForPkgBuilder(SharedPythonBuilder): exe-pkg
        """
        self.cmd.chdir(self.prefix_bin)
        exe = self.product.name_ver
        d = DependencyManager(exe)
        dirs_to_change = d.analyze_executable()
        if dirs_to_change:
            dir_to_change = dirs_to_change[0]
            self.install_name_tool_change(
                dir_to_change,
                f"@executable_path/../lib/{self.product.dylib}", 
                exe
            )
        self.cmd.chdir(self.project.root)

# (6) HomebrewBuilder(PyJsBuilder): exe-brew
def fix_python_exec(self):  # sourcery skip: use-named-expression
    """change ref on executable to point to relative dylib"""
    self.cmd.chdir(self.prefix_bin)
    executable = self.product.name_ver
    result = subprocess.check_output(["otool", "-L", executable])
    entries = [line.decode("utf-8").strip() for line in result.splitlines()]
    for entry in entries:
        match = re.match(r"\s*(\S+)\s*\(compatibility version .+\)$", entry)
        if match:
            path = match.group(1)
            # homebrew files are installed in /usr/local/Cellar
            if path.startswith("/usr/local/Cellar/python"):
                self.install_name_tool_change(
                    path,
                    f"@executable_path/../{self.product.dylib}",
                    executable,
                )
    self.cmd.chdir(self.project.root)

# (7) HomebrewBuilder(PyJsBuilder): dylib-pkg
def fix_python_dylib_for_pkg(self):
    """change dylib ref to point to loader in package build format"""
    self.cmd.chdir(self.prefix)
    self.cmd.chmod(self.product.dylib)
    self.install_name_tool_id(
        f"@loader_path/../../../../support/{self.product.name_ver}/{self.product.dylib}",
        self.product.dylib,
    )
    self.cmd.chdir(self.project.root)

# (8) HomebrewBuilder(PyJsBuilder): ext (resources)
def fix_python_dylib_for_ext_resources(self):
    """change dylib ref to point to loader in external build format"""
    self.cmd.chdir(self.prefix)
    self.cmd.chmod(self.product.dylib)
    self.install_name_tool_id(
        f"@loader_path/../Resources/{self.product.name_ver}/{self.product.dylib}",
        self.product.dylib,
    )
    self.cmd.chdir(self.project.root)
