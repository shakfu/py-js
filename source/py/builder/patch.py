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

def split(s, category):
    _list = []
    lines = [line for line in s.splitlines() if line]
    for line in lines:
        xs = line.split()
        _list.append((category, xs[0], xs[1:]))
    return _list

core = [
    ('core', 'posix', ['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'posixmodule.c']),
    ('core', 'errno', ['errnomodule.c']),
    ('core', 'pwd', ['pwdmodule.c']),
    ('core', '_sre', ['-DPy_BUILD_CORE_BUILTIN', '_sre.c']),
    ('core', '_codecs', ['_codecsmodule.c']),
    ('core', '_weakref', ['_weakref.c']),
    ('core', '_functools', ['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_functoolsmodule.c']),
    ('core', '_operator', ['-DPy_BUILD_CORE_BUILTIN','_operator.c']),
    ('core', '_collections', ['_collectionsmodule.c']),
    ('core', '_abc', ['-DPy_BUILD_CORE_BUILTIN', '_abc.c']),
    ('core', 'itertools', ['itertoolsmodule.c']),
    ('core', 'atexit', ['atexitmodule.c']),
    ('core', '_signal', ['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'signalmodule.c']),
    ('core', '_stat', ['_stat.c']),
    ('core', 'time', ['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'timemodule.c']),
    ('core', '_thread', ['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_threadmodule.c']),
    ('core', '_locale', ['-DPy_BUILD_CORE_BUILTIN', '_localemodule.c']),
    ('core', '_io',
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
    ('core', 'faulthandler', ['faulthandler.c']),
    ('core', '_tracemalloc', ['_tracemalloc.c']),
    # ('core', '_peg_parser', ['_peg_parser.c']), # removed in 3.10
    ('core', '_symtable',  ['symtablemodule.c']),
 ]




optional = [
    ('optional', 'readline', ['readline.c', '-lreadline', '-ltermcap']),
    ('optional', 'array', ['arraymodule.c']),
    ('optional', 'cmath', ['cmathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', 'math', ['mathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_contextvars', ['_contextvarsmodule.c']),
    ('optional', '_struct', ['_struct.c']),
    ('optional', '_weakref', ['_weakref.c']),
    ('optional', '_testcapi', ['_testcapimodule.c']),
    ('optional', '_testinternalcapi', ['_testinternalcapi.c', '-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_random', ['_randommodule.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_zoneinfo', ['_zoneinfo.c']),
    ('optional', '_elementtree', ['-I$(srcdir)/Modules/expat', '-DHAVE_EXPAT_CONFIG_H', '-DUSE_PYEXPAT_CAPI', '_elementtree.c']),
    ('optional', '_pickle', ['_pickle.c']),
    ('optional', '_datetime', ['_datetimemodule.c']),
    ('optional', '_bisect', ['_bisectmodule.c']),
    ('optional', '_heapq', ['_heapqmodule.c']),
    ('optional', '_asyncio', ['_asynciomodule.c']),
    ('optional', '_json', ['-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_BUILTIN', '_json.c']),
    ('optional', '_statistics', ['_statisticsmodule.c']),
    ('optional', 'unicodedata', ['unicodedata.c']),
    ('optional', '_uuid', ['_uuidmodule.c']),
    ('optional', '_opcode', ['_opcode.c']),
    ('optional', '_multiprocessing', ['_multiprocessing/multiprocessing.c', '_multiprocessing/semaphore.c']),
    ('optional', '_posixshmem', ['_multiprocessing/posixshmem.c']),
    ('optional', 'fcntl', ['fcntlmodule.c']),
    ('optional', 'spwd', ['spwdmodule.c']),
    ('optional', 'grp', ['grpmodule.c']),
    ('optional', 'select', ['selectmodule.c']),
    ('optional', 'mmap', ['mmapmodule.c']),
    ('optional', '_csv', ['_csv.c']),
    ('optional', '_posixsubprocess', ['_posixsubprocess.c']),
    ('optional', '_blake2', ['_blake2/blake2module.c', '_blake2/blake2b_impl.c', '_blake2/blake2s_impl.c']),
    ('optional', 'binascii', ['binascii.c']),
    ('optional', 'parser', ['parsermodule.c']),
    ('optional', '_queue', ['_queuemodule.c']),
    ('optional', '_md5', ['md5module.c']),
    ('optional', '_sha1', ['sha1module.c']),
    ('optional', '_sha256', ['sha256module.c', '-DPy_BUILD_CORE_BUILTIN']),
    ('optional', '_sha512', ['sha512module.c', '-DPy_BUILD_CORE_BUILTIN']),
    ('optional', '_sha3', ['_sha3/sha3module.c']),
    ('optional', '_socket', ['socketmodule.c']),
    ('optional', '_lsprof', ['_lsprof.o', 'rotatingtree.c']),
    ('optional', 'pyexpat',
        ['expat/xmlparse.c',
        'expat/xmlrole.c',
        'expat/xmltok.c',
        'pyexpat.c',
        '-I$(srcdir)/Modules/expat',
        '-DHAVE_EXPAT_CONFIG_H',
        '-DUSE_PYEXPAT_CAPI',
        '-DXML_DEV_URANDOM']),
    ('optional', 'zlib', ['zlibmodule.c', '-I$(prefix)/include', '-lz'])
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

    ('optional', 'readline', ['readline.c', '-lreadline', '-ltermcap']),

# Modules that should always be present (non UNIX dependent):

    ('optional', 'array', ['-DPy_BUILD_CORE_MODULE', 'arraymodule.c']),
    ('optional', 'cmath', ['cmathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', 'math', ['mathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_contextvars', ['_contextvarsmodule.c']),
    ('optional', '_struct', ['-DPy_BUILD_CORE_MODULE', '_struct.c']),
    ('optional', '_weakref', ['_weakref.c']),
    ('optional', '_testcapi', ['_testcapimodule.c']),
    ('optional', '_testinternalcapi', ['_testinternalcapi.c', '-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_random', ['_randommodule.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_elementtree', ['-I$(srcdir)/Modules/expat', '-DHAVE_EXPAT_CONFIG_H', '-DUSE_PYEXPAT_CAPI', '_elementtree.c']),
    ('optional', '_pickle', ['-DPy_BUILD_CORE_MODULE', '_pickle.c']),
    ('optional', '_datetime', ['_datetimemodule.c']),
    ('optional', '_zoneinfo', ['_zoneinfo.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_bisect', ['_bisectmodule.c']),
    ('optional', '_heapq', ['_heapqmodule.c', '-DPy_BUILD_CORE_MODULE']),
    ('optional', '_asyncio', ['_asynciomodule.c']),
    ('optional', '_json', ['-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_BUILTIN', '_json.c']),
    ('optional', '_statistics', ['_statisticsmodule.c']),
    ('optional', 'unicodedata', ['unicodedata.c', '-DPy_BUILD_CORE_BUILTIN']),

    # all 4 modules are removed in 3.10
    # ('optional', '_uuid', ['_uuidmodule.c']),
    # ('optional', '_opcode', ['_opcode.c']),
    # ('optional', '_multiprocessing', ['_multiprocessing/multiprocessing.c', '_multiprocessing/semaphore.c']),
    # ('optional', '_posixshmem', ['_multiprocessing/posixshmem.c']),

# Modules with some UNIX dependencies -- on by default:
# (If you have a really backward UNIX, select and socket may not be
# supported...)

    ('optional', 'fcntl', ['fcntlmodule.c']),
    ('optional', 'spwd', ['spwdmodule.c']),
    ('optional', 'grp', ['grpmodule.c']),
    ('optional', 'select', ['selectmodule.c']),

# Memory-mapped files (also works on Win32).

    ('optional', 'mmap', ['mmapmodule.c']),

# CSV file helper

    ('optional', '_csv', ['_csv.c']),


# Socket module helper for socket(2)

    ('optional', '_socket', ['socketmodule.c']),


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

#_crypt _cryptmodule.c # -lcrypt    # crypt(3); needs -lcrypt on some systems


# Some more UNIX dependent modules -- off by default, since these
# are not supported by all UNIX systems:

#nis nismodule.c -lnsl  # Sun yellow pages -- not everywhere
#termios termios.c  # Steen Lumholt's termios module
#resource resource.c    # Jeremy Hylton's rlimit interface

    ('optional', '_posixsubprocess', ['-DPy_BUILD_CORE_BUILTIN', '_posixsubprocess.c']),


# Multimedia modules -- off by default.
# These don't work for 64-bit platforms!!!
# #993173 says audioop works on 64-bit platforms, though.
# These represent audio samples or images as strings:

#audioop audioop.c  # Operations on audio samples


# Note that the _md5 and _sha modules are normally only built if the
# system does not have the OpenSSL libs containing an optimized version.

    ('optional', '_md5', ['md5module.c']),
    ('optional', '_sha1', ['sha1module.c']),
    ('optional', '_sha256', ['sha256module.c', '-DPy_BUILD_CORE_BUILTIN']),
    ('optional', '_sha512', ['sha512module.c', '-DPy_BUILD_CORE_BUILTIN']),
    ('optional', '_sha3', ['_sha3/sha3module.c']),

# _blake module
    ('optional', '_blake2', ['_blake2/blake2module.c', '_blake2/blake2b_impl.c', '_blake2/blake2s_impl.c']),


# The _tkinter module.
#
# The command for _tkinter is long and site specific.  Please
# uncomment and/or edit those parts as indicated.  If you don't have a
# specific extension (e.g. Tix or BLT), leave the corresponding line
# commented out.  (Leave the trailing backslashes in!  If you
# experience strange errors, you may want to join all uncommented
# lines and remove the backslashes -- the backslash interpretation is
# done by the shell's "read" command and it may not be implemented on
# every system.

# *** Always uncomment this (leave the leading underscore in!):
# _tkinter _tkinter.c tkappinit.c -DWITH_APPINIT \
# *** Uncomment and edit to reflect where your Tcl/Tk libraries are:
#   -L/usr/local/lib \
# *** Uncomment and edit to reflect where your Tcl/Tk headers are:
#   -I/usr/local/include \
# *** Uncomment and edit to reflect where your X11 header files are:
#   -I/usr/X11R6/include \
# *** Or uncomment this for Solaris:
#   -I/usr/openwin/include \
# *** Uncomment and edit for Tix extension only:
#   -DWITH_TIX -ltix8.1.8.2 \
# *** Uncomment and edit for BLT extension only:
#   -DWITH_BLT -I/usr/local/blt/blt8.0-unoff/include -lBLT8.0 \
# *** Uncomment and edit for PIL (TkImaging) extension only:
#     (See http://www.pythonware.com/products/pil/ for more info)
#   -DWITH_PIL -I../Extensions/Imaging/libImaging  tkImaging.c \
# *** Uncomment and edit for TOGL extension only:
#   -DWITH_TOGL togl.c \
# *** Uncomment and edit to reflect your Tcl/Tk versions:
#   -ltk8.2 -ltcl8.2 \
# *** Uncomment and edit to reflect where your X11 libraries are:
#   -L/usr/X11R6/lib \
# *** Or uncomment this for Solaris:
#   -L/usr/openwin/lib \
# *** Uncomment these for TOGL extension only:
#   -lGL -lGLU -lXext -lXmu \
# *** Uncomment for AIX:
#   -lld \
# *** Always uncomment this; X11 libraries to link with:
#   -lX11

# Lance Ellinghaus's syslog module
#syslog syslogmodule.c      # syslog daemon interface
('optional', 'syslog', ['syslogmodule.c']),

# Curses support, requiring the System V version of curses, often
# provided by the ncurses library.  e.g. on Linux, link with -lncurses
# instead of -lcurses).

('optional', '_curses', ['_cursesmodule.c', '-lcurses', '-ltermcap', '-DPy_BUILD_CORE_MODULE']),
# Wrapper for the panel library that's part of ncurses and SYSV curses.
('optional', '_curses_panel', ['_curses_panel.c', '-lpanel', '-lncurses']),

# Modules that provide persistent dictionary-like semantics.  You will
# probably want to arrange for at least one of them to be available on
# your machine, though none are defined by default because of library
# dependencies.  The Python module dbm/__init__.py provides an
# implementation independent wrapper for these; dbm/dumb.py provides
# similar functionality (but slower of course) implemented in Python.

#_dbm _dbmmodule.c  # dbm(3) may require -lndbm or similar
('optional', '_dbm', ['_dbmmodule.c', '-lndbm']),

# Anthony Baxter's gdbm module.  GNU dbm(3) will require -lgdbm:

#_gdbm _gdbmmodule.c -I/usr/local/include -L/usr/local/lib -lgdbm
('optional', '_gdbm', ['_gdbmmodule.c', '-I/usr/local/include', '-L/usr/local/lib', '-lgdbm']),

# Helper module for various ascii-encoders
('optional', 'binascii', ['binascii.c']),

    # removed in 3.10
    # ('optional', '_lsprof', ['_lsprof.o', 'rotatingtree.c']),


# Andrew Kuchling's zlib module.
# This require zlib 1.1.3 (or later).
# See http://www.gzip.org/zlib/

    ('optional', 'zlib', ['zlibmodule.c', '-I$(prefix)/include', '-lz'])

# Interface to the Expat XML parser
# More information on Expat can be found at www.libexpat.org.

    ('optional', 'pyexpat',
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
 ('optional', '_multibytecodec', ['cjkcodecs/multibytecodec.c']),

 ('optional', '_codecs_cn', ['cjkcodecs/_codecs_cn.c']),
 ('optional', '_codecs_hk', ['cjkcodecs/_codecs_hk.c']),
 ('optional', '_codecs_iso2022', ['cjkcodecs/_codecs_iso2022.c']),
 ('optional', '_codecs_jp', ['cjkcodecs/_codecs_jp.c']),
 ('optional', '_codecs_kr', ['cjkcodecs/_codecs_kr.c']),
 ('optional', '_codecs_tw', ['cjkcodecs/_codecs_tw.c']),

# Example -- included for reference only:
# xx xxmodule.c

# Another example -- the 'xxsubtype' module shows C-level subtyping in action
xxsubtype xxsubtype.c

# Uncommenting the following line tells makesetup that all following modules
# are not built (see above for more detail).
#
#*disabled*
#
#_sqlite3 _tkinter _curses pyexpat
#_codecs_jp _codecs_kr _codecs_tw unicodedata


