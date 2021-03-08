"""xz: a builder for xz dependency of python



"""

from .opsys import OSXBuilder


class XzBuilder(OSXBuilder):
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
