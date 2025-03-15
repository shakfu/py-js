#!/usr/bin/env python3
"""Hello World server in Python

- Binds REP socket to tcp://*:5555

- Expects b"Hello" from client, replies with b"World"

"""


import logging
import time
from typing import Optional, Any
import datetime

import zmq

MEM = {} # dict of eval / exec

DEBUG = True
COLOR = True

# ----------------------------------------------------------------------------
# logging config

class CustomFormatter(logging.Formatter):
    """custom logging formatting class"""

    white = "\x1b[97;20m"
    grey = "\x1b[38;20m"
    green = "\x1b[32;20m"
    cyan = "\x1b[36;20m"
    yellow = "\x1b[33;20m"
    red = "\x1b[31;20m"
    bold_red = "\x1b[31;1m"
    reset = "\x1b[0m"
    fmt = "%(delta)s - %(levelname)s - %(name)s.%(funcName)s - %(message)s"
    cfmt = (f"{white}%(delta)s{reset} - "
            f"{{}}%(levelname)s{{}} - "
            f"{white}%(name)s.%(funcName)s{reset} - "
            f"{grey}%(message)s{reset}")

    FORMATS = {
        logging.DEBUG: cfmt.format(grey, reset),
        logging.INFO: cfmt.format(green, reset),
        logging.WARNING: cfmt.format(yellow, reset),
        logging.ERROR: cfmt.format(red, reset),
        logging.CRITICAL: cfmt.format(bold_red, reset),
    }

    def __init__(self, use_color=COLOR):
        self.use_color = use_color

    def format(self, record):
        """custom logger formatting method"""
        if not self.use_color:
            log_fmt = self.fmt
        else:
            log_fmt = self.FORMATS.get(record.levelno)
        duration = datetime.datetime.fromtimestamp(
            record.relativeCreated / 1000, datetime.UTC
        )
        record.delta = duration.strftime("%H:%M:%S")
        formatter = logging.Formatter(log_fmt)
        return formatter.format(record)


strm_handler = logging.StreamHandler()
strm_handler.setFormatter(CustomFormatter())
# file_handler = logging.FileHandler("log.txt", mode='w')
# file_handler.setFormatter(CustomFormatter(use_color=False))
logging.basicConfig(
    level=logging.DEBUG if DEBUG else logging.INFO,
    handlers=[strm_handler],
    # handlers=[strm_handler, file_handler],
)


def py_eval(code: str) -> Optional[Any]:
    res = None
    try:
        res = eval(code, MEM, MEM)
    except:
        pass
    try:
        exec(code, MEM, MEM)
    except Exception as e:
        res = str(e)
    return res


def serve():
    log = logging.getLogger("zthread")
    log.info("server starting...")
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind("tcp://*:5555")

    while True:
        #  Wait for next request from client
        message = socket.recv()

        #  Do some 'work'
        # time.sleep(1)
        time.sleep(0.1)

        msg = message.decode()


        # print(f"request: {msg}")
        log.info("request: %s", msg)
        
        res = py_eval(msg)

        # print(f"response: {res}")
        log.info(f"response: %s", res)

        #  Send reply back to client
        socket.send_string(str(res))

if __name__ == '__main__':
    serve()
