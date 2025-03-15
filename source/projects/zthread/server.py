#!/usr/bin/env python3
"""Hello World server in Python

- Binds REP socket to tcp://*:5555

- Expects b"Hello" from client, replies with b"World"

"""

import time
from typing import Optional, Any

import zmq

mem = {} # dict of eval / exec


def py_eval(code: str) -> Optional[Any]:
    res = None
    try:
        res = eval(code, mem, mem)
    except:
        pass
    try:
        exec(code, mem, mem)
    except Exception as e:
        res = str(e)
    return res


def serve():
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

        print(f"request: {msg}")
        
        res = py_eval(msg)

        print(f"response: {res}")

        #  Send reply back to client
        socket.send_string(str(res))

if __name__ == '__main__':
    serve()
