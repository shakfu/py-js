#!/usr/bin/env python3
"""python3 -m builder
usage: builder [-h] [-v]  ...

builder: builds the py-js max external and python from source.

optional arguments:
  -h, --help            show this help message and exit
  -v, --version         show program's version number and exit

subcommands:
  valid subcommands

                        additional help
    dep_analyze         analyze dependencies
    pyjs_homebrew_ext   build portable pyjs externals (homebrew)
    pyjs_homebrew_pkg   build portable pyjs package (homebrew)
    pyjs_homebrew_sys   build non-portable pyjs externals (homebrew)
    pyjs_shared_ext     build portable pyjs externals (shared)
    pyjs_shared_pkg     build portable pyjs package (shared)
    pyjs_static_ext     build portable pyjs externals (static)
    pyjs_static_ext_full
                        build portable pyjs externals (fully-loaded static)
    python_framework    build framework python
    python_shared       build shared python
    python_shared_ext   build shared python to embed in external
    python_shared_pkg   build shared python to embed in package
    python_static       build static python
    python_static_full  build static python (fully-loaded)
    test                interactive testing shell
"""
import sysconfig
from . import config
from .cli import Commander, option, option_group
from .depend import DependencyManager
from .factory import python_builder_factory, pyjs_builder_factory

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
    option("-p", "--py-version", type=str, default=sysconfig.get_config_var('py_version'),
           help="set required python version to download and build")
)


class Application(Commander):
    """builder: builds the py-js max external and python from source."""
    name = 'pybuild'
    epilog = ''
    version = '0.1'
    default_args = ['--help']
    _argparse_levels = 1

    def ordered_dispatch(self, builder, args):
        """generic ordered argument dispatcher"""
        order = ['download', 'install', 'build', 'clean', 'ziplib']
        d = vars(args)
        builder.settings.update(d)
        for key in order:
            if d[key]:
                try:
                    getattr(builder, key)()
                    # print(builder.settings)
                except AttributeError:
                    print(builder, 'has no method', key)


    def do_python(self, args):
        "download and build python from src"

    @common_options
    def do_python_static(self, args):
        """build static python"""
        self.ordered_dispatch(config.static_python_builder, args)

    @common_options
    def do_python_static_full(self, args):
        """build static python (fully-loaded)"""
        self.ordered_dispatch(config.static_python_builder_full, args)

    @common_options
    def do_python_shared(self, args):
        """build shared python"""
        self.ordered_dispatch(config.shared_python_builder, args)

    @common_options
    def do_python_shared_ext(self, args):
        """build shared python to embed in external"""
        self.ordered_dispatch(config.shared_python_ext_builder, args)

    @common_options
    def do_python_shared_pkg(self, args):
        """build shared python to embed in package"""
        self.ordered_dispatch(config.shared_python_pkg_builder, args)

    @common_options
    def do_python_framework(self, args):
        """build framework python"""
        self.ordered_dispatch(config.framework_python_builder, args)

    # @common_options
    # def do_py_all(self, args):
    #     """build all python variations"""
    #     for builder_class in [#config.framework_python_builder, 
    #                           config.shared_python_builder, 
    #                           config.static_python_builder]:
    #         self.ordered_dispatch(builder_class, args)


    def do_pyjs(self, args):
        """build pyjs externals"""

    def do_pyjs_homebrew_sys(self, args):
        """build non-portable pyjs externals (homebrew)"""
        config.homebrew_builder.install_homebrew_sys()

    def do_pyjs_homebrew_pkg(self, args):
        """build portable pyjs package (homebrew)"""
        config.homebrew_builder.install_homebrew_pkg()

    def do_pyjs_homebrew_ext(self, args):
        """build portable pyjs externals (homebrew)"""
        config.homebrew_builder.install_homebrew_ext()

    @common_options
    def do_pyjs_shared_ext(self, args):
        """build portable pyjs externals (shared)"""
        self.ordered_dispatch(config.shared_ext_builder, args)

    @common_options
    def do_pyjs_static_ext(self, args):
        """build portable pyjs externals (static)"""
        self.ordered_dispatch(config.static_ext_builder, args)

    @common_options
    def do_pyjs_static_ext_full(self, args):
        """build portable pyjs externals (fully-loaded static)"""
        self.ordered_dispatch(config.static_ext_full_builder, args)

    @common_options
    def do_pyjs_shared_pkg(self, args):
        """build portable pyjs package (shared)"""
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
