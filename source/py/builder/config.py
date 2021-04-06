from .core import (Product, Recipe, PYTHON_VERSION_STRING,
                   Bzip2Builder, XzBuilder, OpensslBuilder,
                   StaticPythonBuilder, SharedPythonBuilder, FrameworkPythonBuilder,
                   HomebrewBuilder, StaticExtBuilder)

bzip2_product = Product(
    name="bzip2",
    version="1.0.8",
    url_template="https://sourceware.org/pub/bzip2/{name}-{version}.tar.gz",
    libs_static=["libbz2.a"],
)

ssl_product = Product(
    name="openssl",
    version="1.1.1g",
    url_template="https://www.openssl.org/source/{name}-{version}.tar.gz",
    libs_static=["libssl.a", "libcrypto.a"],
)

xz_product = Product(
    name="xz",
    version="5.2.5",
    url_template="http://tukaani.org/xz/{name}-{version}.tar.gz",
    libs_static=["libxz.a"],
)

def py_product(build_dir):
    return Product(
        name="Python",
        version=PYTHON_VERSION_STRING,
        build_dir=build_dir,
        url_template="https://www.python.org/ftp/python/{version}/Python-{version}.tgz",
        libs_static=["libpython3.9.a"],
    )

pyjs_product = Product(
    name="Python",
    version=PYTHON_VERSION_STRING,
)

bzip2_builder = Bzip2Builder(product=bzip2_product)

ssl_builder = OpensslBuilder(product=ssl_product)

xz_builder = XzBuilder(product=xz_product)

static_python_builder = StaticPythonBuilder(
    product=py_product('python-static'), depends_on=[
        bzip2_builder, ssl_builder, xz_builder]
)

shared_python_builder = SharedPythonBuilder(
    product=py_product('python-shared'), depends_on=[
        bzip2_builder, ssl_builder, xz_builder]
)

# framework_python_builder = FrameworkPythonBuilder(
#     product=py_product('python-framework'), depends_on=[
#         bzip2_builder, ssl_builder, xz_builder]
# )

homebrew_builder = HomebrewBuilder(product=pyjs_product)

staticext_builder = StaticExtBuilder(product=pyjs_product)

build_all_recipe = Recipe(
    name="build_all",
    builders=[
        static_python_builder,
        shared_python_builder,
        # framework_python_builder,
        homebrew_builder,
    ],
)

self_contained_recipe = Recipe(
    name="self_contained",
    builders=[
        StaticPythonBuilder(
            product=Product(
                name="Python",
                version=PYTHON_VERSION_STRING,
                build_dir="python-static",
                url_template="https://www.python.org/ftp/python/{version}/Python-{version}.tgz",
                libs_static=["libpython3.9.a"],
            ),
            depends_on=[
                Bzip2Builder(product=Product(
                    name="bzip2",
                    version="1.0.8",
                    url_template="https://sourceware.org/pub/bzip2/{name}-{version}.tar.gz",
                    libs_static=["libbz2.a"],
                )),
                OpensslBuilder(product=Product(
                    name="openssl",
                    version="1.1.1g",
                    url_template="https://www.openssl.org/source/{name}-{version}.tar.gz",
                    libs_static=["libssl.a", "libcrypto.a"],
                )),
                XzBuilder(product=Product(
                    name="xz",
                    version="5.2.5",
                    url_template="http://tukaani.org/xz/{name}-{version}.tar.gz",
                    libs_static=["libxz.a"],
                )),
            ]
        )
    ]
)

# homebrew_builder.install_homebrew_sys()
