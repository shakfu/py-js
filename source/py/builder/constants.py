import platform


DEFAULT_PYTHON_VERSION = platform.python_version()
DEFAULT_BZ2_VERSION = "1.0.8"
DEFAULT_SSL_VERSION = "1.1.1g"
DEFAULT_XZ_VERSION = "5.2.5"


PACKAGES = [
    # project.python.config_ver_platform,
    "config-{ver}-{platform}",
    "idlelib",
    "lib2to3",
    "tkinter",
    "turtledemo",
    "turtle.py",
    "ctypes",
    "curses",
    "ensurepip",
    "venv",
]

EXTENSIONS = [
    "_tkinter",
    "_ctypes",
    "_multibytecodec",
    "_codecs_jp",
    "_codecs_hk",
    "_codecs_cn",
    "_codecs_kr",
    "_codecs_tw",
    "_codecs_iso2022",
    "_curses",
    "_curses_panel",
]

BINS = [
    "2to3-{ver}",
    "idle{ver}",
    "easy_install-{ver}",
    "pip{ver}",
    "pyvenv-{ver}",
    "pydoc{ver}",
]
