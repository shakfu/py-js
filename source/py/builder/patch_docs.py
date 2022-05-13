"""
Each line in this file describes one or more optional modules.
Modules configured here will not be compiled by the setup.py script,
so the file can be used to override setup.py's behavior.
Tag lines containing just the word "*static*", "*shared*" or "*disabled*"
(without the quotes but with the stars) are used to tag the following module
descriptions. Tag lines may alternate throughout this file.  Modules are
built statically when they are preceded by a "*static*" tag line or when
there is no tag line between the start of the file and the module
description.  Modules are built as a shared library when they are preceded by
a "*shared*" tag line.  Modules are not built at all, not by the Makefile,
nor by the setup.py script, when they are preceded by a "*disabled*" tag
line.

Lines have the following structure:

<module> ... [<sourcefile> ...] [<cpparg> ...] [<library> ...]

<sourcefile> is anything ending in .c (.C, .cc, .c++ are C++ files)
<cpparg> is anything starting with -I, -D, -U or -C
<library> is anything ending in .a or beginning with -l or -L
<module> is anything else but should be a valid Python
identifier (letters, digits, underscores, beginning with non-digit)

(As the makesetup script changes, it may recognize some other
arguments as well, e.g. *.so and *.sl as libraries.  See the big
case statement in the makesetup script.)

Lines can also have the form

<name> = <value>

which defines a Make variable definition inserted into Makefile.in

The build process works like this:

1. Build all modules that are declared as static in Modules/Setup,
   combine them into libpythonxy.a, combine that into python.
2. Build all modules that are listed as shared in Modules/Setup.
3. Invoke setup.py. That builds all modules that
    a) are not builtin, and
    b) are not listed in Modules/Setup, and
    c) can be build on the target

Therefore, modules declared to be shared will not be
included in the config.c file, nor in the list of objects to be
added to the library archive, and their linker options won't be
added to the linker options. Rules to create their .o files and
their shared libraries will still be added to the Makefile, and
their names will be collected in the Make variable SHAREDMODS.  This
is used to build modules as shared libraries.  (They can be
installed using "make sharedinstall", which is implied by the
toplevel "make install" target.)  (For compatibility, *noconfig* 
has the same effect as *shared*.)

NOTE: As a standard policy, as many modules as can be supported by a
platform should be present.  The distribution comes with all modules
enabled that are supported by most platforms and don't require you
to ftp sources from elsewhere.

"""
from typing import List, Tuple, Dict

def split(s, category):
    _list = []
    lines = [line for line in s.splitlines() if line]
    for line in lines:
        xs = line.split()
        _list.append((category, xs[0], xs[1:]))
    return _list


class Module:
    def __init__(self, name: str, mode: Tuple[int,int,int,int], 
                       vers: Dict[str, int], elems: List[str]):
        self.name = name
        self.mode = mode
        self.vers = vers
        self.elems = elems

    def __repr__(self):
        return f"<{self.__class__.__name__} '{self.name}'>"

    def __str__(self):
        return " ".join(self.elems)

    def as_tuple(self):
        m = self
        return (m.name,) + m.mode + tuple(m.vers.values())

# core
# object name          Module definition           0=disabled, 1=static, 2=shared
posix                = Module('posix',             mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'posixmodule.c'])
errno                = Module('errno',             mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['errnomodule.c'])
pwd                  = Module('pwd',               mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['pwdmodule.c'])
_sre                 = Module('_sre',              mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_sre.c'])
_codecs              = Module('_codecs',           mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_codecsmodule.c'])
_weakref             = Module('_weakref',          mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_weakref.c'])
_functools           = Module('_functools',        mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_functoolsmodule.c'])
_operator            = Module('_operator',         mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_operator.c'])
_collections         = Module('_collections',      mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_collectionsmodule.c'])
_abc                 = Module('_abc',              mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_abc.c'])
itertools            = Module('itertools',         mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['itertoolsmodule.c'])
atexit               = Module('atexit',            mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['atexitmodule.c'])
_signal              = Module('_signal',           mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'signalmodule.c'])
_stat                = Module('_stat',             mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_stat.c'])
time                 = Module('time',              mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'timemodule.c'])
_thread              = Module('_thread',           mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_threadmodule.c'])
_locale              = Module('_locale',           mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_localemodule.c'])
_io                  = Module('_io',               mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '-I$(srcdir)/Modules/_io', '_io/_iomodule.c', '_io/iobase.c', '_io/fileio.c', '_io/bytesio.c', '_io/bufferedio.c', '_io/textio.c', '_io/stringio.c'])
faulthandler         = Module('faulthandler',      mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['faulthandler.c'])
zipimport            = Module('zipimport',         mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 0, '3.9': 0, '3.10': 0}, elems=['-DPy_BUILD_CORE', 'zipimport.c'])
_tracemalloc         = Module('_tracemalloc',      mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_tracemalloc.c'])
_peg_parser          = Module('_peg_parser',       mode=(1, 1, 1, 1), vers={'3.7': 0, '3.8': 0, '3.9': 1, '3.10': 1}, elems=['_peg_parser.c'])
_symtable            = Module('_symtable',         mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['symtablemodule.c'])


# The rest of the modules listed in this file are all commented out by
# default.  Usually they can be detected and built as dynamically
# loaded modules by the new setup.py script added in Python 2.1.  If
# you're on a platform that doesn't support dynamic loading, want to
# compile modules statically into the Python binary, or need to
# specify some odd set of compiler switches, you can uncomment the
# appropriate lines below.

# Uncommenting the following line tells makesetup that all following
# modules are to be built as shared libraries (see above for more
# detail; also note that *static* or *disabled* cancels this effect):

#*shared*


# GNU readline.  Unlike previous Python incarnations, GNU readline is
# now incorporated in an optional module, configured in the Setup file
# instead of by a configure script switch.  You may have to insert a
# -L option pointing to the directory where libreadline.* lives,
# and you may have to change -ltermcap to -ltermlib or perhaps remove
# it, depending on your system -- see the GNU readline instructions.
# It's okay for this to be a shared library, too.

readline             = Module('readline',          mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['readline.c', '-lreadline', '-ltermcap'])

# Modules that should always be present (non UNIX dependent):

array                = Module('array',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_MODULE', 'arraymodule.c'])
cmath                = Module('cmath',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cmathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE'])
math                 = Module('math',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['mathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE'])
_contextvars         = Module('_contextvars',      mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_contextvarsmodule.c'])
_struct              = Module('_struct',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_MODULE', '_struct.c'])
_weakref             = Module('_weakref',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_weakref.c'])
_testcapi            = Module('_testcapi',         mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_testcapimodule.c'])
_testinternalcapi    = Module('_testinternalcapi', mode=(0, 0, 0, 0), vers={'3.7': 0, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_testinternalcapi.c', '-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_MODULE'])
_random              = Module('_random',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_randommodule.c', '-DPy_BUILD_CORE_MODULE'])
_elementtree         = Module('_elementtree',      mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-I$(srcdir)/Modules/expat', '-DHAVE_EXPAT_CONFIG_H', '-DUSE_PYEXPAT_CAPI', '_elementtree.c'])
_pickle              = Module('_pickle',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_MODULE', '_pickle.c'])
_datetime            = Module('_datetime',         mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_datetimemodule.c'])
_zoneinfo            = Module('_zoneinfo',         mode=(0, 0, 0, 0), vers={'3.7': 0, '3.8': 0, '3.9': 1, '3.10': 1}, elems=['_zoneinfo.c', '-DPy_BUILD_CORE_MODULE'])
_bisect              = Module('_bisect',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_bisectmodule.c'])
_heapq               = Module('_heapq',            mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_heapqmodule.c', '-DPy_BUILD_CORE_MODULE'])
_asyncio             = Module('_asyncio',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_asynciomodule.c'])
_json                = Module('_json',             mode=(0, 0, 0, 0), vers={'3.7': 0, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_BUILTIN', '_json.c'])
_statistics          = Module('_statistics',       mode=(0, 0, 0, 0), vers={'3.7': 0, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_statisticsmodule.c'])
unicodedata          = Module('unicodedata',       mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['unicodedata.c', '-DPy_BUILD_CORE_BUILTIN'])


# all 4 modules are removed in 3.10
_uuid                = Module('_uuid',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_uuidmodule.c'])
_opcode              = Module('_opcode',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_opcode.c'])
_multiprocessing     = Module('_multiprocessing',  mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_multiprocessing/multiprocessing.c', '_multiprocessing/semaphore.c'])
_posixshmem          = Module('_posixshmem',       mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_multiprocessing/posixshmem.c'])

# Modules with some UNIX dependencies -- on by default:
# (If you have a really backward UNIX, select and socket may not be
# supported...)

fcntl                = Module('fcntl',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['fcntlmodule.c'])
spwd                 = Module('spwd',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['spwdmodule.c'])
grp                  = Module('grp',               mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['grpmodule.c'])
select               = Module('select',            mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['selectmodule.c'])

# Memory-mapped files (also works on Win32).

mmap                 = Module('mmap',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['mmapmodule.c'])

# CSV file helper

_csv                 = Module('_csv',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_csv.c'])



# Socket module helper for socket(2)

_socket              = Module('_socket',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['socketmodule.c'])

# Socket module helper for SSL support; you must comment out the other
# socket line above, and edit the OPENSSL variable:

# OPENSSL=/path/to/openssl/directory
_ssl_shared          = Module('_ssl',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_ssl.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-lssl', '-lcrypto'])
_hashlib_shared      = Module('_hashlib',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_hashopenssl.c.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-lssl', '-lcrypto'])

# To statically link OpenSSL:
_ssl                 = Module('_ssl',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_ssl.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-l:libssl.a', '-Wl,--exclude-libs,libssl.a', '-l:libcrypto.a', '-Wl,--exclude-libs,libcrypto.a'])
_hashlib             = Module('_hashlib',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_hashopenssl.c.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-l:libcrypto.a', '-Wl,--exclude-libs,libcrypto.a'])

# The crypt module is now disabled by default because it breaks builds
# on many systems (where -lcrypt is needed), e.g. Linux (I believe).

_crypt               = Module('_crypt',            mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_cryptmodule.c', '-lcrypt'])


# Some more UNIX dependent modules -- off by default, since these
# are not supported by all UNIX systems:

nis                  = Module('nis',               mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['nismodule.c', '-lnsl'])
termios              = Module('termios',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['termios.c'])
resource             = Module('resource',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['resource.c'])


_posixsubprocess     = Module('_posixsubprocess',  mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_posixsubprocess.c'])

# Multimedia modules -- off by default.
# These don't work for 64-bit platforms!!!
# #993173 says audioop works on 64-bit platforms, though.
# These represent audio samples or images as strings:

audioop              = Module('audioop',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['audioop.c'])

# Note that the _md5 and _sha modules are normally only built if the
# system does not have the OpenSSL libs containing an optimized version.

_md5                 = Module('_md5',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['md5module.c'])
_sha1                = Module('_sha1',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['sha1module.c'])
_sha256              = Module('_sha256',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['sha256module.c', '-DPy_BUILD_CORE_BUILTIN'])
_sha512              = Module('_sha512',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['sha512module.c', '-DPy_BUILD_CORE_BUILTIN'])
_sha3                = Module('_sha3',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_sha3/sha3module.c'])


_blake2              = Module('_blake2',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_blake2/blake2module.c', '_blake2/blake2b_impl.c', '_blake2/blake2s_impl.c'])


# The _tkinter module. (NOT USED)

# Lance Ellinghaus's syslog module
syslog               = Module('syslog',            mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['syslogmodule.c'])


# Curses support, requiring the System V version of curses, often
# provided by the ncurses library.  e.g. on Linux, link with -lncurses
# instead of -lcurses).

_curses              = Module('_curses',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_cursesmodule.c', '-lcurses', '-ltermcap', '-DPy_BUILD_CORE_MODULE'])
# Wrapper for the panel library that's part of ncurses and SYSV curses.
_curses_panel        = Module('_curses_panel',     mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_curses_panel.c', '-lpanel', '-lncurses'])

# Modules that provide persistent dictionary-like semantics.  You will
# probably want to arrange for at least one of them to be available on
# your machine, though none are defined by default because of library
# dependencies.  The Python module dbm/__init__.py provides an
# implementation independent wrapper for these; dbm/dumb.py provides
# similar functionality (but slower of course) implemented in Python.

#_dbm _dbmmodule.c  # dbm(3) may require -lndbm or similar
_dbm                 = Module('_dbm',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_dbmmodule.c', '-lndbm'])


# Anthony Baxter's gdbm module.  GNU dbm(3) will require -lgdbm:

#_gdbm _gdbmmodule.c -I/usr/local/include -L/usr/local/lib -lgdbm
_gdbm                = Module('_gdbm',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_gdbmmodule.c', '-I/usr/local/include', '-L/usr/local/lib', '-lgdbm'])

# Helper module for various ascii-encoders
binascii             = Module('binascii',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['binascii.c'])
_lsprof              = Module('_lsprof',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_lsprof.o', 'rotatingtree.c'])

# Andrew Kuchling's zlib module.
# This require zlib 1.1.3 (or later).
# See http://www.gzip.org/zlib/

zlib                 = Module('zlib',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['zlibmodule.c', '-I$(prefix)/include', '-lz'])

# Interface to the Expat XML parser
# More information on Expat can be found at www.libexpat.org.

pyexpat              = Module('pyexpat',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['expat/xmlparse.c', 'expat/xmlrole.c', 'expat/xmltok.c', 'pyexpat.c', '-I$(srcdir)/Modules/expat', '-DHAVE_EXPAT_CONFIG_H', '-DUSE_PYEXPAT_CAPI', '-DXML_DEV_URANDOM'])

# Hye-Shik Chang's CJKCodecs

# multibytecodec is required for all the other CJK codec modules
_multibytecodec      = Module('_multibytecodec',   mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/multibytecodec.c'])
_codecs_cn           = Module('_codecs_cn',        mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_cn.c'])
_codecs_hk           = Module('_codecs_hk',        mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_hk.c'])
_codecs_iso2022      = Module('_codecs_iso2022',   mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_iso2022.c'])
_codecs_jp           = Module('_codecs_jp',        mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_jp.c'])
_codecs_kr           = Module('_codecs_kr',        mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_kr.c'])
_codecs_tw           = Module('_codecs_tw',        mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_tw.c'])

# Uncommenting the following line tells makesetup that all following modules
# are not built (see above for more detail).
#
#parser               = Module('parser',            mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 0}, elems=['parser.c'])

#*disabled*
#
#_sqlite3 _tkinter _curses pyexpat
#_codecs_jp _codecs_kr _codecs_tw unicodedata
