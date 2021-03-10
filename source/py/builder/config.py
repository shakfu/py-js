"""config: global configuration

Place slow changing constants and variables here in UPPERCASE.



"""
import logging

IGNORE_ERRORS = False

DEBUG = True
if DEBUG:
    LOG_LEVEL = logging.DEBUG
else:
    LOG_LEVEL = logging.INFO

LOG_FORMAT = '%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s'



# class Builder:

#     py_version_major = '3.9'
#     py_version_minor = '2'
#     py_semver = '3.9.2'
#     py_version = py_version_major
#     py_ver = '39'
#     py_name = f'python{py_ver}'

#     root = pathlib.Path.cwd()
#     product = 'python'
#     support = root.parent.parent / 'support'
#     externals = root.parent.parent / 'externals'
#     source = root.parent / 'source'
#     frameworks = support / 'Frameworks'
#     scripts = root / 'scripts'
#     build = root / 'targets' / 'build'
#     prefix = support / product
#     bin = prefix / 'bin'
#     lib = prefix / 'lib' / py_version
#     dylib = f'libpython_{py_version}.dylib'

#     py_external = externals / 'py.mxo'
#     pyjs_external = externals / 'pyjs.mxo'

#     bzip2_version = '1.0.8'
#     ssl_version='1.1.1g'
#     mac_dep_target='10.13'

#     url_python='https://www.python.org/ftp/python/${SEMVER}/Python-${SEMVER}.tgz'
#     url_openssl='https://www.openssl.org/source/openssl-${SSL_VERSION}.tar.gz'
#     url_getpip='https://bootstrap.pypa.io/get-pip.py'
