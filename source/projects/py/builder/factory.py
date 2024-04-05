from . import core
from .config import (
    Project,
    CURRENT_PYTHON_VERSION,
    DEFAULT_BZ2_VERSION,
    DEFAULT_SSL_VERSION,
    DEFAULT_XZ_VERSION,
)


# -----------------------------------------------------------------------------
# UTILITY FUNCTIONS


# returns default if k is not found in d or d[k] returns None
def get(d, k, default):
    return d[k] if k in d and d[k] else default


# -----------------------------------------------------------------------------
# FACTORY MANAGER


class FactoryManager:
    """
    The builder.FactoryManager organizes production of custom Python builds and
    Python3 externals.

    It provides factory methods which dispatch to specialized builders
    which build a corresponding product.

    FM -> factories -> builders -> products.
    """

    PYTHON_BUILDERS = dict(
        python_shared=core.SharedPythonBuilder,
        python_shared_ext=core.SharedPythonForExtBuilder,
        python_shared_pkg=core.SharedPythonForPkgBuilder,
        python_shared_tiny=core.TinySharedPythonBuilder,
        python_framework=core.FrameworkPythonBuilder,
        python_framework_ext=core.FrameworkPythonForExtBuilder,
        python_framework_pkg=core.FrameworkPythonForPkgBuilder,
        python_relocatable=core.RelocatablePythonBuilder,
        python_static=core.StaticPythonBuilder,
        python_static_tiny=core.TinyStaticPythonBuilder,
        python_beeware=core.BeewarePythonBuilder,
        python_cmake=core.PythonCmakeBuilder,
    )

    PYJS_BUILDERS = dict(
        pyjs_local_sys=(core.LocalSystemBuilder, []),
        pyjs_homebrew_ext=(core.HomebrewExtBuilder, []),
        pyjs_homebrew_pkg=(core.HomebrewPkgBuilder, []),
        pyjs_shared_ext=(core.SharedExtBuilder, ["python_shared_ext"]),
        pyjs_shared_tiny_ext=(core.SharedExtBuilder, ["python_shared_tiny"]),
        pyjs_shared_pkg=(core.SharedPkgBuilder, ["python_shared_pkg"]),
        pyjs_framework_ext=(core.FrameworkExtBuilder, ["python_framework_ext"]),
        pyjs_framework_pkg=(core.FrameworkPkgBuilder, ["python_framework_pkg"]),
        pyjs_relocatable_pkg=(core.RelocatablePkgBuilder, ["python_relocatable"]),
        pyjs_static_ext=(core.StaticExtBuilder, ["python_static"]),
        pyjs_static_tiny_ext=(core.StaticExtBuilder, ["python_static_tiny"]),
        pyjs_beeware_ext=(core.BeewareExtBuilder, ["python_beeware"]),
    )

    # -----------------------------------------------------------------------------
    # DEPENDENCY PRODUCTS

    def get_bzip2_product(self, bz2_version=DEFAULT_BZ2_VERSION, **settings):
        return core.Product(
            name="bzip2",
            version=bz2_version,
            url_template="https://sourceware.org/pub/bzip2/{name}-{version}.tar.gz",
            libs_static=["libbz2.a"],
        )

    def get_ssl_product(self, ssl_version=DEFAULT_SSL_VERSION, **settings):
        return core.Product(
            name="openssl",
            version=ssl_version,
            url_template="https://www.openssl.org/source/{name}-{version}.tar.gz",
            libs_static=["libssl.a", "libcrypto.a"],
        )

    def get_xz_product(self, xz_version=DEFAULT_XZ_VERSION, **settings):
        return core.Product(
            name="xz",
            version="5.2.5",
            url_template="https://github.com/python/cpython-source-deps/archive/refs/tags/{name}-{version}.tar.gz",
            # url_template="http://tukaani.org/xz/{name}-{version}.tar.gz",
            libs_static=["liblzma.a"],
        )

    # -----------------------------------------------------------------------------
    # DEPENDENCY BUILDERS

    def dependency_builder_factory(self, name, **settings):
        bz2_version = get(settings, "bz2_version", DEFAULT_BZ2_VERSION)
        ssl_version = get(settings, "ssl_version", DEFAULT_SSL_VERSION)
        xz_version = get(settings, "xz_version", DEFAULT_XZ_VERSION)

        return {
            "bz2": core.Bzip2Builder(
                product=self.get_bzip2_product(bz2_version), **settings
            ),
            "ssl": core.OpensslBuilder(
                product=self.get_ssl_product(ssl_version), **settings
            ),
            "xz": core.XzBuilder(product=self.get_xz_product(xz_version), **settings),
        }[name]

    # -----------------------------------------------------------------------------
    # PYTHON BUILDERS

    def python_builder_factory(self, name, **settings):
        py_version = get(settings, "python_version", CURRENT_PYTHON_VERSION)
        bz2_version = get(settings, "bz2_version", DEFAULT_BZ2_VERSION)
        ssl_version = get(settings, "ssl_version", DEFAULT_SSL_VERSION)
        xz_version = get(settings, "xz_version", DEFAULT_XZ_VERSION)

        _builder = self.PYTHON_BUILDERS[name]

        _dependencies = [
            core.Bzip2Builder(product=self.get_bzip2_product(bz2_version), **settings),
            core.OpensslBuilder(product=self.get_ssl_product(ssl_version), **settings),
            core.XzBuilder(product=self.get_xz_product(xz_version), **settings),
        ]

        product = core.Product(
            name="Python",
            version=py_version,
            # build_dir="-".join(name.split("_")),
            build_dir="-".join(name.split("_")[:2]),
            url_template="https://www.python.org/ftp/python/{version}/Python-{version}.tgz",
            libs_static=[f"libpython{'.'.join(py_version.split('.')[:-1])}.a"],
        )

        _independent_python_builders = [
            core.PythonCmakeBuilder,
            core.RelocatablePythonBuilder,
        ]

        if any(isinstance(_builder, i) for i in _independent_python_builders):
            return _builder(product=product, **settings)
        else:
            return _builder(product=product, depends_on=_dependencies, **settings)

    # -----------------------------------------------------------------------------
    # PYJS BUILDERS

    def pyjs_builder_factory(self, name, **settings):
        py_version = get(settings, "python_version", CURRENT_PYTHON_VERSION)
        get(settings, "bz2_version", DEFAULT_BZ2_VERSION)
        get(settings, "ssl_version", DEFAULT_SSL_VERSION)
        get(settings, "xz_version", DEFAULT_XZ_VERSION)

        _builder, dependencies = self.PYJS_BUILDERS[name]
        if dependencies:
            return _builder(
                product=core.Product(name="Python", version=py_version),
                depends_on=[
                    self.python_builder_factory(name, **settings)
                    for name in dependencies
                ],
                **settings,
            )
        else:
            # no dep builder is a local builder therefore default_py_ver
            return _builder(
                product=core.Product(name="Python", version=CURRENT_PYTHON_VERSION),
                **settings,
            )

    # -----------------------------------------------------------------------------
    # GENERIC BUILDERS

    def builder_factory(self, name, **settings):
        builder = None
        try:
            builder = self.pyjs_builder_factory(name, **settings)
            Project().record_variant(name)
        except KeyError:
            try:
                builder = self.python_builder_factory(name, **settings)
            except KeyError:
                try:
                    builder = self.dependency_builder_factory(name, **settings)
                except KeyError:
                    print(f"builder type '{name}' not found in any factory.")

        return builder

    # -----------------------------------------------------------------------------
    # RECIPES

    # An example of a recipe which can fix requirements to versions.

    def get_static_python_recipe(
        self,
        name,
        py_version=CURRENT_PYTHON_VERSION,
        bz2_version=DEFAULT_BZ2_VERSION,
        ssl_version=DEFAULT_SSL_VERSION,
        xz_version=DEFAULT_XZ_VERSION,
        **settings,
    ):
        return core.Recipe(
            name=name,
            builders=[
                core.StaticPythonBuilder(
                    product=core.Product(
                        name="Python",
                        version=py_version,
                        build_dir="python-static",
                        url_template="https://www.python.org/ftp/python/{version}/Python-{version}.tgz",
                        libs_static=[
                            f"libpython{'.'.join(py_version.split('.')[:-1])}.a"
                        ],
                    ),
                    depends_on=[
                        core.Bzip2Builder(
                            product=self.get_bzip2_product(bz2_version, **settings)
                        ),
                        core.OpensslBuilder(
                            product=self.get_ssl_product(ssl_version, **settings)
                        ),
                        core.XzBuilder(
                            product=self.get_xz_product(xz_version, **settings)
                        ),
                    ],
                )
            ],
        )
