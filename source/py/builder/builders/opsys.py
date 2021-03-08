"""opsys: os platform-specific builders

currently implemented:
    - OSXBuilder -- macos platform-specific operations

"""

from .abstract import Builder


class OSXBuilder(Builder):
    """Macos platform-specific class and operations."""
    name = 'osx'
    mac_dep_target = '10.14'

    @property
    def dylib(self):
        """name of dynamic library in macos parlance."""
        return f'lib{self.name.lower()}{self.ver}.dylib' # pylint: disable=E1101

    def download(self):
        """download src using curl and tar.

        curl and tar are automatically available on mac platforms.
        """

        self.project.downloads.mkdir(parents=True, exist_ok=True)
        for dep in self.depends_on:
            dep.download()

        # download
        if not self.download_path.exists():
            self.log.info("downloading %s", self.download_path)
            self.cmd(f'curl -L --fail {self.url} -o {self.download_path}')

        # unpack
        if not self.src_path.exists():
            self.project.src.mkdir(parents=True, exist_ok=True)
            self.log.info("unpacking %s", self.src_path)
            self.cmd(f'tar -C {self.project.src} -xvf {self.download_path}')
