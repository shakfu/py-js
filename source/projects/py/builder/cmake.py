""" builder.cmake

cmake automation for builder using the python-cmake-buildsystem

ref: "https://github.com/python-cmake-buildsystem/python-cmake-buildsystem

"""

import os
import shutil

from .config import Project

PYTHON_CMAKE_BUILDSYSTEM_URL = \
    "https://github.com/python-cmake-buildsystem/python-cmake-buildsystem"

CMAKE_DEFAULT_OPTIONS = {
    "CMAKE_INSTALL_PREFIX:PATH": None,
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


def shellcmd(cmd, *args, **kwds):
    """run shellcmd"""
    os.system(cmd.format(*args, **kwds))

def git_clone(repo, to_dir):
    """retrieve git clone of repo"""
    shellcmd(f"git clone --depth=1 {repo} {to_dir}")

def cmake_generate(src_dir, build_dir, **options):
    """activate cmake configuration / generation stage"""
    opts = " ".join(f"-D{k}={v}" for k, v in options.items())
    shellcmd(
        f"cmake -S {src_dir} -B {build_dir} {opts}"
    )

def cmake_build(build_dir):
    """activate cmake build stage"""
    shellcmd(f"cmake --build {build_dir}")

def cmake_install(install_dir):
    """activate cmake install stage"""
    shellcmd(f"cmake --install {install_dir}")

def process():
    python_cmake_buildsystem = Project.build_downloads / 'python-cmake-buildsystem'
    python_cmake_build = Project.build_src / 'python-cmake-build'
    python_cmake_install = Project.build_lib / 'python-cmake'
    if not python_cmake_buildsystem.exists():
        git_clone(PYTHON_CMAKE_BUILDSYSTEM_URL, python_cmake_buildsystem)
    if python_cmake_build.exists():
        shutil.rmtree(python_cmake_build)
    python_cmake_build.mkdir(exist_ok=True)
    if python_cmake_install.exists():
        shutil.rmtree(python_cmake_install)
    python_cmake_install.mkdir(exist_ok=True)
    CMAKE_DEFAULT_OPTIONS["CMAKE_INSTALL_PREFIX"] = python_cmake_install
    cmake_generate(python_cmake_buildsystem, python_cmake_build, **CMAKE_DEFAULT_OPTIONS)
    cmake_build(python_cmake_build)
    cmake_install(python_cmake_install)







