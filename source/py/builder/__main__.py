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
from . import utils
from .cli import Commander, option, option_group
from .depend import DependencyManager
from .ext.relocatable_python import relocatable_options
from .factory import builder_factory
from .sign import sign_externals_folder

# ----------------------------------------------------------------------------
# Commandline interface

common_options = option_group(
    option("-p", "--python-version", type=str,
           help="set required python version to download and build"),
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
    option("-z", 
            "--ziplib", action="store_true", help="zip python library"),
    option("--dump", action="store_true", help="dump project and product vars"),
    option("--release", action="store_true", help="set configuration to release"),
)

# combined_options = common_options + relocatable_options

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
        builder = builder_factory(name, **kwdargs)
        if args.dump:
            builder.to_yaml()
        for method in order:
            if method in kwdargs and kwdargs[method]:
                getattr(builder, method)()

# ----------------------------------------------------------------------------
# python builder methods

    def do_python(self, args):
        "download and build python from src"

    @common_options
    def do_python_static(self, args):
        """build static python"""
        self.ordered_dispatch('python_static', args)

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

    @option("--dump", action="store_true", 
        help="dump project and product vars")
    @option("-i", "--install", action="store_true",
           help="install python to build/lib")
    @relocatable_options
    def do_python_relocatable(self, args):
        """download relocatable framework python"""
        self.ordered_dispatch('python_relocatable', args)

# ----------------------------------------------------------------------------
# py-js builder methods


    def do_pyjs(self, args):
        """build pyjs externals"""

    @common_options
    def do_pyjs_local_sys(self, args):
        """build non-portable pyjs externals"""
        builder_factory('pyjs_local_sys', **vars(args)).build()
 
    @common_options
    def do_pyjs_homebrew_pkg(self, args):
        """build portable pyjs package (homebrew)"""
        builder_factory('pyjs_homebrew_pkg', **vars(args)).install_homebrew_pkg()

    @common_options
    def do_pyjs_homebrew_ext(self, args):
        """build portable pyjs externals (homebrew)"""
        builder_factory('pyjs_homebrew_ext', **vars(args)).install_homebrew_ext()

    @common_options
    def do_pyjs_static_pkg(self, args):
        """build portable pyjs externals (static)"""
        self.ordered_dispatch('pyjs_static', args)

    @common_options
    def do_pyjs_static_ext(self, args):
        """build portable pyjs externals (minimal static)"""
        self.ordered_dispatch('pyjs_static_ext', args)

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

    @option("--dump", action="store_true", 
        help="dump project and product vars")
    @option("-i", "--install", action="store_true",
           help="install python to build/lib")
    @option("-b", "--build", action="store_true",
           help="build python")
    @relocatable_options
    def do_pyjs_relocatable_pkg(self, args):
        """build portable pyjs package (framework)"""
        self.ordered_dispatch('pyjs_relocatable_pkg', args)

# ----------------------------------------------------------------------------
# dependency builder methods

    def do_dep(self, args):
        """dependency commands"""

    def do_dep_bz2(self, args):
        """build bzip2 dependency"""
        builder_factory('bz2').build()

    def do_dep_ssl(self, args):
        """build openssl dependency"""
        builder_factory('ssl').build()

    def do_dep_xz(self, args):
        """build xz dependency"""
        builder_factory('xz').build()



# ----------------------------------------------------------------------------
# help methods

    def do_help(self, args):
        """display online help"""
        utils.display_help()


# ----------------------------------------------------------------------------
# test methods

    @option('--without-homebrew', action='store_false', help='exclude homebrew entries')
    def do_test(self, args):
        """run all tests"""
        utils.run_logged_tests(with_homebrew=args.without_homebrew)

    def do_test_default(self, args):
        """run default test"""
        utils.runlog('default')

    def do_test_homebrew_ext(self, args):
        """run homebrew-ext test"""
        utils.runlog('homebrew-ext')

    def do_test_homebrew_pkg(self, args):
        """run homebrew-pkg test"""
        utils.runlog('homebrew-pkg')

    def do_test_framework_ext(self, args):
        """run framework-ext test"""
        utils.runlog('framework-ext')

    def do_test_framework_pkg(self, args):
        """run framework-pkg test"""
        utils.runlog('framework-pkg')

    def do_test_shared_ext(self, args):
        """run shared-ext test"""
        utils.runlog('shared-ext')

    def do_test_shared_pkg(self, args):
        """run shared-pkg test"""
        utils.runlog('shared-pkg')

    def do_test_static_ext(self, args):
        """run static-ext test"""
        utils.runlog('static-ext')

    def do_test_static_pkg(self, args):
        """run static-pkg test"""
        utils.runlog('static-pkg')


# ----------------------------------------------------------------------------
# utility methods

    def do_sign(self, args):
        """sign externals folder recursively"""
        sign_externals_folder()




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
