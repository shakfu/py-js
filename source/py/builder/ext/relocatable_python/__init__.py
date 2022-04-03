from . import get
from .fix import fix_broken_signatures, fix_other_things
from .install import install_extras
from .relocatablizer import analyze, relocatablize

from ...cli import option, option_group

from sysconfig import get_config_var


def get_default_py_version():
    py_version = get_config_var('py_version_short')
    latest = ["3.7.9", "3.8.9", "3.9.12", "3.10.4"]
    for ver in latest:
        if ver.startswith(py_version):
            return ver


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
    # option("--python-version", default=get.DEFAULT_PYTHON_VERSION,
    option("--python-version", default=get_default_py_version(),
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
    ),
)



def process_args_for_relocatable_python(args):
    print(args.python_version)
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
