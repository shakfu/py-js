"""
Builder [1]
    Bzip2Builder
    OpensslBuilder
    XzBuilder
    PythonBuilder
        PythonSrcBuilder
            FrameworkPythonBuilder
            SharedPythonBuilder
                SharedPythonForExtBuilder
                SharedPythonForPkgBuilder
            StaticPythonBuilder
                StaticPythonFullBuilder
        PyJsBuilder
            HomebrewBuilder [2,3,4] homebrew-sys, homebrew-pkg, homebrew-ext
            StaticExtBuilder [5] static-ext
                StaticExtFullBuilder [6] static-ext-full
            SharedExtBuilder [7] shared-ext
            SharedPkgBuilder [8] shared-pk

"""

# (1) Builder
def xcodebuild(self, project: str, targets: List[str], *preprocessor_flags, **xcconfig_flags):
    """python wrapper around command-line xcodebuild"""
    x_flags = " ".join([f"{k}={repr(v)}" for k,v in xcconfig_flags.items()]) if xcconfig_flags else ''
    p_flags = "GCC_PREPROCESSOR_DEFINITIONS='$GCC_PREPROCESSOR_DEFINITIONS {flags}'".format(
        flags=" ".join([f"{k}=1" for k in preprocessor_flags])) if preprocessor_flags else ''
    for target in targets:
        self.cmd(
            f"xcodebuild -project 'targets/{project}/py-js.xcodeproj'"
            f" -target {repr(target)} {x_flags} {p_flags}"
        )

# [2,3,4] HomebrewBuilder
self.xcodebuild("homebrew-sys", targets=["py", "pyjs"])
self.xcodebuild("homebrew-pkg", targets=["py", "pyjs"])
self.xcodebuild("homebrew-ext", targets=["py", "pyjs"])

# [5] StaticExtBuilder
self.xcodebuild("static-ext", targets=["py", "pyjs"])

# [6] StaticExtFullBuilder
self.xcodebuild("static-ext-full", targets=["py", "pyjs"])

# [7] SharedExtBuilder
self.xcodebuild("shared-ext", targets=["py", "pyjs"])

# [8] SharedPkgBuilder
self.xcodebuild("shared-pkg", targets=["py", "pyjs"])
