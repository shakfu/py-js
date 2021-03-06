#!/usr/bin/env python3

import sys

import liblo
from pythonosc import udp_client

from prompt_toolkit import PromptSession
from prompt_toolkit.completion import WordCompleter
from pygments.lexers.python import PythonLexer
from prompt_toolkit.lexers import PygmentsLexer

python_completer= WordCompleter([]) # put python keywords here


def main():

    session = PromptSession(lexer=PygmentsLexer(PythonLexer))

    while True:
        try:
            # text = session.prompt('=>> ', lexer=PygmentsLexer(PythonLexer), multiline=True)
            text = session.prompt('=>> ')
        except KeyboardInterrupt:
            continue
        except EOFError:
            break
        else:
            liblo.send(liblo.Address(7000), text)
    print("goodbye")


if __name__ == '__main__':
    main()
