""" builder.cmake

Preliminary sketch of `PythonCmakeBuilder` which provides python build via
cmake courtesy of the the python-cmake-buildsystem project.

ref: "https://github.com/python-cmake-buildsystem/python-cmake-buildsystem

"""

import os
import shutil

from .config import Project

PYTHON_CMAKE_BUILDSYSTEM_URL = \
    "https://github.com/shakfu/python-cmake-buildsystem"

CMAKE_DEFAULT_OPTIONS = {
    "PYTHON_VERSION":               "3.9.17",
    "PYTHON_APPLY_PATCHES":         "ON",
    "CMAKE_BUILD_TYPE":             "Debug",
    "DOWNLOAD_SOURCES":             "ON",
    "BUILD_LIBPYTHON_SHARED":       "OFF",
    "BUILD_EXTENSIONS_AS_BUILTIN":  "OFF",
    "WITH_STATIC_DEPENDENCIES":     "OFF",
    "INSTALL_DEVELOPMENT":          "ON",
    "INSTALL_MANUAL":               "OFF",
    "INSTALL_TEST":                 "OFF",
    "USE_LIB64":                    "OFF",
    "WITH_C_LOCALE_COERCION":       "ON",
    "WITH_DECIMAL_CONTEXTVAR":      "ON",
    "WITH_TRACE_REFS":              "OFF",
    "WITH_SSL_DEFAULT_SUITES":      "python",
    "EXTRA_PYTHONPATH":             "", # colon (:) separated
    "USE_SYSTEM_LIBRARIES":         "ON",
    "USE_SYSTEM_Curses":            "ON",
    "USE_SYSTEM_EXPAT":             "ON",
    "USE_SYSTEM_LibFFI":            "OFF",
    "USE_SYSTEM_OpenSSL":           "ON",
    "USE_SYSTEM_TCL":               "ON",
    "USE_SYSTEM_ZLIB":              "ON",
    "USE_SYSTEM_DB":                "ON",
    "USE_SYSTEM_GDBM":              "ON",
    "USE_SYSTEM_LZMA":              "ON",
    "USE_SYSTEM_READLINE":          "ON",
    "USE_SYSTEM_SQLite3":           "ON",

    # extension options
    "ENABLE_ARRAY":                 "ON",
    "ENABLE_AUDIOOP":               "ON",
    "ENABLE_BINASCII":              "ON",
    "ENABLE_BISECT":                "ON",
    "ENABLE_BSDDB":                 "ON",
    "ENABLE_BZ2":                   "ON",
    "ENABLE_CMATH":                 "ON",
    "ENABLE_CODECS_CN":             "ON",
    "ENABLE_CODECS_HK":             "ON",
    "ENABLE_CODECS_ISO2022":        "ON",
    "ENABLE_CODECS_JP":             "ON",
    "ENABLE_CODECS_KR":             "ON",
    "ENABLE_CODECS_TW":             "ON",
    "ENABLE_COLLECTIONS":           "ON",
    "ENABLE_CPICKLE":               "ON",
    "ENABLE_CRYPT":                 "ON",
    "ENABLE_CSTRINGIO":             "ON",
    "ENABLE_CSV":                   "ON",
    "ENABLE_CTYPES":                "ON",
    "ENABLE_CTYPES_TEST":           "ON",
    "ENABLE_CURSES":                "ON",
    "ENABLE_CURSES_PANEL":          "ON",
    "ENABLE_DATETIME":              "ON",
    "ENABLE_DBM":                   "ON",
    "ENABLE_ELEMENTTREE":           "ON",
    "ENABLE_FCNTL":                 "ON",
    "ENABLE_FUNCTOOLS":             "ON",
    "ENABLE_FUTURE_BUILTINS":       "ON",
    "ENABLE_GDBM":                  "ON",
    "ENABLE_GRP":                   "ON",
    "ENABLE_HASHLIB":               "ON",
    "ENABLE_HEAPQ":                 "ON",
    "ENABLE_HOTSHOT":               "ON",
    "ENABLE_IO":                    "ON",
    "ENABLE_ITERTOOLS":             "ON",
    "ENABLE_JSON":                  "ON",
    "ENABLE_LINUXAUDIODEV":         "ON",
    "ENABLE_LOCALE":                "ON",
    "ENABLE_LSPROF":                "ON",
    "ENABLE_LZMA":                  "ON",
    "ENABLE_MATH":                  "ON",
    "ENABLE_MMAP":                  "ON",
    "ENABLE_MULTIBYTECODEC":        "ON",
    "ENABLE_MULTIPROCESSING":       "ON",
    "ENABLE_NIS":                   "ON",
    "ENABLE_NT":                    "ON",
    "ENABLE_OPERATOR":              "ON",
    "ENABLE_OSSAUDIODEV":           "ON",
    "ENABLE_PARSER":                "ON",
    "ENABLE_POSIX":                 "ON",
    "ENABLE_PWD":                   "ON",
    "ENABLE_PYEXPAT":               "ON",
    "ENABLE_RANDOM":                "ON",
    "ENABLE_READLINE":              "ON",
    "ENABLE_RESOURCE":              "ON",
    "ENABLE_SELECT":                "ON",
    "ENABLE_SOCKET":                "ON",
    "ENABLE_SPWD":                  "ON",
    "ENABLE_SQLITE3":               "ON",
    "ENABLE_SSL":                   "ON",
    "ENABLE_STROP":                 "ON",
    "ENABLE_STRUCT":                "ON",
    "ENABLE_SYSLOG":                "ON",
    "ENABLE_TERMIOS":               "ON",
    "ENABLE_TESTCAPI":              "ON",
    "ENABLE_TIME":                  "ON",
    "ENABLE_TKINTER":               "ON",
    "ENABLE_UNICODEDATA":           "ON",
    "ENABLE_ZLIB":                  "ON",

    "BUILTIN_ARRAY":                "OFF",
    "BUILTIN_AUDIOOP":              "OFF",
    "BUILTIN_BINASCII":             "OFF",
    "BUILTIN_BISECT":               "OFF",
    "BUILTIN_BSDDB":                "OFF",
    "BUILTIN_BZ2":                  "OFF",
    "BUILTIN_CMATH":                "OFF",
    "BUILTIN_CODECS_CN":            "OFF",
    "BUILTIN_CODECS_HK":            "OFF",
    "BUILTIN_CODECS_ISO2022":       "OFF",
    "BUILTIN_CODECS_JP":            "OFF",
    "BUILTIN_CODECS_KR":            "OFF",
    "BUILTIN_CODECS_TW":            "OFF",
    "BUILTIN_COLLECTIONS":          "OFF",
    "BUILTIN_CPICKLE":              "OFF",
    "BUILTIN_CRYPT":                "OFF",
    "BUILTIN_CSTRINGIO":            "OFF",
    "BUILTIN_CSV":                  "OFF",
    "BUILTIN_CTYPES":               "OFF",
    "BUILTIN_CTYPES_TEST":          "OFF",
    "BUILTIN_CURSES":               "OFF",
    "BUILTIN_CURSES_PANEL":         "OFF",
    "BUILTIN_DATETIME":             "OFF",
    "BUILTIN_DBM":                  "OFF",
    "BUILTIN_ELEMENTTREE":          "OFF",
    "BUILTIN_FCNTL":                "OFF",
    "BUILTIN_FUNCTOOLS":            "OFF",
    "BUILTIN_FUTURE_BUILTINS":      "OFF",
    "BUILTIN_GDBM":                 "OFF",
    "BUILTIN_GRP":                  "OFF",
    "BUILTIN_HASHLIB":              "OFF",
    "BUILTIN_HEAPQ":                "OFF",
    "BUILTIN_HOTSHOT":              "OFF",
    "BUILTIN_IO":                   "OFF",
    "BUILTIN_ITERTOOLS":            "OFF",
    "BUILTIN_JSON":                 "OFF",
    "BUILTIN_LINUXAUDIODEV":        "OFF",
    "BUILTIN_LOCALE":               "OFF",
    "BUILTIN_LSPROF":               "OFF",
    "BUILTIN_LZMA":                 "OFF",
    "BUILTIN_MATH":                 "OFF",
    "BUILTIN_MMAP":                 "OFF",
    "BUILTIN_MULTIBYTECODEC":       "OFF",
    "BUILTIN_MULTIPROCESSING":      "OFF",
    "BUILTIN_NIS":                  "OFF",
    "BUILTIN_NT":                   "ON",
    "BUILTIN_OPERATOR":             "OFF",
    "BUILTIN_OSSAUDIODEV":          "OFF",
    "BUILTIN_PARSER":               "OFF",
    "BUILTIN_POSIX":                "ON",
    "BUILTIN_PWD":                  "ON",
    "BUILTIN_PYEXPAT":              "OFF",
    "BUILTIN_RANDOM":               "OFF",
    "BUILTIN_READLINE":             "OFF",
    "BUILTIN_RESOURCE":             "OFF",
    "BUILTIN_SELECT":               "OFF",
    "BUILTIN_SOCKET":               "OFF",
    "BUILTIN_SPWD":                 "OFF",
    "BUILTIN_SQLITE3":              "OFF",
    "BUILTIN_SSL":                  "OFF",
    "BUILTIN_STROP":                "OFF",
    "BUILTIN_STRUCT":               "OFF",
    "BUILTIN_SYSLOG":               "OFF",
    "BUILTIN_TERMIOS":              "OFF",
    "BUILTIN_TESTCAPI":             "OFF",
    "BUILTIN_TIME":                 "OFF",
    "BUILTIN_TKINTER":              "OFF",
    "BUILTIN_UNICODEDATA":          "OFF",
    "BUILTIN_ZLIB":                 "OFF",
}

CUSTOM_OPTIONS = {
    "CMAKE_INSTALL_PREFIX:PATH":    None,
    "BUILD_EXTENSIONS_AS_BUILTIN":  "ON",
    "BUILD_LIBPYTHON_SHARED":       "ON",
    # "WITH_STATIC_DEPENDENCIES":   "ON",

    # extensions
    "ENABLE_AUDIOOP":               "OFF",
    "ENABLE_BSDDB":                 "OFF",
    "ENABLE_CODECS_CN":             "OFF",
    "ENABLE_CODECS_HK":             "OFF",
    "ENABLE_CODECS_ISO2022":        "OFF",
    "ENABLE_CODECS_JP":             "OFF",
    "ENABLE_CODECS_KR":             "OFF",
    "ENABLE_CODECS_TW":             "OFF",
    "ENABLE_CPICKLE":               "OFF",
    "ENABLE_CTYPES":                "OFF",
    "ENABLE_CTYPES_TEST":           "OFF",
    "ENABLE_CURSES":                "OFF",
    "ENABLE_CURSES_PANEL":          "OFF",
    "ENABLE_DBM":                   "OFF",
    "ENABLE_GDBM":                  "OFF",
    "ENABLE_HASHLIB":               "OFF",
    "ENABLE_HOTSHOT":               "OFF",
    "ENABLE_LINUXAUDIODEV":         "OFF",
    "ENABLE_LOCALE":                "OFF",
    "ENABLE_LSPROF":                "OFF",
    "ENABLE_MULTIBYTECODEC":        "OFF",
    "ENABLE_NIS":                   "OFF",    
    "ENABLE_OSSAUDIODEV":           "OFF",
    "ENABLE_READLINE":              "OFF",
    "ENABLE_SPWD":                  "OFF",
    "ENABLE_SQLITE3":               "OFF",
    "ENABLE_SSL":                   "OFF",
    "ENABLE_SYSLOG":                "OFF",
    "ENABLE_TERMIOS":               "OFF",
    "ENABLE_TESTCAPI":              "OFF",
    "ENABLE_TKINTER":               "OFF",
    "ENABLE_UNICODEDATA":           "OFF",
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
        python_cmake_buildsystem = Project.build_downloads / 'python-cmake-buildsystem'
        python_cmake_build = Project.build_src / 'python-cmake-build'
        python_cmake_install = Project.build_lib / 'python-cmake'
        if not python_cmake_buildsystem.exists():
            self.git_clone(PYTHON_CMAKE_BUILDSYSTEM_URL, python_cmake_buildsystem)
            # patch is applied to fork
            # self.apply_patch(
            #     to_file=python_cmake_buildsystem / 'cmake' / 'extensions' / 'CMakeLists.txt',
            #     patch=(Project.patch / 'python-cmake-buildsystem' / 'scproxy.patch')
            # )
        for _dir in [python_cmake_build, python_cmake_install]: # reset dirs every run
            if _dir.exists():
                shutil.rmtree(_dir)
            _dir.mkdir(exist_ok=True)
        CUSTOM_OPTIONS["CMAKE_INSTALL_PREFIX"] = python_cmake_install
        self.cmake_generate(python_cmake_buildsystem, python_cmake_build, **CUSTOM_OPTIONS)
        self.cmake_build(python_cmake_build)
        self.cmake_install(python_cmake_build)


if __name__ == '__main__':
    CMakeBuilder().build()






