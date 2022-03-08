#!/usr/bin/env python3
"""Hello World server in Python

- Binds REP socket to tcp://*:5555

- Expects b"Hello" from client, replies with b"World"

"""

import time
import random
import zmq


ns = {}

def parse(msg):
    msg = msg.decode('utf8')
    tokens = msg.split()
    response = "<error>"
    if tokens[0] == 'test':
        if tokens[1] == 'float':
            response = 'float 10.2'
        elif tokens[1] == 'int':
            response = 'int 2'
        elif tokens[1] == 'list':
            response = 'list 10 sam 10.2 2 hello'
        else:
            response = 'test <need-type>'
    else:
        try:
            response = repr(eval(msg))
        except:
            response = 'python-eval-error'
    return response



context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")

print("zpy server running...")
while True:
    #  Wait for next request from client
    message = socket.recv()
    print(f"Received request: {message}")

    #  Do some 'work'
    time.sleep(1)

    response = parse(message)
    print(f'response: {response}')

    #  Send reply back to client
    socket.send_string(response)
