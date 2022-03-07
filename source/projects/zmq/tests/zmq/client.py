import time
import sys

import zmq

PORT="5555"

def main():
    print("Connecting to python server")
    context = zmq.Context()
    requester = context.socket(zmq.REQ)
    requester.connect("tcp://localhost:5555")

    for i in range(10):
        print(f"Sending 1+{i}")
        requester.send_string(f"1+{i}")
        response = requester.recv_string()
        print(f"Received: {response}")

    requester.close()
    context.destroy()

main()
