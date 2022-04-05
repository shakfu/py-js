from . import get
from .fix import fix_broken_signatures, fix_other_things
from .install import install_extras
from .relocatablizer import analyze, relocatablize

from ...cli import option, option_group

from sysconfig import get_config_var

AVAILABLE_VERSIONS = ["3.7.9", "3.8.9", "3.9.12", "3.10.4"]

def get_default_py_version(version: str = None):
    _short_version = None
    if version:
        parts = version.split(".")
        if len(parts) == 2:
            _short_version = version
        else:
            _short_version = ".".join(parts[:2])
    else:
        _short_version = get_config_var("py_version_short")

    if _short_version:
        for ver in AVAILABLE_VERSIONS:
            if ver.startswith(_short_version):
                return ver

    print('version not found, selecting latest available version')
    return AVAILABLE_VERSIONS[-1]


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
    option("--release", action="store_true", 
        help="set configuration to release"
    ),
)


def download_relocatable_to(directory, settings):
    py_version = get_default_py_version(settings.python_version)
    py_short_version = ".".join(py_version.split(".")[0:2])
    framework_path = get.FrameworkGetter(
        python_version=py_version,
        os_version=settings.os_version,
        base_url=settings.baseurl,
        ).download_and_extract(destination=directory)

    if framework_path:
        files_relocatablized = relocatablize(framework_path)
        fix_broken_signatures(files_relocatablized)
        
        install_extras(
            framework_path,
            version=py_short_version,
            requirements_file=settings.pip_requirements,
            upgrade_pip=settings.upgrade_pip,
            without_pip=settings.without_pip
        )
        if fix_other_things(framework_path, py_short_version):
            print()
            print("Done!")
            print("Customized, relocatable framework is at %s" % framework_path)
