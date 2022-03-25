#!/usr/bin/env python3
"""
usage: python3 -m builder [-h] [-v]  ...

builder: builds python and py-js max externals from source or other methods.

optional arguments:
  -h, --help     show this help message and exit
  -v, --version  show program's version number and exit

subcommands:
  valid subcommands

                 additional help
    pyjs         build pyjs externals
    python       download and build python from src

"""
from .cli import Commander, option, option_group
from .depend import DependencyManager
from .factory import python_builder_factory, pyjs_builder_factory
from .ext.relocatable_python import (
    process_args_for_relocatable_python, relocatable_options)


# ----------------------------------------------------------------------------
# Commandline interface

common_options = option_group(
    option("--dump", action="store_true", help="dump project and product vars"),
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
    option("-p", "--py-version", type=str,
           help="set required python version to download and build"),
)


class Application(Commander):
    """builder: builds python and py-js max externals from source or other methods."""
    name = 'builder'
    epilog = ''
    version = '0.1'
    default_args = ['--help']
    _argparse_levels = 1

    def ordered_dispatch(self, name, args):
        """generic ordered argument dispatcher"""
        order = ['dump', 'download', 'install', 'build', 'clean', 'ziplib']
        kwdargs = vars(args)
        print(kwdargs)
        if name.startswith('python'):
            # print('selecting python-factory')
            factory = python_builder_factory
        else:
            # print('selecting pyjs-factory')
            factory = pyjs_builder_factory
        builder = factory(name, **kwdargs)
        if args.dump:
            builder.to_yaml()
        for method in order:
            if method in kwdargs and kwdargs[method]:
                try:
                    getattr(builder, method)()
                except AttributeError:
                    print(builder, 'has no method', method)

# ----------------------------------------------------------------------------
# python builder methods

    def do_python(self, args):
        "download and build python from src"

    @common_options
    def do_python_static(self, args):
        """build static python"""
        self.ordered_dispatch('python_static', args)

    @common_options
    def do_python_static_full(self, args):
        """build static python (fully-loaded)"""
        self.ordered_dispatch('python_static_full', args)

    @common_options
    def do_python_shared(self, args):
        """build shared python"""
        self.ordered_dispatch('python_shared', args)

    @common_options
    def do_python_shared_ext(self, args):
        """build shared python to embed in external"""
        self.ordered_dispatch('python_shared_ext', args)

    @common_options
    def do_python_shared_pkg(self, args):
        """build shared python to embed in package"""
        self.ordered_dispatch('python_shared_pkg', args)

    @common_options
    def do_python_framework(self, args):
        """build framework python"""
        self.ordered_dispatch('python_framework', args)

    @common_options
    def do_python_framework_ext(self, args):
        """build framework python to embed external"""
        self.ordered_dispatch('python_framework_ext', args)

    @common_options
    def do_python_framework_pkg(self, args):
        """build framework python to embed in a package"""
        self.ordered_dispatch('python_framework_pkg', args)

    @relocatable_options
    def do_python_relocatable(self, args):
        """download relocatable framework python"""
        process_args_for_relocatable_python(args)
        python_builder_factory('python_relocatable').install()



# ----------------------------------------------------------------------------
# py-js builder methods


    def do_pyjs(self, args):
        """build pyjs externals"""
        pyjs_builder_factory('pyjs_local_sys').build()

    def do_pyjs_local_sys(self, args):
        """build non-portable pyjs externals"""
        pyjs_builder_factory('pyjs_local_sys').build()

    def do_pyjs_homebrew_pkg(self, args):
        """build portable pyjs package (homebrew)"""
        pyjs_builder_factory('pyjs_homebrew_pkg').install_homebrew_pkg()  # type: ignore

    def do_pyjs_homebrew_ext(self, args):
        """build portable pyjs externals (homebrew)"""
        pyjs_builder_factory('pyjs_homebrew_ext').install_homebrew_ext()  # type: ignore

    @common_options
    def do_pyjs_static_pkg(self, args):
        """build portable pyjs externals (static)"""
        self.ordered_dispatch('pyjs_static', args)

    @common_options
    def do_pyjs_static_ext(self, args):
        """build portable pyjs externals (minimal static)"""
        self.ordered_dispatch('pyjs_static_ext', args)

    @common_options
    def do_pyjs_static_ext_full(self, args):
        """build portable pyjs externals (fully-loaded static)"""
        self.ordered_dispatch('pyjs_static_ext_full', args)

    # @common_options
    # def do_pyjs_static_pkg(self, args):
    #     """build portable pyjs externals (static)"""
    #     self.ordered_dispatch('pyjs_static_pkg', args)

    @common_options
    def do_pyjs_shared_ext(self, args):
        """build portable pyjs externals (shared)"""
        self.ordered_dispatch('pyjs_shared_ext', args)

    @common_options
    def do_pyjs_shared_pkg(self, args):
        """build portable pyjs package (shared)"""
        self.ordered_dispatch('pyjs_shared_pkg', args)

    @common_options
    def do_pyjs_framework_ext(self, args):
        """build portable pyjs externals (framework)"""
        self.ordered_dispatch('pyjs_framework_ext', args)

    @common_options
    def do_pyjs_framework_pkg(self, args):
        """build portable pyjs package (framework)"""
        self.ordered_dispatch('pyjs_framework_pkg', args)

    @relocatable_options
    def do_pyjs_relocatable_pkg(self, args):
        """build portable pyjs package (framework)"""
        process_args_for_relocatable_python(args)
        python_builder_factory('python_relocatable').install()
        pyjs_builder_factory('pyjs_relocatable_pkg').build()


# ----------------------------------------------------------------------------
# utility methods

    # def do_check(self, args):
    #     """check reference utilities"""
    
    # @option("--exec-ref", "-e", type=str, help="back ref for executable or plugin")
    # @option("--staticlibs-dir", "-l", type=str, help="static lib directory fir static substitutes")
    # @option("--dest-dir", "-d", type=str, help="where target dylib will be copied to with copied dependents")
    # @option("target", help="dylib or executable to made relocatable")
    # def do_check_dependencies(self, args):
    #     """analyze dependencies"""
    #     d = DependencyManager(args.target, args.dest_dir, args.staticlibs_dir, args.exec_ref)
    #     d.analyze()
    #     from pprint import pprint
    #     d.get_deps()
    #     pprint(d.install_names)

    # @option("target", help="external to analyze")
    # def do_check_external(self, args):
    #     """analyze external"""
    #     import yaml
    #     with open('dump.yml', 'w') as f:
    #         d = analyze(f'../../externals/{args.target}')
    #         yaml.safe_dump(d, f, indent=4)

    # @common_options
    # def do_test(self, args):
    #     """interactive testing shell"""
    #     from IPython import embed
    #     embed(colors="neutral")


if __name__ == '__main__':
    app = Application()
    app.cmdline()
