#!/usr/bin/env python

import sys
import subprocess

#----------------------------------------------------------------------
## generic pipe functions

def rpl(x, y=''):
    def _func(s):
        return s.replace(x, y)
    return _func

def pipe(*args):
    def _func(txt):
        return subprocess.run(list(args), input=txt, 
        text=True, capture_output=True).stdout
    return _func

def read_file(path):
    with open(path) as f:
        txt = f.read()
    return txt

def remove_blanklines(txt):
    return '\n'.join([l for l in txt.splitlines() if l])

#----------------------------------------------------------------------
## main process pipeline

def main(path):
    # text processing pipeline 
    pipeline = [
        pipe('/usr/local/bin/stripcmt'), # strip comments
        remove_blanklines,
        rpl(';'),
        rpl('C74_CONST', 'const'),
        rpl('(void)', '()'),
    ]

    # read it
    with open(path) as f:
        txt = f.read()

    # process it
    for func in pipeline:
        txt = func(txt)

    return txt

if __name__ == '__main__':
    output = main(sys.argv[1])
    print(output) #  for convenient redirection
