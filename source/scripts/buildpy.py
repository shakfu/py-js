#!/usr/bin/env python3
"""buildpy.py - builds python from source

repo: https://github.com/shakfu/buildpy

features:

- Single script which downloads, builds python from source
- Different build configurations (static, dynamic) possible
- Trims python builds and zips site-packages by default.

class structure:

Config
    PythonConfig311
    PythonConfig312
    PythonConfig313

ShellCmd
    Project
    AbstractBuilder
        Builder
            OpensslBuilder
            Bzip2Builder
            XzBuilder
            PythonBuilder
                PythonDebugBuilder

"""

import datetime
import json
import logging
import os
import platform
import shutil
import stat
import subprocess
import sys
import tarfile
from fnmatch import fnmatch
from pathlib import Path
from typing import Callable, Optional, Union
from urllib.request import urlretrieve

__version__ = "0.0.1"

# ----------------------------------------------------------------------------
# type aliases

Pathlike = Union[str, Path]
MatchFn = Callable[[Path], bool]
ActionFn = Callable[[Path], None]

# ----------------------------------------------------------------------------
# env helpers

def getenv(key: str, default: bool = False) -> bool:
    """convert '0','1' env values to bool {True, False}"""
    return bool(int(os.getenv(key, default)))

def setenv(key: str, default: str):
    """get environ variable if it is exists else set default"""
    if key in os.environ:
        return os.getenv(key, default)
    else:
        os.environ[key] = default
        return default

# ----------------------------------------------------------------------------
# constants

PYTHON = sys.executable
PLATFORM = platform.system()
ARCH = platform.machine()
PY_VER_MINOR = sys.version_info.minor
if PLATFORM == "Darwin":
    MACOSX_DEPLOYMENT_TARGET = setenv("MACOSX_DEPLOYMENT_TARGET", "12.6")
DEFAULT_PY_VERSION = "3.13.2"
DEBUG = getenv('DEBUG', default=True)
COLOR = getenv('COLOR', default=True)

# ----------------------------------------------------------------------------
# logging config

class CustomFormatter(logging.Formatter):
    """custom logging formatting class"""

    white = "\x1b[97;20m"
    grey = "\x1b[38;20m"
    green = "\x1b[32;20m"
    cyan = "\x1b[36;20m"
    yellow = "\x1b[33;20m"
    red = "\x1b[31;20m"
    bold_red = "\x1b[31;1m"
    reset = "\x1b[0m"
    fmt = "%(delta)s - %(levelname)s - %(name)s.%(funcName)s - %(message)s"
    cfmt = (f"{white}%(delta)s{reset} - "
            f"{{}}%(levelname)s{{}} - "
            f"{white}%(name)s.%(funcName)s{reset} - "
            f"{grey}%(message)s{reset}")

    FORMATS = {
        logging.DEBUG: cfmt.format(grey, reset),
        logging.INFO: cfmt.format(green, reset),
        logging.WARNING: cfmt.format(yellow, reset),
        logging.ERROR: cfmt.format(red, reset),
        logging.CRITICAL: cfmt.format(bold_red, reset),
    }

    def __init__(self, use_color=COLOR):
        self.use_color = use_color

    def format(self, record):
        """custom logger formatting method"""
        if not self.use_color:
            log_fmt = self.fmt
        else:
            log_fmt = self.FORMATS.get(record.levelno)
        if PY_VER_MINOR > 10:
            duration = datetime.datetime.fromtimestamp(
                record.relativeCreated / 1000, datetime.UTC
            )
        else:
            duration = datetime.datetime.utcfromtimestamp(
                record.relativeCreated / 1000)
        record.delta = duration.strftime("%H:%M:%S")
        formatter = logging.Formatter(log_fmt)
        return formatter.format(record)


strm_handler = logging.StreamHandler()
strm_handler.setFormatter(CustomFormatter())
# file_handler = logging.FileHandler("log.txt", mode='w')
# file_handler.setFormatter(CustomFormatter(use_color=False))
logging.basicConfig(
    level=logging.DEBUG if DEBUG else logging.INFO,
    handlers=[strm_handler],
    # handlers=[strm_handler, file_handler],
)


# ----------------------------------------------------------------------------
# config classes

BASE_CONFIG = {
    "header": [
        "DESTLIB=$(LIBDEST)",
        "MACHDESTLIB=$(BINLIBDEST)",
        "DESTPATH=",
        "SITEPATH=",
        "TESTPATH=",
        "COREPYTHONPATH=$(DESTPATH)$(SITEPATH)$(TESTPATH)",
        "PYTHONPATH=$(COREPYTHONPATH)",
        "OPENSSL=$(srcdir)/../../install/openssl",
        "BZIP2=$(srcdir)/../../install/bzip2",
        "LZMA=$(srcdir)/../../install/xz",
    ],
    "extensions": {
        "_abc": ["_abc.c"],
        "_asyncio": ["_asynciomodule.c"],
        "_bisect": ["_bisectmodule.c"],
        "_blake2": [
            "_blake2/blake2module.c",
            "_blake2/blake2b_impl.c",
            "_blake2/blake2s_impl.c",
        ],
        "_bz2": [
            "_bz2module.c",
            "-I$(BZIP2)/include",
            "-L$(BZIP2)/lib",
            "$(BZIP2)/lib/libbz2.a",
        ],
        "_codecs": ["_codecsmodule.c"],
        "_codecs_cn": ["cjkcodecs/_codecs_cn.c"],
        "_codecs_hk": ["cjkcodecs/_codecs_hk.c"],
        "_codecs_iso2022": ["cjkcodecs/_codecs_iso2022.c"],
        "_codecs_jp": ["cjkcodecs/_codecs_jp.c"],
        "_codecs_kr": ["cjkcodecs/_codecs_kr.c"],
        "_codecs_tw": ["cjkcodecs/_codecs_tw.c"],
        "_collections": ["_collectionsmodule.c"],
        "_contextvars": ["_contextvarsmodule.c"],
        "_crypt":  ["_cryptmodule.c", "-lcrypt"],
        "_csv": ["_csv.c"],
        "_ctypes": [
            "_ctypes/_ctypes.c",
            "_ctypes/callbacks.c",
            "_ctypes/callproc.c",
            "_ctypes/stgdict.c",
            "_ctypes/cfield.c",
            "-ldl",
            "-lffi",
            "-DHAVE_FFI_PREP_CIF_VAR",
            "-DHAVE_FFI_PREP_CLOSURE_LOC",
            "-DHAVE_FFI_CLOSURE_ALLOC",
        ],
        "_curses": ["-lncurses", "-lncursesw", "-ltermcap", "_cursesmodule.c"],
        "_curses_panel": ["-lpanel", "-lncurses", "_curses_panel.c"],
        "_datetime": ["_datetimemodule.c"],
        "_dbm": ["_dbmmodule.c", "-lgdbm_compat", "-DUSE_GDBM_COMPAT"],
        "_decimal": ["_decimal/_decimal.c", "-DCONFIG_64=1"],
        "_elementtree": ["_elementtree.c"],
        "_functools": [
            "-DPy_BUILD_CORE_BUILTIN",
            "-I$(srcdir)/Include/internal",
            "_functoolsmodule.c",
        ],
        "_gdbm": ["_gdbmmodule.c", "-lgdbm"],
        "_hashlib": [
            "_hashopenssl.c",
            "-I$(OPENSSL)/include",
            "-L$(OPENSSL)/lib",
            "$(OPENSSL)/lib/libcrypto.a",
        ],
        "_heapq": ["_heapqmodule.c"],
        "_io": [
            "_io/_iomodule.c",
            "_io/iobase.c",
            "_io/fileio.c",
            "_io/bytesio.c",
            "_io/bufferedio.c",
            "_io/textio.c",
            "_io/stringio.c",
        ],
        "_json": ["_json.c"],
        "_locale": ["-DPy_BUILD_CORE_BUILTIN", "_localemodule.c"],
        "_lsprof": ["_lsprof.c", "rotatingtree.c"],
        "_lzma": [
            "_lzmamodule.c",
            "-I$(LZMA)/include",
            "-L$(LZMA)/lib",
            "$(LZMA)/lib/liblzma.a",
        ],
        "_md5": ["md5module.c"],
        "_multibytecodec": ["cjkcodecs/multibytecodec.c"],
        "_multiprocessing": [
            "_multiprocessing/multiprocessing.c",
            "_multiprocessing/semaphore.c",
        ],
        "_opcode": ["_opcode.c"],
        "_operator": ["_operator.c"],
        "_pickle": ["_pickle.c"],
        "_posixshmem": ["_multiprocessing/posixshmem.c"],
        "_posixsubprocess": ["_posixsubprocess.c"],
        "_queue": ["_queuemodule.c"],
        "_random": ["_randommodule.c"],
        "_scproxy": ["_scproxy.c"],
        "_sha1": ["sha1module.c"],
        "_sha256": ["sha256module.c"],
        "_sha3": ["_sha3/sha3module.c"],
        "_sha512": ["sha512module.c"],
        "_signal": [
            "-DPy_BUILD_CORE_BUILTIN",
            "-I$(srcdir)/Include/internal",
            "signalmodule.c",
        ],
        "_socket": ["socketmodule.c"],
        "_sqlite3": [
            "_sqlite/blob.c",
            "_sqlite/connection.c",
            "_sqlite/cursor.c",
            "_sqlite/microprotocols.c",
            "_sqlite/module.c",
            "_sqlite/prepare_protocol.c",
            "_sqlite/row.c",
            "_sqlite/statement.c",
            "_sqlite/util.c",
        ],
        "_sre": ["_sre/sre.c", "-DPy_BUILD_CORE_BUILTIN"],
        "_ssl": [
            "_ssl.c",
            "-I$(OPENSSL)/include",
            "-L$(OPENSSL)/lib",
            "$(OPENSSL)/lib/libcrypto.a",
            "$(OPENSSL)/lib/libssl.a",
        ],
        "_stat": ["_stat.c"],
        "_statistics": ["_statisticsmodule.c"],
        "_struct": ["_struct.c"],
        "_symtable": ["symtablemodule.c"],
        "_thread": [
            "-DPy_BUILD_CORE_BUILTIN",
            "-I$(srcdir)/Include/internal",
            "_threadmodule.c",
        ],
        "_tracemalloc": ["_tracemalloc.c"],
        "_typing": ["_typingmodule.c"],
        "_uuid": ["_uuidmodule.c"],
        "_weakref": ["_weakref.c"],
        "_zoneinfo": ["_zoneinfo.c"],
        "array": ["arraymodule.c"],
        "atexit": ["atexitmodule.c"],
        "binascii": ["binascii.c"],
        "cmath": ["cmathmodule.c"],
        "errno": ["errnomodule.c"],
        "faulthandler": ["faulthandler.c"],
        "fcntl": ["fcntlmodule.c"],
        "grp": ["grpmodule.c"],
        "itertools": ["itertoolsmodule.c"],
        "math": ["mathmodule.c"],
        "mmap": ["mmapmodule.c"],
        "ossaudiodev": ["ossaudiodev.c"],
        "posix": [
            "-DPy_BUILD_CORE_BUILTIN",
            "-I$(srcdir)/Include/internal",
            "posixmodule.c",
        ],
        "pwd": ["pwdmodule.c"],
        "pyexpat": [
            "expat/xmlparse.c",
            "expat/xmlrole.c",
            "expat/xmltok.c",
            "pyexpat.c",
            "-I$(srcdir)/Modules/expat",
            "-DHAVE_EXPAT_CONFIG_H",
            "-DUSE_PYEXPAT_CAPI",
            "-DXML_DEV_URANDOM",
        ],
        "readline": ["readline.c", "-lreadline", "-ltermcap"],
        "resource": ["resource.c"],
        "select": ["selectmodule.c"],
        "spwd": ["spwdmodule.c"],
        "syslog": ["syslogmodule.c"],
        "termios": ["termios.c"],
        "time": [
            "-DPy_BUILD_CORE_BUILTIN",
            "-I$(srcdir)/Include/internal",
            "timemodule.c",
        ],
        "unicodedata": ["unicodedata.c"],
        "zlib": ["zlibmodule.c", "-lz"],
    },
    "core": [
        "_abc",
        "_codecs",
        "_collections",
        "_functools",
        "_io",
        "_locale",
        "_operator",
        "_signal",
        "_sre",
        "_stat",
        "_symtable",
        "_thread",
        "_tracemalloc",
        "_weakref",
        "atexit",
        "errno",
        "faulthandler",
        "itertools",
        "posix",
        "pwd",
        "time",
    ],
    "shared": [],
    "static": [
        "_asyncio",
        "_bisect",
        "_blake2",
        "_bz2",
        "_contextvars",
        "_csv",
        "_datetime",
        "_decimal",
        "_elementtree",
        "_hashlib",
        "_heapq",
        "_json",
        "_lsprof",
        "_lzma",
        "_md5",
        "_multibytecodec",
        "_multiprocessing",
        "_opcode",
        "_pickle",
        "_posixshmem",
        "_posixsubprocess",
        "_queue",
        "_random",
        "_sha1",
        "_sha256",
        "_sha3",
        "_sha512",
        "_socket",
        "_sqlite3",
        "_ssl",
        "_statistics",
        "_struct",
        "_typing",
        "_uuid",
        "_zoneinfo",
        "array",
        "binascii",
        "cmath",
        "fcntl",
        "grp",
        "math",
        "mmap",
        "pyexpat",
        "readline",
        "select",
        "unicodedata",
        "zlib",
    ],
    "disabled": [
        "_codecs_cn",
        "_codecs_hk",
        "_codecs_iso2022",
        "_codecs_jp",
        "_codecs_kr",
        "_codecs_tw",
        "_crypt",
        "_ctypes",
        "_curses",
        "_curses_panel",
        "_dbm",
        "_scproxy",
        "_tkinter",
        "_xxsubinterpreters",
        "audioop",
        "nis",
        "ossaudiodev",
        "resource",
        "spwd",
        "syslog",
        "termios",
        "xxlimited",
        "xxlimited_35",
    ],
}


class Config:
    """Abstract configuration class"""

    version: str
    log: logging.Logger

    def __init__(self, cfg: dict):
        self.cfg = cfg.copy()
        self.out = ["# -*- makefile -*-"] + self.cfg["header"] + ["\n# core\n"]
        self.log = logging.getLogger(self.__class__.__name__)
        self.patch()

    def __repr__(self):
        return f"<{self.__class__.__name__} '{self.version}'>"

    def patch(self) -> None:
        """patch cfg attribute"""

    def move_entries(self, src: str, dst: str, *names):
        """generic entry mover"""
        for name in names:
            self.log.info("%s -> %s: %s", src, dst, name)
            self.cfg[src].remove(name)
            self.cfg[dst].append(name)

    def enable_static(self, *names):
        """move disabled entries to static"""
        self.move_entries("disabled", "static", *names)

    def enable_shared(self, *names):
        """move disabled entries to shared"""
        self.move_entries("disabled", "shared", *names)

    def disable_static(self, *names):
        """move static entries to disabled"""
        self.move_entries("static", "disabled", *names)

    def disable_shared(self, *names):
        """move shared entries to disabled"""
        self.move_entries("shared", "disabled", *names)

    def move_static_to_shared(self, *names):
        """move static entries to shared"""
        self.move_entries("static", "shared", *names)

    def move_shared_to_static(self, *names):
        """move shared entries to static"""
        self.move_entries("shared", "static", *names)

    def write(self, method: str, to: Pathlike):
        """write configuration method to a file"""

        def _add_section(name):
            if self.cfg[name]:
                self.out.append(f"\n*{name}*\n")
                for i in sorted(self.cfg[name]):
                    if name == "disabled":
                        line = [i]
                    else:
                        ext = self.cfg["extensions"][i]
                        line = [i] + ext
                    self.out.append(" ".join(line))

        self.log.info("write method '%s' to %s", method, to)
        getattr(self, method)()
        for i in self.cfg["core"]:
            ext = self.cfg["extensions"][i]
            line = [i] + ext
            self.out.append(" ".join(line))
        for section in ["shared", "static", "disabled"]:
            _add_section(section)

        with open(to, "w", encoding='utf8') as f:
            self.out.append("# end \n")
            f.write("\n".join(self.out))

    def write_json(self, method: str, to: Pathlike):
        self.log.info("write method '%s' to json: %s", method, to)
        getattr(self, method)()
        with open(to, 'w') as f:
            json.dump(self.cfg, f, indent=4)


class PythonConfig311(Config):
    """configuration class to build python 3.11"""

    version: str = "3.11.10"

    def patch(self) -> None:
        """patch cfg attribute"""
        if PLATFORM == "Darwin":
            self.enable_static("_scproxy")
        elif PLATFORM == "Linux":
            self.enable_static("ossaudiodev")

    def static_max(self):
        """static build variant max-size"""

    def static_mid(self):
        """static build variant mid-size"""
        self.disable_static("_decimal")
        if PLATFORM == "Linux":
            self.cfg["extensions"]["_ssl"] = [
                "_ssl.c",
                "-I$(OPENSSL)/include",
                "-L$(OPENSSL)/lib",
                "-l:libssl.a -Wl,--exclude-libs,libssl.a",
                "-l:libcrypto.a -Wl,--exclude-libs,libcrypto.a",
            ]
            self.cfg["extensions"]["_hashlib"] = [
                "_hashopenssl.c",
                "-I$(OPENSSL)/include",
                "-L$(OPENSSL)/lib",
                "-l:libcrypto.a -Wl,--exclude-libs,libcrypto.a",
            ]

    def static_tiny(self):
        """static build variant tiny-size"""
        self.disable_static(
            "_bz2",
            "_decimal",
            "_csv",
            "_json",
            "_lzma",
            "_scproxy",
            "_sqlite3",
            "_ssl",
            "pyexpat",
            "readline",
        )

    def static_bootstrap(self):
        """static build variant bootstrap-size"""
        for i in self.cfg["static"]:
            self.cfg["disabled"].append(i)
        self.cfg["static"] = self.cfg["core"].copy()
        self.cfg["core"] = []

    def shared_max(self):
        """shared build variant max-size"""
        self.cfg["disabled"].remove("_ctypes")
        self.move_static_to_shared("_decimal", "_ssl", "_hashlib")

    def shared_mid(self):
        """shared build variant mid-size"""
        self.disable_static("_decimal", "_ssl", "_hashlib")

    def framework_max(self):
        """framework build variant max-size"""
        self.shared_max()
        self.move_static_to_shared(
            "_bz2", "_lzma", "readline", "_sqlite3",
            "_scproxy", "zlib", "binascii",
        )

    def framework_mid(self):
        """framework build variant mid-size"""
        self.framework_max()
        self.disable_shared("_decimal", "_ssl", "_hashlib")


class PythonConfig312(PythonConfig311):
    """configuration class to build python 3.12"""

    version = "3.12.7"

    def patch(self):
        """patch cfg attribute"""

        super().patch()
        # if PLATFORM == "Darwin":
        #     self.enable_static("_scproxy")
        # elif PLATFORM == "Linux":
        #     self.enable_static("ossaudiodev")

        self.cfg["extensions"].update(
            {
                "_md5": [
                    "md5module.c",
                    "-I$(srcdir)/Modules/_hacl/include",
                    "_hacl/Hacl_Hash_MD5.c",
                    "-D_BSD_SOURCE",
                    "-D_DEFAULT_SOURCE",
                ],
                "_sha1": [
                    "sha1module.c",
                    "-I$(srcdir)/Modules/_hacl/include",
                    "_hacl/Hacl_Hash_SHA1.c",
                    "-D_BSD_SOURCE",
                    "-D_DEFAULT_SOURCE",
                ],
                "_sha2": [
                    "sha2module.c",
                    "-I$(srcdir)/Modules/_hacl/include",
                    "_hacl/Hacl_Hash_SHA2.c",
                    "-D_BSD_SOURCE",
                    "-D_DEFAULT_SOURCE",
                    # "Modules/_hacl/libHacl_Hash_SHA2.a",
                ],
                "_sha3": [
                    "sha3module.c",
                    "-I$(srcdir)/Modules/_hacl/include",
                    "_hacl/Hacl_Hash_SHA3.c",
                    "-D_BSD_SOURCE",
                    "-D_DEFAULT_SOURCE",
                ],
            }
        )
        del self.cfg["extensions"]["_sha256"]
        del self.cfg["extensions"]["_sha512"]
        self.cfg["static"].append("_sha2")
        self.cfg["static"].remove("_sha256")
        self.cfg["static"].remove("_sha512")
        self.cfg["disabled"].append("_xxinterpchannels")


class PythonConfig313(PythonConfig312):
    """configuration class to build python 3.13"""

    version = "3.13.0"

    def patch(self):
        """patch cfg attribute"""

        super().patch()

        self.cfg["extensions"].update(
            {
                "_interpchannels" : ["_interpchannelsmodule.c"],
                "_interpqueues" : ["_interpqueuesmodule.c"],
                "_interpreters" : ["_interpretersmodule.c"],
                "_sysconfig" : ["_sysconfig.c"],
                "_testexternalinspection" : ["_testexternalinspection.c"],
            })

        del self.cfg["extensions"]["_crypt"]
        del self.cfg["extensions"]["ossaudiodev"]
        del self.cfg["extensions"]["spwd"]

        self.cfg["static"].append("_interpchannels")
        self.cfg["static"].append("_interpqueues")
        self.cfg["static"].append("_interpreters")
        self.cfg["static"].append("_sysconfig")

        self.cfg["disabled"].remove("_crypt")
        self.cfg["disabled"].remove("_xxsubinterpreters")
        self.cfg["disabled"].remove("audioop")
        self.cfg["disabled"].remove("nis")
        self.cfg["disabled"].remove("ossaudiodev")
        self.cfg["disabled"].remove("spwd")

        self.cfg["disabled"].append("_testexternalinspection")



# ----------------------------------------------------------------------------
# utility classes

class ShellCmd:
    """Provides platform agnostic file/folder handling."""

    log: logging.Logger

    def cmd(self, shellcmd: str, cwd: Pathlike = "."):
        """Run shell command within working directory"""
        self.log.info(shellcmd)
        try:
            subprocess.check_call(shellcmd, shell=True, cwd=str(cwd))
        except subprocess.CalledProcessError:
            self.log.critical("", exc_info=True)
            sys.exit(1)

    def download(self, url: str, tofolder: Optional[Pathlike] = None) -> Pathlike:
        """Download a file from a url to an optional folder"""
        _path = Path(os.path.basename(url))
        if tofolder:
            _path = Path(tofolder).joinpath(_path)
            if _path.exists():
                return _path
        filename, _ = urlretrieve(url, filename=_path)
        return Path(filename)

    def extract(self, archive: Pathlike, tofolder: Pathlike = "."):
        """extract a tar archive"""
        if tarfile.is_tarfile(archive):
            with tarfile.open(archive) as f:
                f.extractall(tofolder)
        # elif zipfile.is_zipfile(archive):
        #     with zipfile.ZipFile(archive) as f:
        #         f.extractall(tofolder)
        else:
            raise TypeError("cannot extract from this file.")

    def fail(self, msg, *args):
        """exits the program with an error msg."""
        self.log.critical(msg, *args)
        sys.exit(1)

    def git_clone(
        self,
        url: str,
        branch: Optional[str] = None,
        directory: Optional[Pathlike] = None,
        recurse: bool = False,
        cwd: Pathlike = ".",
    ):
        """git clone a repository source tree from a url"""
        _cmds = ["git clone --depth 1"]
        if branch:
            _cmds.append(f"--branch {branch}")
        if recurse:
            _cmds.append("--recurse-submodules --shallow-submodules")
        _cmds.append(url)
        if directory:
            _cmds.append(str(directory))
        self.cmd(" ".join(_cmds), cwd=cwd)

    def getenv(self, key: str, default: bool = False) -> bool:
        """convert '0','1' env values to bool {True, False}"""
        self.log.info("checking env variable: %s", key)
        return bool(int(os.getenv(key, default)))

    def chdir(self, path: Pathlike):
        """Change current workding directory to path"""
        self.log.info("changing working dir to: %s", path)
        os.chdir(path)

    def chmod(self, path: Pathlike, perm=0o777):
        """Change permission of file"""
        self.log.info("change permission of %s to %s", path, perm)
        os.chmod(path, perm)

    def get(self, shellcmd, cwd: Pathlike = ".", shell: bool = False) -> str:
        """get output of shellcmd"""
        if not shell:
            shellcmd = shellcmd.split()
        return subprocess.check_output(
            shellcmd, encoding="utf8", shell=shell, cwd=str(cwd)
        ).strip()

    def makedirs(self, path: Pathlike, mode: int = 511, exist_ok: bool = True):
        """Recursive directory creation function"""
        self.log.info("making directory: %s", path)
        os.makedirs(path, mode, exist_ok)

    def move(self, src: Pathlike, dst: Pathlike):
        """Move from src path to dst path."""
        self.log.info("move path %s to %s", src, dst)
        shutil.move(src, dst)

    def copy(self, src: Pathlike, dst: Pathlike):
        """copy file or folders -- tries to be behave like `cp -rf`"""
        self.log.info("copy %s to %s", src, dst)
        src, dst = Path(src), Path(dst)
        if src.is_dir():
            shutil.copytree(src, dst)
        else:
            shutil.copy2(src, dst)

    def remove(self, path: Pathlike, silent: bool = False):
        """Remove file or folder."""

        # handle windows error on read-only files
        def remove_readonly(func, path, exc_info):
            "Clear the readonly bit and reattempt the removal"
            if PY_VER_MINOR < 11:
                if func not in (os.unlink, os.rmdir) or exc_info[1].winerror != 5:
                    raise exc_info[1]
            else:
                if func not in (os.unlink, os.rmdir) or exc_info.winerror != 5:
                    raise exc_info
            os.chmod(path, stat.S_IWRITE)
            func(path)


        path = Path(path)
        if path.is_dir():
            if not silent:
                self.log.info("remove folder: %s", path)
            if PY_VER_MINOR < 11:
                shutil.rmtree(path, ignore_errors=not DEBUG, onerror=remove_readonly)
            else:
                shutil.rmtree(path, ignore_errors=not DEBUG, onexc=remove_readonly)
        else:
            if not silent:
                self.log.info("remove file: %s", path)
            try:
                path.unlink()
            except FileNotFoundError:
                if not silent:
                    self.log.warning("file not found: %s", path)

    def walk(
        self,
        root: Pathlike,
        match_func: MatchFn,
        action_func: ActionFn,
        skip_patterns: list[str],
    ):
        """general recursive walk from root path with match and action functions"""
        for root_, dirs, filenames in os.walk(root):
            _root = Path(root_)
            if skip_patterns:
                for skip_pat in skip_patterns:
                    if skip_pat in dirs:
                        dirs.remove(skip_pat)
            for _dir in dirs:
                current = _root / _dir
                if match_func(current):
                    action_func(current)

            for _file in filenames:
                current = _root / _file
                if match_func(current):
                    action_func(current)

    def glob_remove(self, root: Pathlike, patterns: list[str], skip_dirs: list[str]):
        """applies recursive glob remove using a list of patterns"""

        def match(entry: Path) -> bool:
            # return any(fnmatch(entry, p) for p in patterns)
            return any(fnmatch(entry.name, p) for p in patterns)

        def remove(entry: Path):
            self.remove(entry)

        self.walk(root, match_func=match, action_func=remove,
                  skip_patterns=skip_dirs)

    def pip_install(
        self,
        *pkgs,
        reqs: Optional[str] = None,
        upgrade: bool = False,
        pip: Optional[str] = None,
    ):
        """Install python packages using pip"""
        _cmds = []
        if pip:
            _cmds.append(pip)
        else:
            _cmds.append("pip3")
        _cmds.append("install")
        if reqs:
            _cmds.append(f"-r {reqs}")
        else:
            if upgrade:
                _cmds.append("--upgrade")
            _cmds.extend(pkgs)
        self.cmd(" ".join(_cmds))

    def apt_install(self, *pkgs, update: bool = False):
        """install debian packages using apt"""
        _cmds = []
        _cmds.append("sudo apt install")
        if update:
            _cmds.append("--upgrade")
        _cmds.extend(pkgs)
        self.cmd(" ".join(_cmds))

    def brew_install(self, *pkgs, update: bool = False):
        """install using homebrew"""
        _pkgs = " ".join(pkgs)
        if update:
            self.cmd("brew update")
        self.cmd(f"brew install {_pkgs}")

    def cmake_config(self, src_dir: Pathlike, build_dir: Pathlike, *scripts, **options):
        """activate cmake configuration / generation stage"""
        _cmds = [f"cmake -S {src_dir} -B {build_dir}"]
        if scripts:
            _cmds.append(" ".join(f"-C {path}" for path in scripts))
        if options:
            _cmds.append(" ".join(f"-D{k}={v}" for k, v in options.items()))
        self.cmd(" ".join(_cmds))

    def cmake_build(self, build_dir: Pathlike, release: bool = False):
        """activate cmake build stage"""
        _cmd = f"cmake --build {build_dir}"
        if release:
            _cmd += " --config Release"
        self.cmd(_cmd)

    def cmake_install(self, build_dir: Pathlike, prefix: Optional[str] = None):
        """activate cmake install stage"""
        _cmds = ["cmake --install", str(build_dir)]
        if prefix:
            _cmds.append(f"--prefix {prefix}")
        self.cmd(" ".join(_cmds))

# ----------------------------------------------------------------------------
# main classes

class Project(ShellCmd):
    """Utility class to hold project directory structure"""

    def __init__(self):
        self.cwd = Path.cwd()
        self.build = self.cwd / "build"
        self.downloads = self.build / "downloads"
        self.src = self.build / "src"
        self.install = self.build / "install"
        self.bin = self.build / "bin"
        self.lib = self.build / "lib"
        self.lib_static = self.build / "lib" / "static"

    def setup(self):
        """create main project directories"""
        self.build.mkdir(exist_ok=True)
        self.downloads.mkdir(exist_ok=True)
        self.install.mkdir(exist_ok=True)
        self.src.mkdir(exist_ok=True)

    def reset(self):
        """prepare project for a rebuild"""
        self.remove(self.src)
        self.remove(self.install / "python")


class AbstractBuilder(ShellCmd):
    """Abstract builder class with additional methods common to subclasses."""

    name: str
    version: str
    repo_url: str
    download_url_template: str
    libs_static: list[str]
    depends_on: list[type["Builder"]]

    def __init__(self, version: Optional[str] = None, project: Optional[Project] = None):
        self.version = version or self.version
        self.project = project or Project()
        self.log = logging.getLogger(self.__class__.__name__)

    def __repr__(self):
        return f"<{self.__class__.__name__} '{self.name}-{self.version}'>"

    # def __iter__(self):
    #     for dependency in self.depends_on:
    #         yield dependency
    #         yield from iter(dependency)

    @property
    def ver(self) -> str:
        """short python version: 3.11"""
        return ".".join(self.version.split(".")[:2])

    @property
    def ver_major(self) -> str:
        """major compoent of semantic version: 3 in 3.11.7"""
        return self.version.split(".")[0]

    @property
    def ver_minor(self) -> str:
        """minor compoent of semantic version: 11 in 3.11.7"""
        return self.version.split(".")[1]

    @property
    def ver_patch(self) -> str:
        """patch compoent of semantic version: 7 in 3.11.7"""
        return self.version.split(".")[2]

    @property
    def ver_nodot(self) -> str:
        """concat major and minor version components: 311 in 3.11.7"""
        return self.ver.replace(".", "")

    @property
    def name_version(self) -> str:
        """return name-<fullversion>: e.g. Python-3.11.7"""
        return f"{self.name}-{self.version}"

    @property
    def name_ver(self) -> str:
        """return name.lower-<ver>: e.g. python3.11"""
        return f"{self.name.lower()}{self.ver}"

    @property
    def download_url(self) -> str:
        """return download url with version interpolated"""
        return self.download_url_template.format(ver=self.version)

    @property
    def repo_branch(self) -> str:
        """return repo branch"""
        return self.name.lower()

    @property
    def src_dir(self) -> Path:
        """return extracted source folder of build target"""
        return self.project.src / self.name_version

    @property
    def build_dir(self) -> Path:
        """return 'build' folder src dir of build target"""
        return self.src_dir / "build"

    @property
    def executable_name(self) -> str:
        """executable name of buld target"""
        name = self.name.lower()
        if PLATFORM == "Windows":
            name = f"{self.name}.exe"
        return name

    @property
    def executable(self) -> Path:
        """executable path of buld target"""
        return self.project.bin / self.executable_name

    @property
    def libname(self) -> str:
        """library name prefix"""
        return f"lib{self.name}"

    @property
    def staticlib_name(self) -> str:
        """static libname"""
        suffix = ".a"
        if PLATFORM == "Windows":
            suffix = ".lib"
        return f"{self.libname}{suffix}"

    @property
    def dylib_name(self) -> str:
        """dynamic link libname"""
        if PLATFORM == "Darwin":
            return f"{self.libname}.dylib"
        if PLATFORM == "Linux":
            return f"{self.libname}.so"
        if PLATFORM == "Windows":
            return f"{self.libname}.dll"
        return self.fail("platform not supported")

    @property
    def dylib_linkname(self) -> str:
        """symlink to dylib"""
        if PLATFORM == "Darwin":
            return f"{self.libname}.dylib"
        if PLATFORM == "Linux":
            return f"{self.libname}.so"
        return self.fail("platform not supported")

    @property
    def dylib(self) -> Path:
        """dylib path"""
        return self.project.lib / self.dylib_name

    @property
    def dylib_link(self) -> Path:
        """dylib link path"""
        return self.project.lib / self.dylib_linkname

    @property
    def staticlib(self) -> Path:
        """staticlib path"""
        return self.project.lib_static / self.staticlib_name

    @property
    def prefix(self) -> Path:
        """builder prefix path"""
        return self.project.install / self.name.lower()

    def libs_static_exist(self) -> bool:
        """check if all built stati libs already exist"""
        return all((self.prefix / "lib" / lib).exists()
                    for lib in self.libs_static)

    def pre_process(self):
        """override by subclass if needed"""

    def setup(self):
        """setup build environment"""

    def configure(self):
        """configure build"""

    def build(self):
        """build target"""

    def install(self):
        """install target"""

    def clean(self):
        """clean build"""

    def post_process(self):
        """override by subclass if needed"""

    def process(self):
        """main builder process"""
        self.pre_process()
        self.setup()
        self.configure()
        self.build()
        self.install()
        self.clean()
        self.post_process()


class Builder(AbstractBuilder):
    """concrete builder class"""

    def setup(self):
        """setup build environment"""
        self.project.setup()
        archive = self.download(self.download_url, tofolder=self.project.downloads)
        self.log.info("downloaded %s", archive)
        if not self.src_dir.exists():
            self.extract(archive, tofolder=self.project.src)
            assert self.src_dir.exists(), f"could not extract from {archive}"


class OpensslBuilder(Builder):
    """ssl builder class"""

    name = "openssl"
    version = "1.1.1w"
    repo_url = "https://github.com/openssl/openssl.git"
    download_url_template = "https://www.openssl.org/source/old/1.1.1/openssl-{ver}.tar.gz"
    depends_on = []
    libs_static = ["libssl.a", "libcrypto.a"]

    def build(self):
        """main build method"""
        if not self.libs_static_exist():
            self.cmd(
                f"./config no-shared no-tests --prefix={self.prefix}",
                cwd=self.src_dir
            )
            self.cmd("make install_sw", cwd=self.src_dir)


class Bzip2Builder(Builder):
    """bz2 builder class"""

    name = "bzip2"
    version = "1.0.8"
    repo_url = "https://github.com/libarchive/bzip2.git"
    download_url_template = "https://sourceware.org/pub/bzip2/bzip2-{ver}.tar.gz"
    depends_on = []
    libs_static = ["libbz2.a"]

    def build(self):
        """main build method"""
        if not self.libs_static_exist():
            cflags = "-fPIC"
            self.cmd(
                f"make install PREFIX={self.prefix} CFLAGS='{cflags}'",
                cwd=self.src_dir,
            )


class XzBuilder(Builder):
    """lzma builder class"""

    name = "xz"
    version = "5.6.3"
    repo_url = "https://github.com/python/cpython-source-deps.git"
    download_url_template = "http://tukaani.org/xz/xz-{ver}.tar.gz" # not used
    depends_on = []
    libs_static = ["liblzma.a"]

    @property
    def repo_branch(self):
        """branch of git repository to use"""
        return self.name

    def setup(self):
        """setup build environment"""
        self.project.setup()
        if not self.src_dir.exists():
            self.git_clone(self.repo_url, self.repo_branch, self.src_dir)
            assert self.src_dir.exists(), f"could not git clone {self.download_url}"

    def build(self):
        """main build method"""
        if not self.libs_static_exist():
            configure = self.src_dir / "configure"
            install_sh = self.src_dir / "build-aux" / "install-sh"
            for f in [configure, install_sh]:
                self.chmod(f, 0o755)
            self.cmd(
                " ".join([
                    "/bin/sh",
                    "configure",
                    "--disable-dependency-tracking",
                    "--disable-xzdec",
                    "--disable-lzmadec",
                    "--disable-nls",
                    "--enable-small",
                    "--disable-shared",
                    f"--prefix={self.prefix}",
                    ]),
                cwd=self.src_dir,
            )
            self.cmd("make && make install", cwd=self.src_dir)


class PythonBuilder(Builder):
    """Builds python locally"""

    name = "Python"
    version = DEFAULT_PY_VERSION
    repo_url = "https://github.com/python/cpython.git"
    download_url_template = "https://www.python.org/ftp/python/{ver}/Python-{ver}.tar.xz"

    config_options: list[str] = [
        # "--disable-ipv6",
        # "--disable-profiling",
        "--disable-test-modules",
        # "--enable-framework",
        # "--enable-framework=INSTALLDIR",
        # "--enable-optimizations",
        # "--enable-shared",
        # "--enable-universalsdk",
        # "--enable-universalsdk=SDKDIR",
        # "--enable-loadable-sqlite-extensions",
        # "--with-lto",
        # "--with-lto=thin",
        # "--with-openssl-rpath=auto",
        # "--with-openssl=DIR",
        # "--with-readline=editline",
        # "--with-system-expat",
        # "--with-system-ffi",
        # "--with-system-libmpdec",
        # "--without-builtin-hashlib-hashes",
        # "--without-doc-strings",
        # "--without-ensurepip",
        # "--without-readline",
        # "--without-static-libpython",
    ]

    required_packages: list[str] = []

    remove_patterns: list[str] = [
        "*.exe",
        "*config-3*",
        "*tcl*",
        "*tdbc*",
        "*tk*",
        "__phello__",
        "__pycache__",
        "_codecs_*.so",
        "_test*",
        "_tk*",
        "_xx*.so",
        "distutils",
        "idlelib",
        "lib2to3",
        "libpython*",
        "LICENSE.txt",
        "pkgconfig",
        "pydoc_data",
        "site-packages",
        "test",
        "Tk*",
        "turtle*",
        "venv",
        "xx*.so",
    ]

    depends_on = [OpensslBuilder, Bzip2Builder, XzBuilder]

    def __init__(
        self,
        version: str = DEFAULT_PY_VERSION,
        project: Optional[Project] = None,
        config: str = "shared_max",
        optimize: bool = False,
        pkgs: Optional[list[str]] = None,
        cfg_opts: Optional[list[str]] = None,
        jobs: int = 1,
    ):

        super().__init__(version, project)
        self.config = config
        self.optimize = optimize
        self.pkgs = pkgs or []
        self.cfg_opts = cfg_opts or []
        self.jobs = jobs
        self.log = logging.getLogger(self.__class__.__name__)

    def get_config(self):
        """get configuration class for required python version"""
        return {
            "3.11": PythonConfig311,
            "3.12": PythonConfig312,
            "3.13": PythonConfig313,
        }[self.ver](BASE_CONFIG)

    @property
    def libname(self):
        """library name suffix"""
        return f"lib{self.name_ver}"

    @property
    def build_type(self):
        """build type: 'static', 'shared' or 'framework'"""
        return self.config.split("_")[0]

    @property
    def size_type(self):
        """size qualifier: 'max', 'mid', 'min', etc.."""
        return self.config.split("_")[1]

    @property
    def prefix(self):
        """python builder prefix path"""
        if PLATFORM == "Darwin" and self.build_type == "framework":
            return self.project.install / "Python.framework" / "Versions" / self.ver
        name = self.name.lower() + "-" + self.build_type
        return self.project.install / name

    @property
    def python(self):
        """path to python3 executable"""
        return self.prefix / "bin" / "python3"

    @property
    def pip(self):
        """path to pip3 executable"""
        return self.prefix / "bin" / "pip3"

    def pre_process(self):
        """override by subclass if needed"""

    def configure(self):
        """configure build"""
        config = self.get_config()
        prefix = self.prefix

        if self.build_type == "shared":
            self.config_options.extend([
                "--enable-shared", "--without-static-libpython"])
        elif self.build_type == "framework":
            prefix = self.project.install
            self.config_options.append(f"--enable-framework={prefix}")
        elif self.build_type != "static":
            self.fail(f"{self.build_type} not recognized build type")

        if self.optimize:
            self.config_options.append("--enable-optimizations")

        if not self.pkgs and not self.required_packages:
            self.config_options.append("--without-ensurepip")
            self.remove_patterns.append("ensurepip")
        else:
            self.pkgs.extend(self.required_packages)

        if self.cfg_opts:
            for cfg_opt in self.cfg_opts:
                cfg_opt = cfg_opt.replace('_', '-')
                cfg_opt = '--' + cfg_opt
                if cfg_opt not in self.config_options:
                     self.config_options.append(cfg_opt)

        config.write(self.config, to=self.src_dir / "Modules" / "Setup.local")
        config_opts = " ".join(self.config_options)
        self.cmd(f"./configure --prefix={prefix} {config_opts}",
                 cwd=self.src_dir)

    def build(self):
        """main build process"""
        self.cmd(f"make -j{self.jobs}", cwd=self.src_dir)

    def install(self):
        """install to prefix"""
        if self.prefix.exists():
            self.remove(self.prefix)
        self.cmd("make install", cwd=self.src_dir)

    def clean(self):
        """clean installed build"""
        self.glob_remove(
            self.prefix / "lib" / self.name_ver,
            self.remove_patterns,
            skip_dirs=[".git"],
        )

        bins = [
            "2to3",
            "idle3",
            f"idle{self.ver}",
            "pydoc3",
            f"pydoc{self.ver}",
            f"2to3-{self.ver}",
        ]
        for executable in bins:
            self.remove(self.prefix / "bin" / executable)

    def ziplib(self):
        """zip python site-packages"""

        src = self.prefix / "lib" / self.name_ver

        self.move(
            src / "lib-dynload",
            self.project.build / "lib-dynload",
        )
        self.move(src / "os.py", self.project.build / "os.py")

        zip_path = self.prefix / "lib" / f"python{self.ver_nodot}"
        shutil.make_archive(str(zip_path), "zip", str(src))
        self.remove(src)

        site_packages = src / "site-packages"
        self.remove(self.prefix / "lib" / "pkgconfig")
        src.mkdir()
        site_packages.mkdir()
        self.move(self.project.build / "lib-dynload", src / "lib-dynload")
        self.move(self.project.build / "os.py", src / "os.py")

    def install_pkgs(self):
        """install python packages"""
        required_pkgs = " ".join(self.required_packages)
        self.cmd(f"{self.python} -m ensurepip")
        self.cmd(f"{self.pip} install {required_pkgs}")

    def make_relocatable(self):
        """fix dylib/exe @rpath shared buildtype in macos"""
        if PLATFORM == "Darwin":
            if self.build_type == "shared":
                dylib = self.prefix / "lib" / self.dylib_name
                self.chmod(dylib)
                # self.cmd(f"install_name_tool -id @rpath/{self.dylib_name} {dylib}")
                self.cmd(f"install_name_tool -id @loader_path/../Resources/lib/{self.dylib_name} {dylib}")
                to = f"@executable_path/../lib/{self.dylib_name}"
                exe = self.prefix / "bin" / self.name_ver
                self.cmd(f"install_name_tool -change {dylib} {to} {exe}")
            elif self.build_type == "framework":
                dylib = self.prefix / self.name
                self.chmod(dylib)
                self.cmd(f"install_name_tool -id @loader_path/../Resources/Python.framework/Versions/{self.name_ver}/{self.name} {dylib}")
                # self.cmd(f"install_name_tool -id @rpath/{self.name} {dylib}")
                to = f"@executable_path/../{self.name}"
                exe = self.prefix / "bin" / self.name_ver
                self.cmd(f"install_name_tool -change {dylib} {to} {exe}")
        elif PLATFORM == "Linux":
            if self.build_type == "shared":
                exe = self.prefix / "bin" / self.name_ver
                self.cmd(f"patchelf --set-rpath '$ORIGIN'/../lib {exe}")

    def post_process(self):
        """override by subclass if needed"""
        if self.build_type in ["shared", "framework"]:
            self.make_relocatable()
        self.log.info("%s DONE", self.config)


    def process(self):
        """main builder process"""
        for dependency_class in self.depends_on:
            dependency_class().process()
        self.pre_process()
        self.setup()
        self.configure()
        self.build()
        self.install()
        self.clean()
        self.ziplib()
        if self.pkgs:
            self.install_pkgs()
        self.post_process()


class PythonDebugBuilder(PythonBuilder):
    """Builds debug python locally"""

    name = "python"

    config_options = [
        "--disable-test-modules",
        "--without-static-libpython",
        "--with-pydebug",
        # "--with-trace-refs",
        # "--with-valgrind",
        # "--with-address-sanitizer",
        # "--with-memory-sanitizer",
        # "--with-undefined-behavior-sanitizer",
    ]

    required_packages = []


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        prog="buildpy.py",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="A python builder",
    )
    opt = parser.add_argument

    opt("-a", "--cfg-opts", help="add config options", type=str, nargs="+", metavar="CFG")
    opt("-c", "--config", default="shared_mid", help="build configuration (default: %(default)s)", metavar="NAME")
    opt("-d", "--debug", help="build debug python", action="store_true")
    opt("-o", "--optimize", help="optimize build", action="store_true")
    opt("-p", "--pkgs", help="install pkgs", type=str, nargs="+", metavar="PKG")
    opt("-r", "--reset", help="reset build", action="store_true")
    opt("-v", "--version", default=DEFAULT_PY_VERSION, help="python version (default: %(default)s)")
    opt("-w", "--write", help="write configuration", action="store_true")
    opt("-j", "--jobs", help="# of build jobs (default: %(default)s)", type=int, default=4)
    opt("-s", "--json", help="serialize config to json file")

    args = parser.parse_args()
    python_builder_class: type[PythonBuilder] = PythonBuilder
    if args.debug:
        python_builder_class = PythonDebugBuilder
    builder = python_builder_class(
        version=args.version,
        config=args.config,
        optimize=args.optimize,
        pkgs=args.pkgs,
        cfg_opts=args.cfg_opts,
        jobs=args.jobs,
    )
    if args.write:
        if not args.json:
            patch_dir = Path.cwd() / "patch"
            if not patch_dir.exists():
                patch_dir.mkdir()
            cfg_file = patch_dir / args.config.replace("_", ".")
            builder.get_config().write(args.config, to=cfg_file)
        else:
            builder.get_config().write_json(args.config, to=args.json)
            sys.exit()

    if args.reset:
        builder.remove("build")
    builder.process()
