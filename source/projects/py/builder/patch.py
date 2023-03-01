"""patch.py or setup_local.py

Generates the Setup.local using during python compilation from source.

Versions = ['3.7', '3.8', ...]
Levels = (0, range(4), range(4), range(4), range(4))
    values:
        0 -> excluded
        1 -> static
        2 -> shared
        3 -> disabled

Levels[0] = Should always = default setup local
Levels[1] = minimal shared
Levels[2] = tiny shared
Levels[3] = minimal static
Levels[4] = tiny static

>>> pyjs targets
make default              : non-portable pyjs externals linked to your system
make homebrew-pkg         : portable package w/ pyjs (requires homebrew python)
make homebrew-ext         : portable pyjs externals (requires homebrew python)
make shared-pkg           : portable package with pyjs externals (shared)
make shared-ext           : portable pyjs externals (shared)
make static-ext           : portable pyjs externals (static)
make tiny-static-ext      : tiny portable pyjs externals (static)
make framework-pkg        : portable package with pyjs externals (framework)
make framework-ext        : portable pyjs externals (framework)
make relocatable-pkg      : portable package w/ more custom options (framework)

>>> python targets
make python-shared        : minimal shared python build
make python-shared-ext    : minimal shared python build for externals
make python-shared-pkg    : minimal shared python build for packages
make python-static        : minimal statically-linked python build
make python-framework     : minimal framework python build
make python-framework-ext : minimal framework python build for externals
make python-framework-pkg : minimal framework python build for packages
make python-relocatable   : custom relocatable python framework build


class FrameworkPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-shared.local"


class SharedPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-shared.local"


class StaticPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-static-min3.local"

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
class BeewarePythonBuilder(StaticPythonBuilder):
    setup_local = "setup.beeware"


class TinyStaticPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-static-min6.local"


class SharedPythonForExtBuilder(SharedPythonBuilder):
    setup_local = "setup-shared.local"


class SharedPythonForPkgBuilder(SharedPythonBuilder):
    setup_local = "setup-shared.local"


class FrameworkPythonForExtBuilder(FrameworkPythonBuilder):
    setup_local = "setup-shared.local"


class FrameworkPythonForPkgBuilder(FrameworkPythonBuilder):
    setup_local = "setup-shared.local"


"""
import sysconfig

from typing import List, Tuple, Dict

from enum import Enum


class Module:
    def __init__(self, name: str, category: str, levels: Tuple[int, int, int, int],
                 elems: List[str]):
        self.name = name
        self.category = category
        self.levels = levels
        self.elems = elems

    def __repr__(self):
        return f"<{self.__class__.__name__} '{self.name}'>"

    def __str__(self):
        return " ".join([self.name] + self.elems)

    def as_tuple(self):
        m = self
        return (m.name, m.category, m.levels)

# ----------------------------------------------------------------------------
# Module Configuration

class Action(Enum):
    exclude  = 0
    static   = 1
    shaed    = 2
    disabled = 3


class Build(Enum):
    default     = 0
    shared_mini = 1
    shared_tiny = 2
    static_mini = 3
    static_tiny = 4


MODULES = {}

    #                                                              0=excluded,
    #                                                              1=static,
    #                                                              2=shared,
    # object name        Module definition             category    3=disabled                    build elements
    #                                                                    def shm sht stm stt 
MODULES['common'] = dict(
    _abc                 = Module('_abc',              'core',     levels=(1, 1, 1, 1, 1), elems=['-DPy_BUILD_CORE_BUILTIN', '_abc.c']),
    _asyncio             = Module('_asyncio',          'optional', levels=(0, 1, 0, 1, 0), elems=['_asynciomodule.c']),
    _bisect              = Module('_bisect',           'optional', levels=(0, 1, 0, 1, 0), elems=['_bisectmodule.c']),
    _blake2              = Module('_blake2',           'optional', levels=(0, 1, 0, 1, 0), elems=['_blake2/blake2module.c', '_blake2/blake2b_impl.c', '_blake2/blake2s_impl.c']),
    _bz2                 = Module('_bz2',              'optional', levels=(0, 3, 3, 3, 0), elems=[]),
    _codecs              = Module('_codecs',           'core',     levels=(1, 1, 1, 1, 1), elems=['_codecsmodule.c']),
    _codecs_cn           = Module('_codecs_cn',        'optional', levels=(0, 3, 3, 3, 0), elems=['cjkcodecs/_codecs_cn.c']),
    _codecs_hk           = Module('_codecs_hk',        'optional', levels=(0, 3, 3, 3, 0), elems=['cjkcodecs/_codecs_hk.c']),
    _codecs_iso2022      = Module('_codecs_iso2022',   'optional', levels=(0, 3, 3, 3, 0), elems=['cjkcodecs/_codecs_iso2022.c']),
    _codecs_jp           = Module('_codecs_jp',        'optional', levels=(0, 3, 3, 3, 0), elems=['cjkcodecs/_codecs_jp.c']),
    _codecs_kr           = Module('_codecs_kr',        'optional', levels=(0, 3, 3, 3, 0), elems=['cjkcodecs/_codecs_kr.c']),
    _codecs_tw           = Module('_codecs_tw',        'optional', levels=(0, 3, 3, 3, 0), elems=['cjkcodecs/_codecs_tw.c']),
    _collections         = Module('_collections',      'core',     levels=(1, 1, 1, 1, 1), elems=['_collectionsmodule.c']),
    _contextvars         = Module('_contextvars',      'optional', levels=(0, 1, 0, 1, 0), elems=['_contextvarsmodule.c']),
    _crypt               = Module('_crypt',            'optional', levels=(0, 0, 0, 3, 0), elems=['_cryptmodule.c', '-lcrypt']),
    _csv                 = Module('_csv',              'optional', levels=(0, 1, 0, 1, 0), elems=['_csv.c']),
    _ctypes              = Module('_ctypes',           'optional', levels=(0, 3, 3, 3, 0), elems=[]),
    _curses              = Module('_curses',           'optional', levels=(0, 3, 3, 3, 0), elems=['_cursesmodule.c', '-lcurses', '-ltermcap', '-DPy_BUILD_CORE_MODULE']),
    _curses_panel        = Module('_curses_panel',     'optional', levels=(0, 3, 3, 3, 0), elems=['_curses_panel.c', '-lpanel', '-lncurses']),
    _datetime            = Module('_datetime',         'optional', levels=(0, 1, 0, 1, 0), elems=['_datetimemodule.c']),
    _dbm                 = Module('_dbm',              'optional', levels=(0, 3, 3, 3, 0), elems=['_dbmmodule.c', '-lndbm']),
    _decimal             = Module('_decimal',          'optional', levels=(0, 3, 3, 3, 0), elems=[]),
    _elementtree         = Module('_elementtree',      'optional', levels=(0, 1, 0, 1, 0), elems=['-I$(srcdir)/Modules/expat', '-DHAVE_EXPAT_CONFIG_H', '-DUSE_PYEXPAT_CAPI', '_elementtree.c']),
    _functools           = Module('_functools',        'core',     levels=(1, 1, 1, 1, 1), elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_functoolsmodule.c']),
    _gdbm                = Module('_gdbm',             'optional', levels=(0, 3, 3, 3, 0), elems=['_gdbmmodule.c', '-I/usr/local/include', '-L/usr/local/lib', '-lgdbm']),
    _hashlib             = Module('_hashlib',          'optional', levels=(0, 0, 0, 1, 0), elems=['_hashopenssl.c.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-l:libcrypto.a', '-Wl,--exclude-libs,libcrypto.a']),
    _hashlib_shared      = Module('_hashlib',          'optional', levels=(0, 2, 2, 0, 0), elems=['_hashopenssl.c.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-lssl', '-lcrypto']),
    _heapq               = Module('_heapq',            'optional', levels=(0, 1, 0, 1, 0), elems=['_heapqmodule.c', '-DPy_BUILD_CORE_MODULE']),
    _io                  = Module('_io',               'core',     levels=(1, 1, 1, 1, 1), elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '-I$(srcdir)/Modules/_io', '_io/_iomodule.c', '_io/iobase.c', '_io/fileio.c', '_io/bytesio.c', '_io/bufferedio.c', '_io/textio.c', '_io/stringio.c']),
    _locale              = Module('_locale',           'core',     levels=(1, 1, 1, 1, 1), elems=['-DPy_BUILD_CORE_BUILTIN', '_localemodule.c']),
    _lsprof              = Module('_lsprof',           'optional', levels=(0, 1, 0, 1, 0), elems=['_lsprof.o', 'rotatingtree.c']),
    _lzma                = Module('_lzma',             'optional', levels=(0, 3, 3, 3, 0), elems=[]),
    _md5                 = Module('_md5',              'optional', levels=(0, 1, 0, 1, 0), elems=['md5module.c']),
    _multibytecodec      = Module('_multibytecodec',   'optional', levels=(0, 3, 3, 3, 0), elems=['cjkcodecs/multibytecodec.c']),
    _multiprocessing     = Module('_multiprocessing',  'optional', levels=(0, 1, 0, 1, 0), elems=['_multiprocessing/multiprocessing.c', '_multiprocessing/semaphore.c']),
    _opcode              = Module('_opcode',           'optional', levels=(0, 1, 0, 1, 0), elems=['_opcode.c']),
    _operator            = Module('_operator',         'core',     levels=(1, 1, 1, 1, 1), elems=['-DPy_BUILD_CORE_BUILTIN', '_operator.c']),
    _pickle              = Module('_pickle',           'optional', levels=(0, 1, 0, 1, 0), elems=['-DPy_BUILD_CORE_MODULE', '_pickle.c']),
    _posixshmem          = Module('_posixshmem',       'optional', levels=(0, 1, 0, 1, 0), elems=['_multiprocessing/posixshmem.c']),
    _posixsubprocess     = Module('_posixsubprocess',  'optional', levels=(0, 1, 0, 1, 0), elems=['-DPy_BUILD_CORE_BUILTIN', '_posixsubprocess.c']),
    _random              = Module('_random',           'optional', levels=(0, 1, 0, 1, 0), elems=['_randommodule.c', '-DPy_BUILD_CORE_MODULE']),
    _scproxy             = Module('_scproxy',          'optional', levels=(0, 3, 3, 3, 0), elems=['_scproxy.c', '-framework', 'SystemConfiguration', '-framework', 'CoreFoundation']),
    _sha1                = Module('_sha1',             'optional', levels=(0, 1, 0, 1, 0), elems=['sha1module.c']),
    _sha256              = Module('_sha256',           'optional', levels=(0, 1, 0, 1, 0), elems=['sha256module.c', '-DPy_BUILD_CORE_BUILTIN']),
    _sha3                = Module('_sha3',             'optional', levels=(0, 1, 0, 1, 0), elems=['_sha3/sha3module.c']),
    _sha512              = Module('_sha512',           'optional', levels=(0, 1, 0, 1, 0), elems=['sha512module.c', '-DPy_BUILD_CORE_BUILTIN']),
    _signal              = Module('_signal',           'core',     levels=(1, 1, 1, 1, 1), elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'signalmodule.c']),
    _socket              = Module('_socket',           'optional', levels=(0, 1, 0, 1, 0), elems=['socketmodule.c']),
    _sqlite3             = Module('_sqlite3',          'optional', levels=(0, 3, 3, 3, 0), elems=[]),
    _sre                 = Module('_sre',              'core',     levels=(1, 1, 1, 1, 1), elems=['-DPy_BUILD_CORE_BUILTIN', '_sre.c']),
    _ssl                 = Module('_ssl',              'optional', levels=(0, 0, 0, 1, 0), elems=['_ssl.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-l:libssl.a', '-Wl,--exclude-libs,libssl.a', '-l:libcrypto.a', '-Wl,--exclude-libs,libcrypto.a']),
    _ssl_shared          = Module('_ssl',              'optional', levels=(0, 2, 2, 0, 0), elems=['_ssl.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-lssl', '-lcrypto']),
    _stat                = Module('_stat',             'core',     levels=(1, 1, 1, 1, 1), elems=['_stat.c']),
    _struct              = Module('_struct',           'optional', levels=(0, 1, 0, 1, 0), elems=['-DPy_BUILD_CORE_MODULE', '_struct.c']),
    _symtable            = Module('_symtable',         'core',     levels=(1, 1, 1, 1, 1), elems=['symtablemodule.c']),
    _testcapi            = Module('_testcapi',         'optional', levels=(0, 3, 3, 3, 0), elems=['_testcapimodule.c']),
    _thread              = Module('_thread',           'core',     levels=(1, 1, 1, 1, 1), elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_threadmodule.c']),
    _tkinter             = Module('_tkinter',          'optional', levels=(0, 3, 3, 3, 0), elems=[]),    
    _tracemalloc         = Module('_tracemalloc',      'core',     levels=(1, 1, 1, 1, 1), elems=['_tracemalloc.c']),
    _uuid                = Module('_uuid',             'optional', levels=(0, 1, 0, 1, 0), elems=['_uuidmodule.c']),
    _weakref             = Module('_weakref',          'core',     levels=(1, 1, 1, 1, 1), elems=['_weakref.c']),
    _xxsubinterpreters   = Module('_xxsubinterpreters','optional', levels=(0, 3, 3, 3, 0), elems=[]),
    array                = Module('array',             'optional', levels=(0, 1, 0, 1, 0), elems=['-DPy_BUILD_CORE_MODULE', 'arraymodule.c']),
    atexit               = Module('atexit',            'core',     levels=(1, 1, 1, 1, 1), elems=['atexitmodule.c']),
    audioop              = Module('audioop',           'optional', levels=(0, 3, 3, 3, 0), elems=['audioop.c']),
    binascii             = Module('binascii',          'optional', levels=(0, 1, 0, 1, 0), elems=['binascii.c']),
    cmath                = Module('cmath',             'optional', levels=(0, 1, 0, 1, 0), elems=['cmathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE']),
    errno                = Module('errno',             'core',     levels=(1, 1, 1, 1, 1), elems=['errnomodule.c']),
    faulthandler         = Module('faulthandler',      'core',     levels=(1, 1, 1, 1, 1), elems=['faulthandler.c']),
    fcntl                = Module('fcntl',             'optional', levels=(0, 1, 0, 1, 0), elems=['fcntlmodule.c']),
    grp                  = Module('grp',               'optional', levels=(0, 1, 0, 1, 0), elems=['grpmodule.c']),
    itertools            = Module('itertools',         'core',     levels=(1, 1, 1, 1, 1), elems=['itertoolsmodule.c']),
    math                 = Module('math',              'optional', levels=(0, 1, 0, 1, 0), elems=['mathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE']),
    mmap                 = Module('mmap',              'optional', levels=(0, 3, 3, 3, 0), elems=['mmapmodule.c']),
    nis                  = Module('nis',               'optional', levels=(0, 3, 3, 3, 0), elems=['nismodule.c', '-lnsl']),
    parser               = Module('parser',            'optional', levels=(0, 1, 0, 1, 0), elems=['parser.c']),
    posix                = Module('posix',             'core',     levels=(1, 1, 1, 1, 1), elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'posixmodule.c']),
    pwd                  = Module('pwd',               'core',     levels=(1, 1, 1, 1, 1), elems=['pwdmodule.c']),
    pyexpat              = Module('pyexpat',           'optional', levels=(0, 1, 0, 1, 0), elems=['expat/xmlparse.c', 'expat/xmlrole.c', 'expat/xmltok.c', 'pyexpat.c', '-I$(srcdir)/Modules/expat', '-DHAVE_EXPAT_CONFIG_H', '-DUSE_PYEXPAT_CAPI', '-DXML_DEV_URANDOM']),
    readline             = Module('readline',          'optional', levels=(0, 3, 3, 3, 0), elems=['readline.c', '-lreadline', '-ltermcap']),
    resource             = Module('resource',          'optional', levels=(0, 3, 3, 3, 0), elems=['resource.c']),
    select               = Module('select',            'optional', levels=(0, 1, 0, 1, 0), elems=['selectmodule.c']),
    spwd                 = Module('spwd',              'optional', levels=(0, 0, 0, 0, 0), elems=['spwdmodule.c']),
    syslog               = Module('syslog',            'optional', levels=(0, 3, 3, 3, 0), elems=['syslogmodule.c']),
    termios              = Module('termios',           'optional', levels=(0, 3, 3, 3, 0), elems=['termios.c']),
    time                 = Module('time',              'core',     levels=(1, 1, 1, 1, 1), elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'timemodule.c']),
    unicodedata          = Module('unicodedata',       'optional', levels=(0, 1, 0, 1, 0), elems=['unicodedata.c', '-DPy_BUILD_CORE_BUILTIN']),
    xxlimited            = Module('xxlimited',         'optional', levels=(0, 3, 3, 3, 0), elems=[]),
    xxlimited_35         = Module('xxlimited_35',      'optional', levels=(0, 3, 3, 3, 0), elems=[]),
    xxsubtype            = Module('xxsubtype',         'optional', levels=(0, 3, 3, 3, 0), elems=[]),
    zlib                 = Module('zlib',              'optional', levels=(0, 1, 0, 1, 0), elems=['zlibmodule.c', '-I$(prefix)/include', '-lz']),
)


def update(version, **kwds):
    MODULES[version] = {}
    MODULES[version].update(MODULES['common'])
    MODULES[version].update(dict(**kwds))


update('3.7',
    zipimport            = Module('zipimport',         'core',     levels=(1, 1, 1, 1, 1), elems=['-DPy_BUILD_CORE', 'zipimport.c']),
)

update('3.8',
    _json                = Module('_json',             'optional', levels=(0, 1, 0, 1, 1), elems=['-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_BUILTIN', '_json.c']),
    _statistics          = Module('_statistics',       'optional', levels=(0, 1, 0, 1, 1), elems=['_statisticsmodule.c']),
    _testinternalcapi    = Module('_testinternalcapi', 'optional', levels=(0, 3, 3, 3, 3), elems=['_testinternalcapi.c', '-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_MODULE']),
)

update('3.9',
    _json                = Module('_json',             'optional', levels=(0, 1, 0, 1, 1), elems=['-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_BUILTIN', '_json.c']),
    _peg_parser          = Module('_peg_parser',       'core',     levels=(1, 1, 1, 1, 1), elems=['_peg_parser.c']),
    _statistics          = Module('_statistics',       'optional', levels=(0, 1, 0, 1, 1), elems=['_statisticsmodule.c']),
    _testinternalcapi    = Module('_testinternalcapi', 'optional', levels=(0, 3, 3, 3, 3), elems=['_testinternalcapi.c', '-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_MODULE']),
    _zoneinfo            = Module('_zoneinfo',         'optional', levels=(0, 1, 0, 1, 1), elems=['_zoneinfo.c', '-DPy_BUILD_CORE_MODULE']),
)

update('3.10',
   _json                 = Module('_json',             'optional', levels=(0, 1, 0, 1, 1), elems=['-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_BUILTIN', '_json.c']),
    _peg_parser          = Module('_peg_parser',       'core',     levels=(1, 1, 1, 1, 1), elems=['_peg_parser.c']),
    _statistics          = Module('_statistics',       'optional', levels=(0, 1, 0, 1, 1), elems=['_statisticsmodule.c']),
    _testinternalcapi    = Module('_testinternalcapi', 'optional', levels=(0, 3, 3, 3, 3), elems=['_testinternalcapi.c', '-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_MODULE']),
    _zoneinfo            = Module('_zoneinfo',         'optional', levels=(0, 1, 0, 1, 1), elems=['_zoneinfo.c', '-DPy_BUILD_CORE_MODULE']),
)

update('3.11',
    _json                = Module('_json',             'optional', levels=(0, 1, 0, 1, 0), elems=['-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_BUILTIN', '_json.c']),
    _peg_parser          = Module('_peg_parser',       'core',     levels=(1, 1, 1, 1, 0), elems=['_peg_parser.c']),
    _statistics          = Module('_statistics',       'optional', levels=(0, 1, 0, 1, 0), elems=['_statisticsmodule.c']),
    _testinternalcapi    = Module('_testinternalcapi', 'optional', levels=(0, 3, 3, 3, 0), elems=['_testinternalcapi.c', '-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_MODULE']),
    _zoneinfo            = Module('_zoneinfo',         'optional', levels=(0, 1, 0, 1, 0), elems=['_zoneinfo.c', '-DPy_BUILD_CORE_MODULE']),
    _sre                 = Module('_sre',              'core',     levels=(1, 1, 1, 1, 1), elems=['_sre/sre.c',  '-DPy_BUILD_CORE_BUILTIN']),
)





# ----------------------------------------------------------------------------
# Setup Management 


class PythonSetup:
    def __init__(self, version=None):
        if not version:
            self.version = self.py_version_short
        else:
            self.version = version
        self.modules = MODULES[self.version]

    @property
    def py_version_short(self):
        return sysconfig.get_config_var('py_version_short')

    def filter(self, callback):
        return dict({k:v for k,v in self.modules.items() if callback(v)})

    def get_core(self):
        return self.filter(lambda m: m.category == 'core')

    def get_optional(self):
        return self.filter(lambda m: m.category == 'optional')

    def get_config(self, level):
        excluded =  self.filter(lambda m: m.levels[level] == 0)
        static = self.filter(lambda m: m.levels[level] == 1)
        shared = self.filter(lambda m: m.levels[level] == 2)
        disabled = self.filter(lambda m: m.levels[level] == 3)
        return excluded, static, shared, disabled

    def get_setup(self, level: int):  # type: ignore
        return self.filter(lambda m: m.levels[level] in [0, 1, 2, 3])

    # def render(self, level: int, setup_path: str = 'setup.local', static_ssl=False):
    #     setup = self.get_setup(level)
    #     core = self.get_core()
    #     optional = self.get_optional()
    #     excluded, static, shared, disabled = self.get_config(level)
    #     with open(setup_path, 'w') as f:
    #         writeln = lambda *s: print(*s, file=f)
    #         writeln("# VARS")
    #         writeln("# " + "-"*77)
    #         writeln("DESTLIB=$(LIBDEST)")
    #         writeln("DESTLIB=$(LIBDEST)")
    #         writeln("MACHDESTLIB=$(BINLIBDEST)")
    #         writeln("DESTPATH=")
    #         writeln("SITEPATH=")
    #         writeln("TESTPATH=")
    #         writeln("COREPYTHONPATH=$(DESTPATH)$(SITEPATH)$(TESTPATH)")
    #         writeln("PYTHONPATH=$(COREPYTHONPATH)")
    #         if static_ssl:
    #             writeln("OPENSSL=/path/to/openssl/directory")
    #         writeln()
    #         writeln("# CORE MODULES")
    #         writeln("# " + "-"*77)
    #         for module in core:
    #             writeln(module, " ".join(core[module].elems))
    #         if static:
    #             writeln()
    #             writeln("# OPTIONAL STATIC")
    #             writeln("# " + "-"*77)
    #             writeln("*static*")
    #             for module in static:
    #                 writeln(module, " ".join(static[module].elems))
    #         if shared:
    #             writeln()
    #             writeln("# OPTIONAL SHARED")
    #             writeln("# " + "-"*77)
    #             writeln("*shared*")
    #             for module in shared:
    #                 writeln(module, " ".join(shared[module].elems))
    #         if disabled:
    #             writeln()
    #             writeln("# OPTIONAL DISABLED")
    #             writeln("# " + "-"*77)
    #             writeln("*disabled*")
    #             for module in disabled:
    #                 writeln(module)

    def render(self, level: int, setup_path: str = 'setup.local', static_ssl=False):
        setup = self.get_setup(level)
        excluded, static, shared, disabled = self.get_config(level)
        with open(setup_path, 'w') as f:
            writeln = lambda *s: print(*s, file=f)
            writeln("# VARS")
            writeln("# " + "-"*77)
            writeln("DESTLIB=$(LIBDEST)")
            writeln("DESTLIB=$(LIBDEST)")
            writeln("MACHDESTLIB=$(BINLIBDEST)")
            writeln("DESTPATH=")
            writeln("SITEPATH=")
            writeln("TESTPATH=")
            writeln("COREPYTHONPATH=$(DESTPATH)$(SITEPATH)$(TESTPATH)")
            writeln("PYTHONPATH=$(COREPYTHONPATH)")
            if static_ssl:
                writeln("OPENSSL=/path/to/openssl/directory")
            writeln()
            if static:
                writeln()
                writeln("# STATIC")
                writeln("# " + "-"*77)
                writeln("*static*")
                for module in static:
                    writeln(module, " ".join(static[module].elems))
            if shared:
                writeln()
                writeln("# SHARED")
                writeln("# " + "-"*77)
                writeln("*shared*")
                for module in shared:
                    writeln(module, " ".join(shared[module].elems))
            if disabled:
                writeln()
                writeln("# DISABLED")
                writeln("# " + "-"*77)
                writeln("*disabled*")
                for module in disabled:
                    writeln(module)


def render_all():
    for version in MODULES.keys():
        if version == 'common':
            continue
        p = PythonSetup(version)
        for i in Build: # build types
            name = f'setup-{version}-{i.name}.local.sh'
            p.render(level=i.value, setup_path=f'setups/{name}')

if __name__ == '__main__':
    render_all()



