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
from .depend import DependencyManager

# ----------------------------------------------------------------------------
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

    def ordered_dispatch(self, builder, args):
        """generic ordered argument dispatcher"""
        order = ['download', 'install', 'build', 'clean', 'ziplib']
        d = vars(args)
        for key in order:
            if d[key]:
                try:
                    getattr(builder, key)()
                except AttributeError:
                    print(builder, 'has no method', key)

    @common_options
    def do_py_static(self, args):
        """build static python"""
        self.ordered_dispatch(config.static_python_builder, args)

    @common_options
    def do_py_static_full(self, args):
        """build static python (fully-loaded)"""
        self.ordered_dispatch(config.static_python_builder_full, args)

    @common_options
    def do_py_shared(self, args):
        """build shared python"""
        self.ordered_dispatch(config.shared_python_builder, args)

    @common_options
    def do_py_shared_ext(self, args):
        """build shared python to embed in external"""
        self.ordered_dispatch(config.shared_python_ext_builder, args)

    @common_options
    def do_py_shared_pkg(self, args):
        """build shared python to embed in package"""
        self.ordered_dispatch(config.shared_python_pkg_builder, args)

    # @common_options
    # def do_py_framework(self, args):
    #     """build framework python"""
    #     self.ordered_dispatch(config.framework_python_builder, args)

    # @common_options
    # def do_py_all(self, args):
    #     """build all python variations"""
    #     for builder_class in [#config.framework_python_builder, 
    #                           config.shared_python_builder, 
    #                           config.static_python_builder]:
    #         self.ordered_dispatch(builder_class, args)

    def do_brew_sys(self, args):
        """build non-portable pyjs package (homebrew)"""
        config.homebrew_builder.install_homebrew_sys()

    def do_brew_pkg(self, args):
        """build portable pyjs package (homebrew)"""
        config.homebrew_builder.install_homebrew_pkg()

    def do_brew_ext(self, args):
        """build portable pyjs externals (homebrew)"""
        config.homebrew_builder.install_homebrew_ext()

    @common_options
    def do_shared_ext(self, args):
        """build portable pyjs externals (shared py)"""
        self.ordered_dispatch(config.shared_ext_builder, args)

    @common_options
    def do_static_ext(self, args):
        """build portable pyjs externals (static py)"""
        self.ordered_dispatch(config.static_ext_builder, args)

    @common_options
    def do_static_ext_full(self, args):
        """build portable pyjs externals (fully loaded static py)"""
        self.ordered_dispatch(config.static_ext_full_builder, args)

    @common_options
    def do_shared_pkg(self, args):
        """build portable pyjs externals embedded in a package"""
        self.ordered_dispatch(config.shared_pkg_builder, args)


    @option("--exec-ref", "-e", type=str, help="back ref for executable or plugin")
    @option("--staticlibs-dir", "-l", type=str, help="static lib directory fir static substitutes")
    @option("--dest-dir", "-d", type=str, help="where target dylib will be copied to with copied dependents")
    @option("target", help="dylib or executable to made relocatable")
    def do_dep_analyze(self, args):
        """analyze dependencies"""
        d = DependencyManager(args.target, args.dest_dir, args.staticlibs_dir, args.exec_ref)
        d.analyze()
        from pprint import pprint
        d.get_deps()
        pprint(d.install_names)

    @common_options
    def do_test(self, args):
        """interactive testing shell"""
        from IPython import embed
        embed(colors="neutral")




if __name__ == '__main__':
    app = Application()
    app.cmdline()
