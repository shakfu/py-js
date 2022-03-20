from .core import (
    Product,
    Bzip2Builder, XzBuilder, OpensslBuilder,
    StaticPythonBuilder, SharedPythonBuilder, 
    FrameworkPythonBuilder,
    HomebrewBuilder, StaticExtBuilder, 
    SharedPythonForExtBuilder, SharedExtBuilder,
    SharedPythonForPkgBuilder, SharedPkgBuilder,
    StaticExtFullBuilder, StaticPythonFullBuilder,
)

from .constants import (
    DEFAULT_PYTHON_VERSION, 
    DEFAULT_BZ2_VERSION,
    DEFAULT_SSL_VERSION,
    DEFAULT_XZ_VERSION,
)



# -----------------------------------------------------------------------------
# UTILITY FUNCTIONS

# returns default if k is not found in d or d[k] returns None
get = lambda d, k, default: d[k] if (k in d and d[k]) else default

# -----------------------------------------------------------------------------
# DEPENDENCY PRODUCTS

def get_bzip2_product(bz2_version=DEFAULT_BZ2_VERSION, **settings):
    return Product(
        name="bzip2",
        version=bz2_version,
        url_template="https://sourceware.org/pub/bzip2/{name}-{version}.tar.gz",
        libs_static=["libbz2.a"],
        **settings
    )   

def get_ssl_product(ssl_version=DEFAULT_SSL_VERSION, **settings):
    return Product(
        name="openssl",
        version=ssl_version,
        url_template="https://www.openssl.org/source/{name}-{version}.tar.gz",
        libs_static=["libssl.a", "libcrypto.a"],
        **settings
    )

def get_xz_product(xz_version=DEFAULT_XZ_VERSION, **settings):
    return Product(
        name="xz",
        version="5.2.5",
        url_template="http://tukaani.org/xz/{name}-{version}.tar.gz",
        libs_static=["libxz.a"],
        **settings
    )


# -----------------------------------------------------------------------------
# PYTHON BUILDERS


def python_builder_factory(name, **settings):

    py_version = get(settings, 'py_version', DEFAULT_PYTHON_VERSION)
    bz2_version = get(settings, 'bz2_version', DEFAULT_BZ2_VERSION)
    ssl_version = get(settings, 'ssl_version', DEFAULT_SSL_VERSION) 
    xv_version = get(settings, 'xv_version', DEFAULT_XZ_VERSION)

    return dict(
        python_static = StaticPythonBuilder,
        python_static_full = StaticPythonFullBuilder,
        python_shared = SharedPythonBuilder,
        python_shared_ext = SharedPythonForExtBuilder,
        python_shared_pkg = SharedPythonForPkgBuilder,
        python_framework = FrameworkPythonBuilder,
    )[name](
        product=Product(
            name="Python",
            version=py_version,
            build_dir="-".join(name.split('_')[-2:]),
            url_template="https://www.python.org/ftp/python/{version}/Python-{version}.tgz",
            libs_static=[f"libpython{'.'.join(py_version.split('.')[:-1])}.a"],
        ),
        depends_on=[
            Bzip2Builder(product=get_bzip2_product(bz2_version), **settings),
            OpensslBuilder(product=get_ssl_product(ssl_version), **settings),
            XzBuilder(product=get_xz_product(xz_version), **settings),
        ],
        **settings
    )



# -----------------------------------------------------------------------------
# PYJS BUILDERS

def pyjs_builder_factory(name, py_version=DEFAULT_PYTHON_VERSION,
                 bz2_version=DEFAULT_BZ2_VERSION, ssl_version=DEFAULT_SSL_VERSION, 
                 xv_version=DEFAULT_XZ_VERSION, **settings):
    _builder, dependencies = dict(
        homebrew_builder = (HomebrewBuilder, []),
        static_ext_builder = (StaticExtBuilder, ['static_python_builder']),
        static_ext_full_builder = (StaticExtFullBuilder, ['static_python_builder_full']),
        shared_ext_builder = (SharedExtBuilder, ['shared_python_ext_builder']),
        shared_pkg_builder = (SharedPkgBuilder, ['shared_python_pkg_builder']),
    )[name]
    return _builder(
        product=Product(name='Python', version=py_version),
        depends_on=[
            python_builder(name, py_version, bz2_version, ssl_version, xv_version, **settings)
            for name in dependencies
        ],
        **settings
    )


# -----------------------------------------------------------------------------
# RECIPES

def get_static_python_recipe(name, py_version=DEFAULT_PYTHON_VERSION, 
                             bz2_version=DEFAULT_BZ2_VERSION, 
                             ssl_version=DEFAULT_SSL_VERSION, 
                             xz_version=DEFAULT_XZ_VERSION, **settings):
    return Recipe(
        name=name,
        builders=[
            StaticPythonBuilder(
                product=Product(
                    name="Python",
                    version=py_version,
                    build_dir="python-static",
                    url_template="https://www.python.org/ftp/python/{version}/Python-{version}.tgz",
                    libs_static=[f"libpython{'.'.join(py_version.split('.')[:-1])}.a"],
                ),
                depends_on=[
                    Bzip2Builder(product=get_bzip2_product(bz2_version, **settings)),
                    OpensslBuilder(product=get_ssl_product(ssl_version, **settings)),
                    XzBuilder(product=get_xz_product(xz_version, **settings)),
                ]
            )
        ]
    )
