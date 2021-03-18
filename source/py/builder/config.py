"""config: global configuration

Place constants and minimally changing variables here in UPPERCASE.

"""
import logging

IGNORE_ERRORS = False

DEBUG = True
if DEBUG:
    LOG_LEVEL = logging.DEBUG
else:
    LOG_LEVEL = logging.INFO

LOG_FORMAT = '%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s'
