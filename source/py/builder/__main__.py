#!/usr/bin/env python3
"""builder -- build python3 from source (currently for macos)

usage: builder [-h] [-v] [subcommand] [action]

builder: builds the py-js max external and python from source.

optional arguments:
  -h, --help            show this help message and exit
  -v, --version         show program's version number and exit

subcommands:
  valid subcommands
                        additional help
    py_all              build all python variations
    py_framework        build framework python
    py_shared           build shared python
    py_static           build static python
    pyjs_ext            build portable pyjs externals (homebrew)
    pyjs_pkg            build portable pyjs package (homebrew)
    pyjs_sys            build non-portable pyjs package (homebrew)
    test                interactive testing shell
"""
from . import config
from .cli import Commander, option, option_group

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

    def dispatch(self, builder, args):
        """generic argument dispatcher"""
        for key in vars(args):
            if key == 'func':
                continue
            if getattr(args, key):
                try:
                    getattr(builder, key)()
                except AttributeError:
                    print(builder, 'has no method', key)

    @common_options
    def do_py_static(self, args):
        """build static python"""
        self.dispatch(config.static_python_builder, args)

    @common_options
    def do_py_shared(self, args):
        """build shared python"""
        self.dispatch(config.shared_python_builder, args)

    @common_options
    def do_py_framework(self, args):
        """build framework python"""
        self.dispatch(config.framework_python_builder, args)

    @common_options
    def do_py_all(self, args):
        """build all python variations"""
        for builder_class in [config.framework_python_builder, 
                              config.shared_python_builder, 
                              config.static_python_builder]:
            self.dispatch(builder_class, args)

    @common_options
    def do_pyjs_sys(self, args):
        """build non-portable pyjs package (homebrew)"""
        config.homebrew_builder.install_homebrew_sys()

    @common_options
    def do_pyjs_pkg(self, args):
        """build portable pyjs package (homebrew)"""
        config.homebrew_builder.install_homebrew_pkg()

    @common_options
    def do_pyjs_ext(self, args):
        """build portable pyjs externals (homebrew)"""
        config.homebrew_builder.install_homebrew_ext()

    @common_options
    def do_test(self, args):
        """interactive testing shell"""
        from IPython import embed
        embed(colors="neutral")




if __name__ == '__main__':
    app = Application()
    app.cmdline()
