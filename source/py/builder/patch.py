import sysconfig

from typing import List, Tuple, Dict


class Module:
    def __init__(self, name: str, category: str, levels: Tuple[int, int, int, int],
                 vers: Dict[str, int], elems: List[str]):
        self.name = name
        self.category = category
        self.levels = levels
        self.vers = vers
        self.elems = elems

    def __repr__(self):
        return f"<{self.__class__.__name__} '{self.name}'>"

    def __str__(self):
        return " ".join([self.name] + self.elems)

    def as_tuple(self):
        m = self
        return (m.name, m.category) + m.levels + tuple(m.vers.values())

modules = dict(
    # object name          Module definition                       0=disabled, 1=static, 2=shared
    posix                = Module('posix',             'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'posixmodule.c']),
    errno                = Module('errno',             'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['errnomodule.c']),
    pwd                  = Module('pwd',               'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['pwdmodule.c']),
    _sre                 = Module('_sre',              'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_sre.c']),
    _codecs              = Module('_codecs',           'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_codecsmodule.c']),
    _weakref             = Module('_weakref',          'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_weakref.c']),
    _functools           = Module('_functools',        'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_functoolsmodule.c']),
    _operator            = Module('_operator',         'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_operator.c']),
    _collections         = Module('_collections',      'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_collectionsmodule.c']),
    _abc                 = Module('_abc',              'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_abc.c']),
    itertools            = Module('itertools',         'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['itertoolsmodule.c']),
    atexit               = Module('atexit',            'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['atexitmodule.c']),
    _signal              = Module('_signal',           'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'signalmodule.c']),
    _stat                = Module('_stat',             'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_stat.c']),
    time                 = Module('time',              'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'timemodule.c']),
    _thread              = Module('_thread',           'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_threadmodule.c']),
    _locale              = Module('_locale',           'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_localemodule.c']),
    _io                  = Module('_io',               'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '-I$(srcdir)/Modules/_io', '_io/_iomodule.c', '_io/iobase.c', '_io/fileio.c', '_io/bytesio.c', '_io/bufferedio.c', '_io/textio.c', '_io/stringio.c']),
    faulthandler         = Module('faulthandler',      'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['faulthandler.c']),
    zipimport            = Module('zipimport',         'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 0, '3.9': 0, '3.10': 0}, elems=['-DPy_BUILD_CORE', 'zipimport.c']),
    _tracemalloc         = Module('_tracemalloc',      'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_tracemalloc.c']),
    _peg_parser          = Module('_peg_parser',       'core',     levels=(1, 1, 1, 1), vers={'3.7': 0, '3.8': 0, '3.9': 1, '3.10': 1}, elems=['_peg_parser.c']),
    _symtable            = Module('_symtable',         'core',     levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['symtablemodule.c']),
    readline             = Module('readline',          'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['readline.c', '-lreadline', '-ltermcap']),
    array                = Module('array',             'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_MODULE', 'arraymodule.c']),
    cmath                = Module('cmath',             'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cmathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE']),
    math                 = Module('math',              'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['mathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE']),
    _contextvars         = Module('_contextvars',      'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_contextvarsmodule.c']),
    _struct              = Module('_struct',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_MODULE', '_struct.c']),
    _testcapi            = Module('_testcapi',         'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_testcapimodule.c']),
    _testinternalcapi    = Module('_testinternalcapi', 'optional', levels=(1, 1, 1, 1), vers={'3.7': 0, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_testinternalcapi.c', '-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_MODULE']),
    _random              = Module('_random',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_randommodule.c', '-DPy_BUILD_CORE_MODULE']),
    _elementtree         = Module('_elementtree',      'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-I$(srcdir)/Modules/expat', '-DHAVE_EXPAT_CONFIG_H', '-DUSE_PYEXPAT_CAPI', '_elementtree.c']),
    _pickle              = Module('_pickle',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_MODULE', '_pickle.c']),
    _datetime            = Module('_datetime',         'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_datetimemodule.c']),
    _zoneinfo            = Module('_zoneinfo',         'optional', levels=(1, 1, 1, 1), vers={'3.7': 0, '3.8': 0, '3.9': 1, '3.10': 1}, elems=['_zoneinfo.c', '-DPy_BUILD_CORE_MODULE']),
    _bisect              = Module('_bisect',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_bisectmodule.c']),
    _heapq               = Module('_heapq',            'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_heapqmodule.c', '-DPy_BUILD_CORE_MODULE']),
    _asyncio             = Module('_asyncio',          'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_asynciomodule.c']),
    _json                = Module('_json',             'optional', levels=(1, 1, 1, 1), vers={'3.7': 0, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_BUILTIN', '_json.c']),
    _statistics          = Module('_statistics',       'optional', levels=(1, 1, 1, 1), vers={'3.7': 0, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_statisticsmodule.c']),
    unicodedata          = Module('unicodedata',       'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['unicodedata.c', '-DPy_BUILD_CORE_BUILTIN']),
    _uuid                = Module('_uuid',             'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_uuidmodule.c']),
    _opcode              = Module('_opcode',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_opcode.c']),
    _multiprocessing     = Module('_multiprocessing',  'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_multiprocessing/multiprocessing.c', '_multiprocessing/semaphore.c']),
    _posixshmem          = Module('_posixshmem',       'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_multiprocessing/posixshmem.c']),
    fcntl                = Module('fcntl',             'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['fcntlmodule.c']),
    spwd                 = Module('spwd',              'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['spwdmodule.c']),
    grp                  = Module('grp',               'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['grpmodule.c']),
    select               = Module('select',            'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['selectmodule.c']),
    mmap                 = Module('mmap',              'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['mmapmodule.c']),
    _csv                 = Module('_csv',              'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_csv.c']),
    _socket              = Module('_socket',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['socketmodule.c']),
    _ssl_shared          = Module('_ssl',              'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_ssl.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-lssl', '-lcrypto']),
    _hashlib_shared      = Module('_hashlib',          'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_hashopenssl.c.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-lssl', '-lcrypto']),
    _ssl                 = Module('_ssl',              'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_ssl.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-l:libssl.a', '-Wl,--exclude-libs,libssl.a', '-l:libcrypto.a', '-Wl,--exclude-libs,libcrypto.a']),
    _hashlib             = Module('_hashlib',          'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_hashopenssl.c.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-l:libcrypto.a', '-Wl,--exclude-libs,libcrypto.a']),
    _crypt               = Module('_crypt',            'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_cryptmodule.c', '-lcrypt']),
    nis                  = Module('nis',               'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['nismodule.c', '-lnsl']),
    termios              = Module('termios',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['termios.c']),
    resource             = Module('resource',          'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['resource.c']),
    _posixsubprocess     = Module('_posixsubprocess',  'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_posixsubprocess.c']),
    audioop              = Module('audioop',           'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['audioop.c']),
    _md5                 = Module('_md5',              'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['md5module.c']),
    _sha1                = Module('_sha1',             'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['sha1module.c']),
    _sha256              = Module('_sha256',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['sha256module.c', '-DPy_BUILD_CORE_BUILTIN']),
    _sha512              = Module('_sha512',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['sha512module.c', '-DPy_BUILD_CORE_BUILTIN']),
    _sha3                = Module('_sha3',             'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_sha3/sha3module.c']),
    _blake2              = Module('_blake2',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_blake2/blake2module.c', '_blake2/blake2b_impl.c', '_blake2/blake2s_impl.c']),
    syslog               = Module('syslog',            'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['syslogmodule.c']),
    _curses              = Module('_curses',           'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_cursesmodule.c', '-lcurses', '-ltermcap', '-DPy_BUILD_CORE_MODULE']),
    _curses_panel        = Module('_curses_panel',     'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_curses_panel.c', '-lpanel', '-lncurses']),
    _dbm                 = Module('_dbm',              'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_dbmmodule.c', '-lndbm']),
    _gdbm                = Module('_gdbm',             'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_gdbmmodule.c', '-I/usr/local/include', '-L/usr/local/lib', '-lgdbm']),
    binascii             = Module('binascii',          'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['binascii.c']),
    parser               = Module('parser',            'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 0}, elems=['parser.c']),
    _lsprof              = Module('_lsprof',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_lsprof.o', 'rotatingtree.c']),
    zlib                 = Module('zlib',              'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['zlibmodule.c', '-I$(prefix)/include', '-lz']),
    pyexpat              = Module('pyexpat',           'optional', levels=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['expat/xmlparse.c', 'expat/xmlrole.c', 'expat/xmltok.c', 'pyexpat.c', '-I$(srcdir)/Modules/expat', '-DHAVE_EXPAT_CONFIG_H', '-DUSE_PYEXPAT_CAPI', '-DXML_DEV_URANDOM']),
    _multibytecodec      = Module('_multibytecodec',   'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/multibytecodec.c']),
    _codecs_cn           = Module('_codecs_cn',        'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_cn.c']),
    _codecs_hk           = Module('_codecs_hk',        'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_hk.c']),
    _codecs_iso2022      = Module('_codecs_iso2022',   'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_iso2022.c']),
    _codecs_jp           = Module('_codecs_jp',        'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_jp.c']),
    _codecs_kr           = Module('_codecs_kr',        'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_kr.c']),
    _codecs_tw           = Module('_codecs_tw',        'optional', levels=(1, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_tw.c']),

)

class PythonSetup:
    def __init__(self):
        self.modules = modules

    @property
    def py_version_short(self):
        return sysconfig.get_config_var('py_version_short')

    def filter(self, callback, modules=None):
        if not modules:
            modules = self.modules
        return dict({k:v for k,v in modules.items() if callback(v)})

    def get_core(self, modules=None):
        if not modules:
            modules = self.modules
        return self.filter(lambda m: m.category == 'core', modules)

    def get_optional(self, modules=None):
        if not modules:
            modules = self.modules
        return self.filter(lambda m: m.category == 'optional')

    def get_config(self, level, modules=None):
        if not modules:
            modules = self.modules
        disabled = self.filter(lambda m: m.levels[level] == 0, modules)
        static = self.filter(lambda m: m.levels[level] == 1, modules)
        shared = self.filter(lambda m: m.levels[level] == 2, modules)
        return disabled, static, shared

    def get_setup(self, level: int, version: str = None):
        if not version:
            version = self.py_version_short
        return self.filter(lambda m: m.vers[version] and m.levels[level] in [0, 1,2])

    def render(self, level: int, version: str = None, static_ssl=False):
        setup = self.get_setup(level, version)
        core = self.get_core(setup)
        optional = self.get_optional(setup)
        disabled, static, shared = self.get_config(level, optional)
        with open('setup.local', 'w') as f:
            println = lambda *s: print(*s, file=f)
            println("# VARS")
            println("# " + "-"*77)
            println("DESTLIB=$(LIBDEST)")
            println("DESTLIB=$(LIBDEST)")
            println("MACHDESTLIB=$(BINLIBDEST)")
            println("DESTPATH=")
            println("SITEPATH=")
            println("TESTPATH=")
            println("COREPYTHONPATH=$(DESTPATH)$(SITEPATH)$(TESTPATH)")
            println("PYTHONPATH=$(COREPYTHONPATH)")
            if static_ssl:
                println("OPENSSL=/path/to/openssl/directory")
            println()
            println("# CORE MODULES")
            println("# " + "-"*77)
            for m in core:
                println(m, " ".join(core[m].elems))
            println()
            println("# OPTIONAL STATIC")
            println("# " + "-"*77)
            println("*static*")
            for m in static:
                println(m, " ".join(static[m].elems))
            if shared:
                println()
                println("# OPTIONAL SHARED")
                println("# " + "-"*77)
                println("*shared*")
                for m in shared:
                    println(m, " ".join(shared[m].elems))
            println()
            println("# OPTIONAL DISABLED")
            println("# " + "-"*77)
            println("*disabled*")
            for m in disabled:
                println(m)

p = PythonSetup()

