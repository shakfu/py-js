"""shell: A mixin providing file and folder handling operations

Should keep this as platform agnostic as possible.

"""
import os
import shutil
from pathlib import Path


class ShellCmd:
    """Mixin for platform agnostic file/folder handling operations."""

    def cmd(self, shellcmd, *args, **kwargs):
        """Run shell command with args and keywords"""
        os.system(shellcmd.format(*args, **kwargs))

    __call__ = cmd

    def chdir(self, path):
        """Change current workding directory to path"""
        os.chdir(path)

    def move(self, src, dst):
        """Move from src path to dst path."""
        shutil.move(src, dst)

    def copytree(self, src, dst):
        """Copy recursively from src path to dst path."""
        shutil.copytree(src, dst)

    def copyfile(self, src, dst):
        """Copy file from src path to dst path."""
        shutil.copyfile(src, dst)

    def remove(self, path):
        """Remove file or folder."""
        path = Path(path)
        if path.is_dir():
            shutil.rmtree(path, ignore_errors=IGNORE_ERRORS)
        else:
            path.unlink(missing_ok=True)
