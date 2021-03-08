"""bzip2: a builder for bzip2 dependency of python



"""

from .opsys import OSXBuilder


class Bzip2Builder(OSXBuilder):
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
