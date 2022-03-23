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
from .cli import Commander, option, option_group
from .depend import DependencyManager
from .factory import python_builder_factory, pyjs_builder_factory
from .ext.relocatable_python import analyze, relocatablize
from .ext.relocatable_python import install_extras
from .ext.relocatable_python import get
from .ext.relocatable_python import fix_broken_signatures, fix_other_things


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

relocatable_options = option_group(
    option("--destination",default="../../support",
        help="Directory destination for the Python.framework",
    ),
    option("--baseurl", default=get.DEFAULT_BASEURL,
        help="Override the base URL used to download the framework.",
    ),
    option("--os-version", default=get.DEFAULT_OS_VERSION,
        help="Override the macOS version of the downloaded pkg. "
        'Current supported versions are "10.6", "10.9", and "11". '
        "Not all Python version and macOS version combinations are valid.",
    ),
    option("--python-version", default=get.DEFAULT_PYTHON_VERSION,
        help="Override the version of the Python framework to be downloaded. "
        "See available versions at "
        "https://www.python.org/downloads/mac-osx/",
    ),
    option("--pip-requirements", default=None,
        help="Path to a pip freeze requirements.txt file that describes extra "
        "Python modules to be installed. If not provided, no modules will be installed.",
    ),
    option("--no-unsign", dest="unsign", action="store_false",
        help="Do not unsign binaries and libraries after they are relocatablized."
    ),
    option("--upgrade-pip", default=False, action="store_true",
        help="Upgrade pip prior to installing extra python modules."
    ),
    option("--without-pip", default=False, action="store_true",
        help="Do not install pip."
    ),)



class Application(Commander):
    """builder: builds the py-js max external and python from source."""
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
        #relocatablize('./targets/build/lib/Python.framework')

    @common_options
    def do_python_framework_pkg(self, args):
        """build framework python to embed in a package"""
        self.ordered_dispatch('python_framework_pkg', args)


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
        pyjs_builder_factory('pyjs_homebrew_pkg').install_homebrew_pkg()

    def do_pyjs_homebrew_ext(self, args):
        """build portable pyjs externals (homebrew)"""
        pyjs_builder_factory('pyjs_homebrew_ext').install_homebrew_ext()

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
        self.ordered_dispatch('pyjs_static_ext_full', args)

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
        framework_path = get.FrameworkGetter(
            python_version=args.python_version,
            os_version=args.os_version,
            base_url=args.baseurl,
        ).download_and_extract(destination=args.destination)

        if framework_path:
            files_relocatablized = relocatablize(framework_path)
            if args.unsign:
                fix_broken_signatures(files_relocatablized)
            short_version = ".".join(args.python_version.split(".")[0:2])
            install_extras(
                framework_path,
                version=short_version,
                requirements_file=args.pip_requirements,
                upgrade_pip=args.upgrade_pip,
                without_pip=args.without_pip
            )
            if fix_other_things(framework_path, short_version):
                print()
                print("Done!")
                print("Customized, relocatable framework is at %s" % framework_path)

        #pyjs_builder_factory('pyjs_framework_pkg').build()



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
