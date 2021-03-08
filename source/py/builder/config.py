"""config: global configuration

Place slow changing constants and variables here in UPPERCASE.



"""
import logging
import pathlib

IGNORE_ERRORS = False

DEBUG = True
if DEBUG:
    LOG_LEVEL = logging.DEBUG
else:
    LOG_LEVEL = logging.INFO

LOG_FORMAT = '%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s'



class Project:
    """Project is the source of per-project configuration and methods"""
    root = pathlib.Path.cwd()
    patch = root / 'patch'
    targets = root / 'targets'
    build = targets / 'build'
    downloads = build / 'downloads'
    src = build / 'src'
    lib = build / 'lib'
