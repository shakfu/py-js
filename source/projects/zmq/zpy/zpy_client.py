#!/usr/bin/env python3

import zmq

context = zmq.Context()

#  Socket to talk to server
print("Connecting to python server...")
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")


request_response = [

    ('test float', 'float 10.2'),
    ('test int', 'int 2'),
    ('test list', 'list 10 sam 10.2 2 hello'),
    ('test XXX', 'test <need-type>'),

    ('1+1', '2'),
    ('XXX', 'python-eval-error'),
]


for request, expected_response in request_response:
    print(f"Sending request {request} ...")
    socket.send_string(request)

    #  Get the reply.
    message = socket.recv()
    message = message.decode('utf8')
    check = message == expected_response
    print(f"Received reply {request} -> {check} [ {message} | {expected_response} ]")

