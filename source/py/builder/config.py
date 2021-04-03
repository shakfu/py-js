"""config: global configuration

Place constants and minimally changing variables here in UPPERCASE.

"""

import logging

import colorlog

IGNORE_ERRORS = False

DEBUG = True
LOG_LEVEL = logging.DEBUG if DEBUG else logging.INFO
LOG_FORMAT = '%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s'

#COLOR_FMT = '%(log_color)s%(levelname)s:%(name)s:%(message)s'
# COLOR_FMT = "%(log_color)s%(levelname)-8s:%(reset)s %(message)s"
COLOR_FMT = "%(log_color)s%(levelname)-5s:%(reset)s %(message)s"

handler = colorlog.StreamHandler()
handler.setFormatter(colorlog.ColoredFormatter(COLOR_FMT))


def get_logger(name):
    logger = colorlog.getLogger(name)
    logger.addHandler(handler)
    logger.setLevel(LOG_LEVEL)
    return logger
