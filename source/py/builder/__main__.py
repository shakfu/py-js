#!/usr/bin/env python3
"""builder -- build python3 from source (currently for macos)

    usage: pybuild.py [-h] [-v] {all,framework,shared,static} ...

    pybuild: builds python from src

    optional arguments:
      -h, --help            show this help message and exit
      -v, --version         show program's version number and exit

    subcommands:
      valid subcommands
                            additional help
        all                 build all variations
        framework           build framework python
        shared              build shared python
        static              build static python

"""
from .builders import (FrameworkPythonBuilder, SharedPythonBuilder,
                       StaticPythonBuilder)
from .utils.cli import Commander, option, option_group

# ------------------------------------------------------------------------------
# Commandline interface

common_options = option_group(
    option("-d",
           "--download",
           action="store_true",
           help="download python build/downloads"),
    option("-r", "--reset", action="store_true", help="reset python build"),
    option("-i",
           "--install",
           action="store_true",
           help="install python to build/lib"),
    option("-b",
           "--build",
           action="store_true",
           help="build python in build/src"),
    option("-c",
           "--clean",
           action="store_true",
           help="clean python in build/src"),
    option("-z", "--ziplib", action="store_true", help="zip python library"),
)


class Application(Commander):
    """builder: builds the py-js max external and python from source."""
    name = 'pybuild'
    epilog = ''
    version = '0.1'
    default_args = ['--help']

    def dispatch(self, builder_class, args):
        """generic dispatcher"""
        builder = builder_class()
        if args.download:
            builder.download()
        elif args.reset:
            builder.reset()
        elif args.install:
            builder.install()
        elif args.build:
            builder.build()
        elif args.clean:
            builder.clean()
        elif args.ziplib:
            builder.ziplib()
        else:
            pass

    @common_options
    def do_static(self, args):
        """build static python"""
        self.dispatch(StaticPythonBuilder, args)

    @common_options
    def do_shared(self, args):
        """build shared python"""
        self.dispatch(SharedPythonBuilder, args)

    @common_options
    def do_framework(self, args):
        """build framework python"""
        self.dispatch(FrameworkPythonBuilder, args)

    @common_options
    def do_all(self, args):
        """build all python variations"""
        for klass in [FrameworkPythonBuilder, SharedPythonBuilder, StaticPythonBuilder]:
            self.dispatch(klass, args)

if __name__ == '__main__':
    app = Application()
    app.cmdline()
