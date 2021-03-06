#!/usr/bin/env python3
"""A simple cmd2 application."""
import cmd2
import sys
import liblo
from pythonosc import udp_client

#data = '\n'.join(sys.stdin.readlines())
#liblo.send(liblo.Address(7000), data.strip()+'\n')


class PyMax(cmd2.Cmd):
    """A simple cmd2 application."""
    client = udp_client.SimpleUDPClient('127.0.0.1', 7000)

    def __init__(self):
        cmd2.Cmd.__init__(self, use_ipython=True)

    def send(self, data):
        liblo.send(liblo.Address(7000), data)

    def send_prefix(self, prefix, data):
        self.client.send_message(prefix, data)

    
    def do_eval(self, statement):
        # statement contains a string
        #self.poutput(statement)
        #self.send(statement)
        self.send_prefix('/eval', statement)


    def do_exec(self, statement):
        # statement also has a list of arguments
        # quoted arguments remain quoted
        for arg in statement.arg_list:
            self.poutput(arg)

    def do_load(self, py_file):
        with open(py_file) as f:
            content = f.read()
        self.send(content)
        self.poutput(f'{py_file} contents sent')



    def do_articulate(self, statement):
        # statement.argv contains the command
        # and the arguments, which have had quotes
        # stripped
        for arg in statement.argv:
            self.poutput(arg)



if __name__ == '__main__':
    import sys
    c = PyMax()
    sys.exit(c.cmdloop())

