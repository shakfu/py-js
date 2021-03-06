#!/usr/bin/env python3
"""
"""
import liblo
from pythonosc import udp_client

from ptpython.python_input import PythonInput

PORT = 7000

client = udp_client.SimpleUDPClient('127.0.0.1', PORT)

def send(data):
    liblo.send(liblo.Address(PORT), data)

def send_prefix(prefix, data):
    client.send_message(prefix, data)


def main():
    prompt = PythonInput(vi_mode=True)

    while True:
        try:
            text = prompt.app.run()
        except KeyboardInterrupt:
            continue
        except EOFError:
            break
        else:
            is_statement = False
            try:
                code = compile(text, '<stdin>', 'eval')
            except SyntaxError:
                is_statement = True
                code = compile(text, '<stdin>', 'exec')

            if is_statement:
                send(text)
            else:
                send_prefix('/eval', text)

    print("goodbye")



if __name__ == "__main__":
    main()