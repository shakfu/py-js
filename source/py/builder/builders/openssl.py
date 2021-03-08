"""bzip2: a builder for openssl dependency of python



"""

from .opsys import OSXBuilder


class OpensslBuilder(OSXBuilder):
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
