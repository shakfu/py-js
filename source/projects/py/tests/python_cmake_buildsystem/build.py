""" builder.cmake

cmake automation for builder using the python-cmake-buildsystem

ref: "https://github.com/python-cmake-buildsystem/python-cmake-buildsystem

"""

import os
import shutil
from pathlib import Path

PYTHON_CMAKE_BUILDSYSTEM_URL = \
    "https://github.com/python-cmake-buildsystem/python-cmake-buildsystem"

CMAKE_DEFAULT_OPTIONS = {
    "PYTHON_VERSION": "3.9.17",
    "PYTHON_APPLY_PATCHES": "ON",
    "CMAKE_BUILD_TYPE": "Debug",
    "DOWNLOAD_SOURCES": "ON",
    "BUILD_LIBPYTHON_SHARED": "OFF",
    "BUILD_EXTENSIONS_AS_BUILTIN": "OFF",
    "WITH_STATIC_DEPENDENCIES": "OFF",
    "INSTALL_DEVELOPMENT": "ON",
    "INSTALL_MANUAL": "OFF",
    "INSTALL_TEST": "OFF",
    "USE_LIB64": "OFF",
    "WITH_C_LOCALE_COERCION": "ON",
    "WITH_DECIMAL_CONTEXTVAR": "ON",
    "WITH_TRACE_REFS": "OFF",
    "WITH_SSL_DEFAULT_SUITES": "python",
    "EXTRA_PYTHONPATH": "", # colon (:) separated
    "USE_SYSTEM_LIBRARIES": "ON",
    "USE_SYSTEM_Curses": "ON",
    "USE_SYSTEM_EXPAT": "ON",
    "USE_SYSTEM_LibFFI": "OFF",
    "USE_SYSTEM_OpenSSL": "ON",
    "USE_SYSTEM_TCL": "ON",
    "USE_SYSTEM_ZLIB": "ON",
    "USE_SYSTEM_DB": "ON",
    "USE_SYSTEM_GDBM": "ON",
    "USE_SYSTEM_LZMA": "ON",
    "USE_SYSTEM_READLINE": "ON",
    "USE_SYSTEM_SQLite3": "ON",

    # extension options
    "ENABLE_ARRAY": "ON",
    "BUILTIN_ARRAY": "OFF",
    "ENABLE_AUDIOOP": "ON",
    "BUILTIN_AUDIOOP": "OFF",
    "ENABLE_BINASCII": "ON",
    "BUILTIN_BINASCII": "OFF",
    "ENABLE_BISECT": "ON",
    "BUILTIN_BISECT": "OFF",
    "ENABLE_BSDDB": "ON",
    "BUILTIN_BSDDB": "OFF",
    "ENABLE_BZ2": "ON",
    "BUILTIN_BZ2": "OFF",
    "ENABLE_CMATH": "ON",
    "BUILTIN_CMATH": "OFF",
    "ENABLE_CODECS_CN": "ON",
    "BUILTIN_CODECS_CN": "OFF",
    "ENABLE_CODECS_HK": "ON",
    "BUILTIN_CODECS_HK": "OFF",
    "ENABLE_CODECS_ISO2022": "ON",
    "BUILTIN_CODECS_ISO2022": "OFF",
    "ENABLE_CODECS_JP": "ON",
    "BUILTIN_CODECS_JP": "OFF",
    "ENABLE_CODECS_KR": "ON",
    "BUILTIN_CODECS_KR": "OFF",
    "ENABLE_CODECS_TW": "ON",
    "BUILTIN_CODECS_TW": "OFF",
    "ENABLE_COLLECTIONS": "ON",
    "BUILTIN_COLLECTIONS": "OFF",
    "ENABLE_CPICKLE": "ON",
    "BUILTIN_CPICKLE": "OFF",
    "ENABLE_CRYPT": "ON",
    "BUILTIN_CRYPT": "OFF",
    "ENABLE_CSTRINGIO": "ON",
    "BUILTIN_CSTRINGIO": "OFF",
    "ENABLE_CSV": "ON",
    "BUILTIN_CSV": "OFF",
    "ENABLE_CTYPES": "ON",
    "BUILTIN_CTYPES": "OFF",
    "ENABLE_CTYPES_TEST": "ON",
    "BUILTIN_CTYPES_TEST": "OFF",
    "ENABLE_CURSES": "ON",
    "BUILTIN_CURSES": "OFF",
    "ENABLE_CURSES_PANEL": "ON",
    "BUILTIN_CURSES_PANEL": "OFF",
    "ENABLE_DATETIME": "ON",
    "BUILTIN_DATETIME": "OFF",
    "ENABLE_DBM": "ON",
    "BUILTIN_DBM": "OFF",
    "ENABLE_ELEMENTTREE": "ON",
    "BUILTIN_ELEMENTTREE": "OFF",
    "ENABLE_FCNTL": "ON",
    "BUILTIN_FCNTL": "OFF",
    "ENABLE_FUNCTOOLS": "ON",
    "BUILTIN_FUNCTOOLS": "OFF",
    "ENABLE_FUTURE_BUILTINS": "ON",
    "BUILTIN_FUTURE_BUILTINS": "OFF",
    "ENABLE_GDBM": "ON",
    "BUILTIN_GDBM": "OFF",
    "ENABLE_GRP": "ON",
    "BUILTIN_GRP": "OFF",
    "ENABLE_HASHLIB": "ON",
    "BUILTIN_HASHLIB": "OFF",
    "ENABLE_HEAPQ": "ON",
    "BUILTIN_HEAPQ": "OFF",
    "ENABLE_HOTSHOT": "ON",
    "BUILTIN_HOTSHOT": "OFF",
    "ENABLE_IO": "ON",
    "BUILTIN_IO": "OFF",
    "ENABLE_ITERTOOLS": "ON",
    "BUILTIN_ITERTOOLS": "OFF",
    "ENABLE_JSON": "ON",
    "BUILTIN_JSON": "OFF",
    "ENABLE_LINUXAUDIODEV": "ON",
    "BUILTIN_LINUXAUDIODEV": "OFF",
    "ENABLE_LOCALE": "ON",
    "BUILTIN_LOCALE": "OFF",
    "ENABLE_LSPROF": "ON",
    "BUILTIN_LSPROF": "OFF",
    "ENABLE_LZMA": "ON",
    "BUILTIN_LZMA": "OFF",
    "ENABLE_MATH": "ON",
    "BUILTIN_MATH": "OFF",
    "ENABLE_MMAP": "ON",
    "BUILTIN_MMAP": "OFF",
    "ENABLE_MULTIBYTECODEC": "ON",
    "BUILTIN_MULTIBYTECODEC": "OFF",
    "ENABLE_MULTIPROCESSING": "ON",
    "BUILTIN_MULTIPROCESSING": "OFF",
    "ENABLE_NIS": "ON",
    "BUILTIN_NIS": "OFF",
    "ENABLE_NT": "ON",
    "BUILTIN_NT": "ON",
    "ENABLE_OPERATOR": "ON",
    "BUILTIN_OPERATOR": "OFF",
    "ENABLE_OSSAUDIODEV": "ON",
    "BUILTIN_OSSAUDIODEV": "OFF",
    "ENABLE_PARSER": "ON",
    "BUILTIN_PARSER": "OFF",
    "ENABLE_POSIX": "ON",
    "BUILTIN_POSIX": "ON",
    "ENABLE_PWD": "ON",
    "BUILTIN_PWD": "ON",
    "ENABLE_PYEXPAT": "ON",
    "BUILTIN_PYEXPAT": "OFF",
    "ENABLE_RANDOM": "ON",
    "BUILTIN_RANDOM": "OFF",
    "ENABLE_READLINE": "ON",
    "BUILTIN_READLINE": "OFF",
    "ENABLE_RESOURCE": "ON",
    "BUILTIN_RESOURCE": "OFF",
    "ENABLE_SELECT": "ON",
    "BUILTIN_SELECT": "OFF",
    "ENABLE_SOCKET": "ON",
    "BUILTIN_SOCKET": "OFF",
    "ENABLE_SPWD": "ON",
    "BUILTIN_SPWD": "OFF",
    "ENABLE_SQLITE3": "ON",
    "BUILTIN_SQLITE3": "OFF",
    "ENABLE_SSL": "ON",
    "BUILTIN_SSL": "OFF",
    "ENABLE_STROP": "ON",
    "BUILTIN_STROP": "OFF",
    "ENABLE_STRUCT": "ON",
    "BUILTIN_STRUCT": "OFF",
    "ENABLE_SYSLOG": "ON",
    "BUILTIN_SYSLOG": "OFF",
    "ENABLE_TERMIOS": "ON",
    "BUILTIN_TERMIOS": "OFF",
    "ENABLE_TESTCAPI": "ON",
    "BUILTIN_TESTCAPI": "OFF",
    "ENABLE_TIME": "ON",
    "BUILTIN_TIME": "OFF",
    "ENABLE_TKINTER": "ON",
    "BUILTIN_TKINTER": "OFF",
    "ENABLE_UNICODEDATA": "ON",
    "BUILTIN_UNICODEDATA": "OFF",
    "ENABLE_ZLIB": "ON",
    "BUILTIN_ZLIB": "OFF",
}

CUSTOM_OPTIONS = {
    "CMAKE_INSTALL_PREFIX:PATH": None,
    "BUILD_EXTENSIONS_AS_BUILTIN": "ON",
    "BUILD_LIBPYTHON_SHARED": "ON",
    # "WITH_STATIC_DEPENDENCIES": "ON",

    # extensions
    "ENABLE_AUDIOOP": "OFF",
    "ENABLE_BSDDB": "OFF",
    "ENABLE_CODECS_CN": "OFF",
    "ENABLE_CODECS_HK": "OFF",
    "ENABLE_CODECS_ISO2022": "OFF",
    "ENABLE_CODECS_JP": "OFF",
    "ENABLE_CODECS_KR": "OFF",
    "ENABLE_CODECS_TW": "OFF",
    "ENABLE_CPICKLE": "OFF",
    "ENABLE_CTYPES": "OFF",
    "ENABLE_CTYPES_TEST": "OFF",
    "ENABLE_CURSES": "OFF",
    "ENABLE_CURSES_PANEL": "OFF",
    "ENABLE_DBM": "OFF",
    "ENABLE_GDBM": "OFF",
    "ENABLE_HASHLIB": "OFF",
    "ENABLE_HOTSHOT": "OFF",
    "ENABLE_LINUXAUDIODEV": "OFF",
    "ENABLE_LOCALE": "OFF",
    "ENABLE_LSPROF": "OFF",
    "ENABLE_MULTIBYTECODEC": "OFF",
    "ENABLE_NIS": "OFF",    
    "ENABLE_OSSAUDIODEV": "OFF",
    "ENABLE_READLINE": "OFF",
    "ENABLE_SPWD": "OFF",
    "ENABLE_SQLITE3": "OFF",
    "ENABLE_SSL": "OFF",
    "ENABLE_SYSLOG": "OFF",
    "ENABLE_TERMIOS": "OFF",
    "ENABLE_TESTCAPI": "OFF",
    "ENABLE_TKINTER": "OFF",
    "ENABLE_UNICODEDATA": "OFF",
}

class CMakeBuilder:

    def shellcmd(self, cmd, *args, **kwds):
        """run shellcmd"""
        os.system(cmd.format(*args, **kwds))

    def git_clone(self, repo, to_dir):
        """retrieve git clone of repo"""
        self.shellcmd(f"git clone --depth=1 {repo} {to_dir}")

    def apply_patch(self, to_file, patch):
        """Apply a patch to a file.
        """
        self.shellcmd(f"patch {to_file} < '{patch}'")

    def cmake_generate(self, src_dir, build_dir, **options):
        """activate cmake configuration / generation stage"""
        _options = CMAKE_DEFAULT_OPTIONS.copy()
        _options.update(options)
        opts = " ".join(f"-D{k}={v}" for k, v in _options.items())
        self.shellcmd(
            f"cmake -S {src_dir} -B {build_dir} {opts}"
        )

    def cmake_build(self, build_dir):
        """activate cmake build stage"""
        self.shellcmd(f"cmake --build {build_dir}")

    def cmake_install(self, build_dir):
        """activate cmake install stage"""
        self.shellcmd(f"cmake --install {build_dir}")

    def build(self):
        python_cmake_buildsystem = Path('python-cmake-buildsystem')
        python_cmake_build = Path('python-cmake-build')
        python_cmake_install = Path('python-cmake-install')
        if not python_cmake_buildsystem.exists():
            self.git_clone(PYTHON_CMAKE_BUILDSYSTEM_URL, python_cmake_buildsystem)
            self.apply_patch(
                to_file=python_cmake_buildsystem / 'cmake' / 'extensions' / 'CMakeLists.txt',
                patch='scproxy.patch'
            )
        for _dir in [python_cmake_build, python_cmake_install]: # reset dirs every run
            if _dir.exists():
                shutil,rmtree(_dir)
            _dir.mkdir(exist_ok=True)
        CUSTOM_OPTIONS["CMAKE_INSTALL_PREFIX"] = python_cmake_install
        self.cmake_generate(python_cmake_buildsystem, python_cmake_build, **CUSTOM_OPTIONS)
        self.cmake_build(python_cmake_build)
        self.cmake_install(python_cmake_build)


if __name__ == '__main__':
    CMakeBuilder().build()

