"""
credit to, idea and implementation heavily derives from Ian Duncan's 
scheme for max solution 
https://github.com/iainctduncan/scheme-for-max-cookbook/tree/master/editor-integration

Add to VIM
==========

let mapleader = ","
nnoremap <leader>e :w !python3 send.py<Enter><Enter>


TODO
====

make it smarter for python:

- check if it is a exec or an eval using code heuristics (AST?)
    - if exec, then prefix with /exec
    - if eval, then prefix with /eval
    etc..

- cmd2??
- prompt-toolkit??


"""


import sys
import liblo

data = '\n'.join(sys.stdin.readlines())
liblo.send(liblo.Address(7000), data.strip()+'\n')







