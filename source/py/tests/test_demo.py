import os

import pytest
from conftest import cwd, join

MODULES = ["os", "sys", "math"]

EXPRESSIONS = {
    "1+1": "int: 2",
    "2*10": "int: 20"
}

EVAL_FAIL = [
    # syntax errors
    "000000000000007",
    "000777", 
    "0777",
    "077787", 
    "090000000000000",
    "0b100e1", 
    "0b101j2", 
    "0e",
    "0o153j2", 
    "0o777e1", 
    "0x.", 
    "0xj", 
    "lambda a,a:0",
    "lambda a,a=1:0",
    "lambda a=1,a=1:0",
]

EVAL_EQUALS = [
    ("0xff", 255)
    ("0777.", 777)
    ("0777.0", 777)
    ("000000000000000000000000000000000000000000000000000777e0", 777)
    ("0777e1", 7770)
    ("0e0", 0)
    ("0000e-012", 0)
    ("09.5", 9.5)
    ("000", 0)
    ("00.0", 0)
    ("0e3", 0)
    ("090000000000000.", 90000000000000.)
    ("090000000000000.0000000000000000000000", 90000000000000.)
    ("090000000000000e0", 90000000000000.)
    ("090000000000000e-0", 90000000000000.)
    ("000000000000008.", 8.)
    ("000000000000009.", 9.)
    ("0b101010", 42)
    ("-0b000000000010", -2)
    ("0o777", 511)
    ("-0o0000010", -8)
]

EXEC_FAIL = [
    # syntax errors
    'def f(a, a): pass',
    'def f(a = 0, a = 1): pass',
    'def f(a): global a; a = 1',
    'def f(a=1, b): pass',
    'None = 0',
    'None += 0',
    '__builtins__.None = 0',
    'def None(): pass',
    'class None: pass',
    '(a, None) = 0, 0',
    'for None in range(10): pass',
    'def f(None): pass',
    'import None',
    'import x as None',
    'from x import None',
    'from x import y as None',
]

IMPORT_FAIL = [
    'import (os, sys)',
    'import (os), (sys)',
    'import ((os), (sys))',
    'import (sys',
    'import sys)',
    'import (os,)',
    'import os As bar',
    'import os.path a bar',
    'from sys import stdin As stdout',
    'from sys import stdin a stdout',
    'from (sys) import stdin',
    'from __future__ import (nested_scopes',
    'from __future__ import nested_scopes)',
    'from __future__ import nested_scopes,\ngenerators',
    'from sys import (stdin',
    'from sys import stdin)',
    'from sys import stdin, stdout,\nstderr',
    'from sys import stdin si',
    'from sys import stdin,',
    'from sys import (*)',
    'from sys import (stdin,, stdout, stderr)',
    'from sys import (stdin, stdout),',
    ]




IMPORT_SUCCEED = [
    'import sys',
    'import os, sys',
    'import os as bar',
    'import os.path as bar',
    'from __future__ import nested_scopes, generators',
    'from __future__ import (nested_scopes,\ngenerators)',
    'from __future__ import (nested_scopes,\ngenerators,)',
    'from sys import stdin, stderr, stdout',
    'from sys import (stdin, stderr,\nstdout)',
    'from sys import (stdin, stderr,\nstdout,)',
    'from sys import (stdin\n, stderr, stdout)',
    'from sys import (stdin\n, stderr, stdout,)',
    'from sys import stdin as si, stdout as so, stderr as se',
    'from sys import (stdin as si, stdout as so, stderr as se)',
    'from sys import (stdin as si, stdout as so, stderr as se,)',
    ]





@pytest.fixture(scope="module", params=MODULES)
def module(request):
    expr = request.param
    yield expr

@pytest.fixture(scope="module", params=EXPRESSIONS.keys())
def expression(request):
    mod = request.param
    yield mod



def test_import(capfd, module):
    arg = f'import {module}'
    os.system(f'./tests/test_py {arg}')
    captured = capfd.readouterr()
    assert captured.out == f"imported: {module}\n"


def test_import_error(capfd):
    arg = f'import hello'
    os.system(f'./tests/test_py {arg}')
    captured = capfd.readouterr()
    assert captured.out == f"PyException('{arg}'): Ooops again.\n"

def test_eval(capfd, expression):
    arg = f'eval {expression}'
    os.system(f'./tests/test_py {arg}')
    captured = capfd.readouterr()
    result = EXPRESSIONS[expression]
    assert captured.out == f"{result}\n"

