"""python_deps: builders for dependencies python

- bzip2
- openssl
- xz

"""

from .abstract import Builder


class Bzip2Builder(Builder):
    """Bzip2 static library builder"""
    name = 'bzip2'
    version = '1.0.8'
    url_template = 'https://sourceware.org/pub/bzip2/{name}-{version}.tar.gz'
    depends_on = []
    libs_static = ['libbz2.a']

    def build(self):
        if not self.libs_static_exist():
            self.chdir(self.src_path)
            self.cmd(f'make install PREFIX={self.prefix}')
            self.chdir(self.project.root)


class OpensslBuilder(Builder):
    """OpenSSL static library builder"""
    name = 'openssl'
    version = '1.1.1g'
    url_template = 'https://www.openssl.org/source/{name}-{version}.tar.gz'
    depends_on = []
    libs_static = ['libssl.a', 'libcrypto.a']

    def build(self):
        if not self.libs_static_exist():
            self.chdir(self.src_path)
            self.cmd(f'./config no-shared no-tests --prefix={self.prefix}')
            self.cmd('make install_sw')
            self.chdir(self.project.root)


class XzBuilder(Builder):
    """Xz static library builder"""
    name = 'xz'
    version = '5.2.5'
    url_template = 'http://tukaani.org/xz/{name}-{version}.tar.gz'
    depends_on = []
    libs_static = ['libxz.a']

    def build(self):
        if not self.libs_static_exist():
            self.chdir(self.src_path)
            self.cmd(f"""MACOSX_DEPLOYMENT_TARGET={self.mac_dep_target} \
                ./configure --disable-shared --enable-static --prefix={self.prefix}"""
                     )
            self.cmd('make && make install')
            self.chdir(self.project.root)
