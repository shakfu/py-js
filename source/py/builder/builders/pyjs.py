"""python: a python builder and its subtypes

TODO: move PYTHON_VERSION_STRING to config.py

"""

import os
import platform
import re
import shutil
import subprocess

from ..projects import PyJsProject
from .python import PythonBuilder

PYTHON_VERSION_STRING = platform.python_version()  # e.g '3.9.2'


# ----------------------------------------------------------------------------
# PY-JS BUILDER

class PyJsBuilder(PythonBuilder):
    """Generic Python from src builder."""
    name = 'Python'
    project_class = PyJsProject
    version = PYTHON_VERSION_STRING
    depends_on = []
    suffix = ""
    setup_local = None
    patch = None
    targets = ['py', 'pyjs']

    def __init__(self, project=None, version=None, depends_on=None):
        if not project:
            project = self.project_class()
        super().__init__(project, version, depends_on)

        # dependency manager attributes (revise)
        self.install_names = {}
        self.deps = []
        self.dep_list = []

    # ------------------------------------------------------------------------
    # python properties


    # ------------------------------------------------------------------------
    # src-level operations
    def xcodebuild(self, project, target, flag=None):
        """build via xcode the given targets"""
            if not flag:
                self.cmd(f'xcodebuild -project targets/{project}/py-js.xcodeproj -target {target}')
            else:
                self.cmd(f"xcodebuild -project targets/{project}/py-js.xcodeproj -target {target} GCC_PREPROCESSOR_DEFINITIONS=$GCC_PREPROCESSOR_DEFINITIONS  {flag} ")

    def xbuild_targets(self, project, flag=None):
        """build via xcode the given targets"""
        for target in self.targets:
            self.xcodebuild(project, target, flag)


    def pre_process(self):
        """pre-build operations"""

    def post_process(self):
        """post-build operations"""

    def install(self):
        """install compilation product into lib"""
        # self.reset()
        # self.download()
        # self.pre_process()
        # self.build()
        # self.post_process()
