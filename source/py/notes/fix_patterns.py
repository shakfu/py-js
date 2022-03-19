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
"""

# (1) PythonBuilder(Builder): dylib-pkg
def fix_python_dylib_for_pkg(self):
    """redirect ref of dylib to loader in a package deployment."""
    self.cmd.chdir(self.prefix_lib)
    self.cmd.chmod(self.product.dylib)
    self.install_name_tool_id(
        f"@loader_path/../../../../support/{self.product.name}/lib/{self.product.dylib}",
        self.product.dylib,
    )
    self.cmd.chdir(self.project.root)

# (2) PythonBuilder(Builder): dylib-ext
def fix_python_dylib_for_ext(self):
    """redirect ref of dylib to loader in a self-contained external deployment."""
    self.cmd.chdir(self.prefix_lib)
    self.cmd.chmod(self.product.dylib)
    self.install_name_tool_id(
        f"@loader_path/{self.product.dylib}", self.product.dylib
    )
    self.cmd.chdir(self.project.root)

# (3) SharedPythonForExtBuilder(SharedPythonBuilder): dylib-ext (resources)
def fix_python_dylib_for_ext_resources(self):
    """change dylib ref to point to loader in external build format"""
    self.cmd.chdir(self.prefix / 'lib')
    dylib_path = self.prefix / 'lib' / self.product.dylib
    assert dylib_path.exists()
    self.cmd.chmod(self.product.dylib)
    self.install_name_tool_id(
        f"@loader_path/../Resources/lib/{self.product.dylib}",
        self.product.dylib,
    )
    self.cmd.chdir(self.project.root)

## same as (1)
# (4) SharedPythonForPkgBuilder(SharedPythonBuilder): dylib-pkg
def fix_python_dylib_for_pkg(self):
    """change dylib ref to point to loader in package build format"""
    self.cmd.chdir(self.prefix / 'lib')
    dylib_path = self.prefix / 'lib' / self.product.dylib
    assert dylib_path.exists(), f"{dylib_path} does not exist"
    self.cmd.chmod(self.product.dylib)
    # both of these are equivalent (and both don't work!)
    self.install_name_tool_id(
        f"@loader_path/../../../../support/{self.product.name_ver}/lib/{self.product.dylib}",
        self.product.dylib,
    )
    self.cmd.chdir(self.project.root)

# (5) SharedPythonForPkgBuilder(SharedPythonBuilder): exe-pkg
def fix_python_exe_for_pkg(self):
    """redirect ref of pythonX to libpythonX.Y.dylib"""
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
