"""
The file Setup is used by the makesetup script to construct the files
Makefile and config.c, from Makefile.pre and config.c.in,
respectively.  Note that Makefile.pre is created from Makefile.pre.in
by the toplevel configure script.

(VPATH notes: Setup and Makefile.pre are in the build directory, as
are Makefile and config.c; the *.in files are in the source directory.)

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
from typing import List

def split(s, category):
    _list = []
    lines = [line for line in s.splitlines() if line]
    for line in lines:
        xs = line.split()
        _list.append((category, xs[0], xs[1:]))
    return _list


class Module:
    def __init__(self, category: str, name: str,
            is_enabled: bool = True, is_static: bool = False,
            included_in: int = 0, excluded_in: int = 0,
            steps: List[str] = None):
        self.category = category
        self.name = name
        self.is_enabled = is_enabled
        self.is_static = is_static
        self.steps = steps

    def __repr__(self):
        return f"<{self.__class__.__name__} '{self.name}'>"

    def as_tuple(self):
        m = self
        return (m.category, m.name, m.is_enabled, m.is_static)

core = [
    ('core', 'posix', 1, 1, ['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'posixmodule.c']),
    ('core', 'errno', 1, 1,  ['errnomodule.c']),
    ('core', 'pwd', 1, 1,  ['pwdmodule.c']),
    ('core', '_sre', 1, 1,  ['-DPy_BUILD_CORE_BUILTIN', '_sre.c']),
    ('core', '_codecs', 1, 1,  ['_codecsmodule.c']),
    ('core', '_weakref', 1, 1,  ['_weakref.c']),
    ('core', '_functools',  1, 1, ['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_functoolsmodule.c']),
    ('core', '_operator',  1, 1, ['-DPy_BUILD_CORE_BUILTIN','_operator.c']),
    ('core', '_collections',  1, 1, ['_collectionsmodule.c']),
    ('core', '_abc',  1, 1, ['-DPy_BUILD_CORE_BUILTIN', '_abc.c']),
    ('core', 'itertools',  1, 1, ['itertoolsmodule.c']),
    ('core', 'atexit',  1, 1, ['atexitmodule.c']),
    ('core', '_signal',  1, 1, ['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'signalmodule.c']),
    ('core', '_stat',  1, 1, ['_stat.c']),
    ('core', 'time',  1, 1, ['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'timemodule.c']),
    ('core', '_thread',  1, 1, ['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_threadmodule.c']),
    ('core', '_locale',  1, 1, ['-DPy_BUILD_CORE_BUILTIN', '_localemodule.c']),
    ('core', '_io',  1, 1,
        ['-DPy_BUILD_CORE_BUILTIN',
        '-I$(srcdir)/Include/internal',
        '-I$(srcdir)/Modules/_io',
        '_io/_iomodule.c',
        '_io/iobase.c',
        '_io/fileio.c',
        '_io/bytesio.c',
        '_io/bufferedio.c',
        '_io/textio.c',
        '_io/stringio.c']),
    ('core', 'faulthandler',  1, 1, ['faulthandler.c']),
    ('core', '_tracemalloc',  1, 1, ['_tracemalloc.c']),
    ('core', '_peg_parser',  1, 1, ['_peg_parser.c']), # removed in 3.10
    ('core', '_symtable',   1, 1, ['symtablemodule.c']),
 ]


# The rest of the modules listed in this file are all commented out by
# default.  Usually they can be detected and built as dynamically
# loaded modules by the new setup.py script added in Python 2.1.  If
# you're on a platform that doesn't support dynamic loading, want to
# compile modules statically into the Python binary, or need to
# specify some odd set of compiler switches, you can uncomment the
# appropriate lines below.

optional = [

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

    ('optional', 'readline',  1, 1, ['readline.c', '-lreadline', '-ltermcap']),

# Modules that should always be present (non UNIX dependent):

    ('optional', 'array',  1, 1, ['-DPy_BUILD_CORE_MODULE', 'arraymodule.c']),
    ('optional', 'cmath',  1, 1, ['cmathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', 'math',  1, 1, ['mathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_contextvars',  1, 1, ['_contextvarsmodule.c']),
    ('optional', '_struct',  1, 1, ['-DPy_BUILD_CORE_MODULE', '_struct.c']),
    ('optional', '_weakref',  1, 1, ['_weakref.c']),
    ('optional', '_testcapi',  1, 1, ['_testcapimodule.c']),
    ('optional', '_testinternalcapi',  1, 1, ['_testinternalcapi.c', '-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_random',  1, 1, ['_randommodule.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_elementtree',  1, 1, ['-I$(srcdir)/Modules/expat', '-DHAVE_EXPAT_CONFIG_H', '-DUSE_PYEXPAT_CAPI', '_elementtree.c']),
    ('optional', '_pickle',  1, 1, ['-DPy_BUILD_CORE_MODULE', '_pickle.c']),
    ('optional', '_datetime',  1, 1, ['_datetimemodule.c']),
    ('optional', '_zoneinfo',  1, 1, ['_zoneinfo.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_bisect',  1, 1, ['_bisectmodule.c']),
    ('optional', '_heapq',  1, 1, ['_heapqmodule.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_asyncio',  1, 1, ['_asynciomodule.c']),
    ('optional', '_json',  1, 1, ['-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_BUILTIN', '_json.c']),
    ('optional', '_statistics',  1, 1, ['_statisticsmodule.c']),
    ('optional', 'unicodedata',  1, 1, ['unicodedata.c', '-DPy_BUILD_CORE_BUILTIN']),

    # all 4 modules are removed in 3.10
    ('optional', '_uuid',  1, 1, ['_uuidmodule.c']),
    ('optional', '_opcode',  1, 1, ['_opcode.c']),
    ('optional', '_multiprocessing',  1, 1, ['_multiprocessing/multiprocessing.c', '_multiprocessing/semaphore.c']),
    ('optional', '_posixshmem',  1, 1, ['_multiprocessing/posixshmem.c']),

# Modules with some UNIX dependencies -- on by default:
# (If you have a really backward UNIX, select and socket may not be
# supported...)

    ('optional', 'fcntl',  1, 1, ['fcntlmodule.c']),
    ('optional', 'spwd',  1, 1, ['spwdmodule.c']),
    ('optional', 'grp',  1, 1, ['grpmodule.c']),
    ('optional', 'select',  1, 1, ['selectmodule.c']),

# Memory-mapped files (also works on Win32).

    ('optional', 'mmap',  1, 1, ['mmapmodule.c']),

# CSV file helper

    ('optional', '_csv',  1, 1, ['_csv.c']),


# Socket module helper for socket(2)

    ('optional', '_socket',  1, 1, ['socketmodule.c']),


# Socket module helper for SSL support; you must comment out the other
# socket line above, and edit the OPENSSL variable:
# OPENSSL=/path/to/openssl/directory
# _ssl _ssl.c \
#     -I$(OPENSSL)/include -L$(OPENSSL)/lib \
#     -lssl -lcrypto
#_hashlib _hashopenssl.c \
#     -I$(OPENSSL)/include -L$(OPENSSL)/lib \
#     -lcrypto

# To statically link OpenSSL:
# _ssl _ssl.c \
#     -I$(OPENSSL)/include -L$(OPENSSL)/lib \
#     -l:libssl.a -Wl,--exclude-libs,libssl.a \
#     -l:libcrypto.a -Wl,--exclude-libs,libcrypto.a
#_hashlib _hashopenssl.c \
#     -I$(OPENSSL)/include -L$(OPENSSL)/lib \
#     -l:libcrypto.a -Wl,--exclude-libs,libcrypto.a

# The crypt module is now disabled by default because it breaks builds
# on many systems (where -lcrypt is needed), e.g. Linux (I believe).

    ('optional', '_crypt',  1, 1, ['_cryptmodule.c', '-lcrypt']),

# Some more UNIX dependent modules -- off by default, since these
# are not supported by all UNIX systems:

    ('optional', 'nis',       1, 1, ['nismodule.c', '-lnsl']),
    ('optional', 'termios',   1, 1, ['termios.c']),
    ('optional', 'resource',  1, 1, ['resource.c']),


    ('optional', '_posixsubprocess',  1, 1, ['-DPy_BUILD_CORE_BUILTIN', '_posixsubprocess.c']),

# Multimedia modules -- off by default.
# These don't work for 64-bit platforms!!!
# #993173 says audioop works on 64-bit platforms, though.
# These represent audio samples or images as strings:

    ('optional', 'audioop',  1, 1, ['audioop.c']),

# Note that the _md5 and _sha modules are normally only built if the
# system does not have the OpenSSL libs containing an optimized version.

    ('optional', '_md5',  1, 1, ['md5module.c']),
    ('optional', '_sha1',  1, 1, ['sha1module.c']),
    ('optional', '_sha256',  1, 1, ['sha256module.c', '-DPy_BUILD_CORE_BUILTIN']),
    ('optional', '_sha512',  1, 1, ['sha512module.c', '-DPy_BUILD_CORE_BUILTIN']),
    ('optional', '_sha3',  1, 1, ['_sha3/sha3module.c']),

    ('optional', '_blake2',  1, 1, ['_blake2/blake2module.c', '_blake2/blake2b_impl.c', '_blake2/blake2s_impl.c']),


# The _tkinter module. (NOT USED)

# Lance Ellinghaus's syslog module
    ('optional', 'syslog', 1, 1, ['syslogmodule.c']),

# Curses support, requiring the System V version of curses, often
# provided by the ncurses library.  e.g. on Linux, link with -lncurses
# instead of -lcurses).

    ('optional', '_curses',  1, 1, ['_cursesmodule.c', '-lcurses', '-ltermcap', '-DPy_BUILD_CORE_MODULE']),
# Wrapper for the panel library that's part of ncurses and SYSV curses.
    ('optional', '_curses_panel',  1, 1, ['_curses_panel.c', '-lpanel', '-lncurses']),

# Modules that provide persistent dictionary-like semantics.  You will
# probably want to arrange for at least one of them to be available on
# your machine, though none are defined by default because of library
# dependencies.  The Python module dbm/__init__.py provides an
# implementation independent wrapper for these; dbm/dumb.py provides
# similar functionality (but slower of course) implemented in Python.

#_dbm _dbmmodule.c  # dbm(3) may require -lndbm or similar
    ('optional', '_dbm',  1, 1, ['_dbmmodule.c', '-lndbm']),

# Anthony Baxter's gdbm module.  GNU dbm(3) will require -lgdbm:

#_gdbm _gdbmmodule.c -I/usr/local/include -L/usr/local/lib -lgdbm
    ('optional', '_gdbm',  1, 1, ['_gdbmmodule.c', '-I/usr/local/include', '-L/usr/local/lib', '-lgdbm']),

# Helper module for various ascii-encoders
    ('optional', 'binascii',  1, 1, ['binascii.c']),


    ('optional', '_lsprof',  1, 1, ['_lsprof.o', 'rotatingtree.c']), # removed in 3.10

# Andrew Kuchling's zlib module.
# This require zlib 1.1.3 (or later).
# See http://www.gzip.org/zlib/

    ('optional', 'zlib',  1, 1, ['zlibmodule.c', '-I$(prefix)/include', '-lz']),

# Interface to the Expat XML parser
# More information on Expat can be found at www.libexpat.org.

    ('optional', 'pyexpat',  1, 1, 
        ['expat/xmlparse.c',
        'expat/xmlrole.c',
        'expat/xmltok.c',
        'pyexpat.c',
        '-I$(srcdir)/Modules/expat',
        '-DHAVE_EXPAT_CONFIG_H',
        '-DUSE_PYEXPAT_CAPI',
        '-DXML_DEV_URANDOM']),

# Hye-Shik Chang's CJKCodecs

# multibytecodec is required for all the other CJK codec modules
    ('optional', '_multibytecodec',  1, 1, ['cjkcodecs/multibytecodec.c']),

    ('optional', '_codecs_cn',  1, 1, ['cjkcodecs/_codecs_cn.c']),
    ('optional', '_codecs_hk',  1, 1, ['cjkcodecs/_codecs_hk.c']),
    ('optional', '_codecs_iso2022',  1, 1, ['cjkcodecs/_codecs_iso2022.c']),
    ('optional', '_codecs_jp',  1, 1, ['cjkcodecs/_codecs_jp.c']),
    ('optional', '_codecs_kr',  1, 1, ['cjkcodecs/_codecs_kr.c']),
    ('optional', '_codecs_tw',  1, 1, ['cjkcodecs/_codecs_tw.c']),

# Uncommenting the following line tells makesetup that all following modules
# are not built (see above for more detail).
#
#*disabled*
#
#_sqlite3 _tkinter _curses pyexpat
#_codecs_jp _codecs_kr _codecs_tw unicodedata

]

xs = []
for i in core:
    e = Module(*i)
    xs.append(e)

for i in optional:
    e = Module(*i)
    xs.append(e)

class PythonSetup:
    def __init__(self, modules):
        self.modules = modules

    def display(self):
        for m in self.modules:
            print(f'{m.category:<8} {m.name:<18} {m.is_enabled} {m.is_static}')

    def to_csv(self):
        import csv
        with open('modules.csv', 'w', newline='') as csvfile:
            # writer = csv.writer(csvfile, delimiter=' ',
            #                         quotechar='|', quoting=csv.QUOTE_MINIMAL)
            writer = csv.writer(csvfile)
            for m in self.modules:
                writer.writerow(m.as_tuple())

p = PythonSetup(modules = xs)
p.display()
p.to_csv()



