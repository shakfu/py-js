import os

import pytest
from conftest import cwd, join

MODULES = ["os", "sys", "math"]

EXPRESSIONS = {
    "1+1": "int: 2",
    "2*10": "int: 20"
}


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

